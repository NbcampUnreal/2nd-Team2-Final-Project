#include "MonsterSkillComponent.h"
#include "MonsterAI/Data/MonsterDataSubsystem.h"
#include "MonsterAI/BaseMonster.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "MonsterAI/Components/MonsterAnimComponent.h"
#include "Utility/CombatStatics.h"
#include "CR4S.h"
#include "MonsterStateComponent.h"

UMonsterSkillComponent::UMonsterSkillComponent()
	: MyHeader(TEXT("MonsterSkillComp"))
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMonsterSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const ABaseMonster* Monster = Cast<ABaseMonster>(GetOwner()))
	{
		InitializeMonsterSkills(Monster->MonsterID);
	}

	if (const AActor* Owner = GetOwner())
	{
		TArray<UPrimitiveComponent*> CapsuleComps;
		GetOwner()->GetComponents<UPrimitiveComponent>(CapsuleComps);

		for (UPrimitiveComponent* Comp : CapsuleComps)
		{
			if (Comp->ComponentHasTag("BodyCollider"))
			{
				BodyColliders.Add(Comp);
				Comp->OnComponentBeginOverlap.AddDynamic(this, &UMonsterSkillComponent::OnAttackHit);
			}
		}
	}

	SpawnHandle = GetWorld()->AddOnActorSpawnedHandler(
			FOnActorSpawned::FDelegate::CreateUObject(
				this, &UMonsterSkillComponent::OnSkillActorSpawned
			)
		);
}

void UMonsterSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] EndPlay : World is null, skipping timer cleanup."), *MyHeader);
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	for (FTimerHandle& Handle : SkillCooldownTimers)
	{
		if (TimerManager.IsTimerActive(Handle))
		{
			TimerManager.ClearTimer(Handle);
		}
	}

	GetWorld()->RemoveOnActorSpawnedHandler(SpawnHandle);

	Super::EndPlay(EndPlayReason);
}

void UMonsterSkillComponent::InitializeMonsterSkills(const FName MonsterID)
{
	if (const UMonsterDataSubsystem* Subsys = GetWorld()->GetGameInstance()->GetSubsystem<UMonsterDataSubsystem>())
	{
		SkillList.Empty();
		bSkillReady.Empty();
		SkillCooldownTimers.Empty();

		SkillList = Subsys->GetMonsterSkillData(MonsterID);

		if (SkillList.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeMonsterSkills : Fail to load the monster skill data table."), *MyHeader);
			return;
		}
		
		for (const auto& Row : SkillList)
		{
			bSkillReady.Add(true);
			SkillCooldownTimers.Add(FTimerHandle());
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeMonsterSkills : %d skills loaded from MonsterSkillTable."), *MyHeader, SkillList.Num());
	}
}

void UMonsterSkillComponent::UseSkill(int32 Index)
{
	if (IsSkillReady(Index) == false)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] UseSkill : Skill %d is not ready."), *MyHeader, Index);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] UseSkill : Use skill %d."), *MyHeader, Index);

	const FMonsterSkillData& Skill = SkillList[Index];
	CurrentSkillIndex = Index;

	if (IsValid(Skill.SkillMontage))
	{
		if (UMonsterAnimComponent* AnimComponent = GetOwner()->GetComponentByClass<UMonsterAnimComponent>())
		{
			AnimComponent->PlayMontage(Skill.SkillMontage);
		}
	}

	// Cooldown
	bSkillReady[Index] = false;

	GetWorld()->GetTimerManager().SetTimer(
		SkillCooldownTimers[Index],
		FTimerDelegate::CreateUObject(this, &UMonsterSkillComponent::ResetCooldown, Index),
		Skill.Cooldown,
		false
	);
}

bool UMonsterSkillComponent::IsSkillReady(int32 Index) const
{
	return bSkillReady.IsValidIndex(Index) && bSkillReady[Index];
}

float UMonsterSkillComponent::GetSkillRange(int32 Index) const
{
	return SkillList.IsValidIndex(Index) ? SkillList[Index].Range : 0.f;
}

TArray<int32> UMonsterSkillComponent::GetAvailableSkillIndices() const
{
	TArray<int32> SkillIndices;
	for (int32 i = 0; i < SkillList.Num(); ++i)
	{
		if (bSkillReady.IsValidIndex(i) && bSkillReady[i])
		{
			SkillIndices.Add(i);
		}
	}
	return SkillIndices;
}

const FMonsterSkillData& UMonsterSkillComponent::GetCurrentSkillData() const
{
	static const FMonsterSkillData Empty;
	return SkillList.IsValidIndex(CurrentSkillIndex) ? SkillList[CurrentSkillIndex] : Empty;
}

const FMonsterSkillData& UMonsterSkillComponent::GetSkillData(int32 Index) const
{
	check(SkillList.IsValidIndex(Index));
	return SkillList[Index];
}

UAnimInstance* UMonsterSkillComponent::GetAnimInstance() const
{
	if (const ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* Mesh = Char->GetMesh())
			return Mesh->GetAnimInstance();
	}
	return nullptr;
}

void UMonsterSkillComponent::ResetCooldown(int32 Index)
{
	if (bSkillReady.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] ResetCooldown : Reset skill cooldown : %d."), *MyHeader, Index);
		bSkillReady[Index] = true;
	}
}

void UMonsterSkillComponent::SetAttackCollisionEnabled(bool bEnable)
{
	bIsAttackActive = bEnable;
	const FMonsterSkillData& Skill = SkillList[CurrentSkillIndex];

	auto ConfigureColliders = [bEnable](const TArray<UPrimitiveComponent*>& Colliders)
		{
			for (UPrimitiveComponent* Collider : Colliders)
			{
				if (bEnable)
				{
					Collider->SetCollisionProfileName(TEXT("MonsterSkillActor"));
					Collider->SetGenerateOverlapEvents(true);
				}
				else
				{
					RestoreOriginalCollisionProfile(Collider);
				}
			}
		};

	if (Skill.bUseBodyCollision && !BodyColliders.IsEmpty())
	{
		ConfigureColliders(BodyColliders);
	}

	if (bEnable)
	{
		AlreadyHitActors.Empty();
	}
}

void UMonsterSkillComponent::RestoreOriginalCollisionProfile(UPrimitiveComponent* Collider)
{
	if (Collider)
	{
		Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Collider->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		Collider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		Collider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Collider->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
		Collider->SetGenerateOverlapEvents(false);
	}
}

void UMonsterSkillComponent::OnAttackHit(
	UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bIsAttackActive || !OtherActor || OtherActor == GetOwner()) return;
	if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic) return;

	const FMonsterSkillData& CurrentSkillData = SkillList[CurrentSkillIndex];

	if (!CurrentSkillData.bAllowMultipleHits && AlreadyHitActors.Contains(OtherActor)) return;
	if (!BodyColliders.Contains(HitComp) || !CurrentSkillData.bUseBodyCollision) return;

	float Damage = CurrentSkillData.BodyDamage;
	float Stun = CurrentSkillData.BodyStunt;

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetOwner()->GetInstigatorController(),
		GetOwner(),
		UDamageType::StaticClass()
	);

	if (OtherActor->GetClass()->ImplementsInterface(UStunnableInterface::StaticClass()))
	{
		UCombatStatics::ApplyStun(OtherActor, Stun);
	}

#if WITH_EDITOR

	const FString AttackerName = GetOwner() ? GetOwner()->GetName() : TEXT("UnknownOwner");
	const FString VictimName = OtherActor->GetName();
	const FString SkillIndexStr = FString::Printf(TEXT("Skill_%d"), CurrentSkillIndex);

	UE_LOG(LogMonster, Log, TEXT("[SkillHit] %s used %s on %s Damage: %.1f, Stun: %.1f"),
		*AttackerName,
		*SkillIndexStr,
		*VictimName,
		Damage,
		Stun
	);

#endif

	AlreadyHitActors.Add(OtherActor);
}

void UMonsterSkillComponent::OnSkillActorSpawned(AActor* NewActor)
{
	if (!NewActor) 
		return;
	
	if (NewActor->GetInstigator() == Cast<APawn>(GetOwner()))
	{
		ActiveSkillActors.Add(NewActor);
	}
	else if (NewActor->GetOwner() == GetOwner())
	{
		ActiveSkillActors.Add(NewActor);
	}
}

void UMonsterSkillComponent::CancelAllSkills()
{
	for (TWeakObjectPtr<AActor> WeakA : ActiveSkillActors)
	{
		if (AActor* A = WeakA.Get())
		{
			A->Destroy();
		}
	}
	ActiveSkillActors.Empty();
	
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearAllTimersForObject(this);
	}
}
