#include "BaseMonster.h"

#include "CR4S.h"
#include "Controller/BaseMonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/MonsterAttributeComponent.h"
#include "Components/MonsterSkillComponent.h"
#include "Components/MonsterStateComponent.h"
#include "Components/MonsterAnimComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/MonsterAIKeyNames.h" 
#include "Game/System/AudioManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Inventory/Components/BaseInventoryComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseMonster::ABaseMonster()
	: MyHeader(TEXT("BaseMonster"))
{
	PrimaryActorTick.bCanEverTick = false;

	// Initialize Components
	AttributeComponent = CreateDefaultSubobject<UMonsterAttributeComponent>(TEXT("AttributeComp"));
	SkillComponent = CreateDefaultSubobject<UMonsterSkillComponent>(TEXT("SkillComp"));
	StateComponent = CreateDefaultSubobject<UMonsterStateComponent>(TEXT("StateComp"));
	AnimComponent = CreateDefaultSubobject<UMonsterAnimComponent>(TEXT("AnimComp"));

	AIControllerClass = ABaseMonsterAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();

	// Bind OnDeath delegate
	if (AttributeComponent)
	{
		AttributeComponent->InitializeMonsterAttribute(MonsterID);
		AttributeComponent->OnDeath.AddDynamic(this, &ABaseMonster::HandleDeath);
	}

	if (StateComponent)
	{
		StateComponent->InitializeStunData(AttributeComponent->GetMonsterAttribute());
		StateComponent->OnStateChanged.AddDynamic(this, &ABaseMonster::OnMonsterStateChanged);
	}

	if (AnimComponent)
	{
		AnimComponent->Initialize(GetMesh());
	}

	if (ABaseMonsterAIController* AIC = Cast<ABaseMonsterAIController>(GetController()))
	{
		AIC->SetupPerceptionFromMonster(this);
	}

	OnTakeAnyDamage.AddDynamic(this, &ABaseMonster::OnTakeDamageEvent);
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		const int32 Count = MeshComp->GetNumMaterials();
		for (int32 i = 0; i < Count; ++i)
		{
			UMaterialInterface* Orig = MeshComp->GetMaterial(i);
			if (Orig)
			{
				UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(Orig, this);
				MeshComp->SetMaterial(i, Dyn);
				DynamicMaterials.Add(Dyn);
			}
		}
	}
}

void ABaseMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABaseMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(
		DamageAmount, DamageEvent, EventInstigator, DamageCauser
	);
	
	if (!AttributeComponent || IsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ABaseMonster] TakeDamage - AttributeComponent is null or Monster is already dead!"));
		return 0.0f;
	}

	// Apply damage via AttributeComponent
	AttributeComponent->ApplyDamage(ActualDamage);
	AddAccumulatedDamage(ActualDamage);

	return ActualDamage;
}

void ABaseMonster::TakeStun_Implementation(const float StunAmount)
{
	if (StateComponent)
		StateComponent->AddStun(StunAmount);
}

void ABaseMonster::UseSkill(int32 SkillIndex)
{
	if (SkillComponent)
	{
		SkillComponent->UseSkill(SkillIndex);
	}
}

int32 ABaseMonster::SelectSkillIndex()
{
	return 0;
}

bool ABaseMonster::IsDead() const
{
	return AttributeComponent && AttributeComponent->IsDead();
}

void ABaseMonster::HandleDeath()
{
	if (bIsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseMonster] HandleDeath already called"));
		return;
	}

	if (UAudioManager* AudioMgr = GetGameInstance()->GetSubsystem<UAudioManager>())
	{
		AudioMgr->StopBGM();
	}
	
	bIsDead = true;
	StateComponent->SetState(EMonsterState::Dead);
	OnDied.Broadcast(this);

	if (UBaseInventoryComponent* InventoryComp = GetPlayerInventory())
	{
		TryDropItems(InventoryComp);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		MoveComp->DisableMovement();

	if (ABaseMonsterAIController* AIC = Cast<ABaseMonsterAIController>(GetController()))
	{
		AIC->StopMovement();
		
		if (UBehaviorTreeComponent* BTComp = AIC->FindComponentByClass<UBehaviorTreeComponent>())
			BTComp->StopTree(EBTStopMode::Safe);

		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			BB->SetValueAsBool(FAIKeys::IsDead, true);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (AnimComponent)
	{
		AnimComponent->PlayDeathMontage();
	}

	SetLifeSpan(5.f);

	FTimerHandle FadeStartTimerHandle;
	FTimerDelegate FadeDelegate = FTimerDelegate::CreateLambda([this]()
	{
		StartFadeOut();
	});

	GetWorld()->GetTimerManager().SetTimer(
		FadeStartTimerHandle,
		FadeDelegate,
		3.0f,
		false
	);
}

void ABaseMonster::StartFadeOut()
{    
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;
    
	for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
	{
		if (UMaterialInstanceDynamic* DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(i))
		{
			DynMat->SetScalarParameterValue(TEXT("Appearance"), 1.0f);
		}
	}

	FTimerDelegate FadeDelegate = FTimerDelegate::CreateUObject(this, &ABaseMonster::UpdateFade);
	GetWorldTimerManager().SetTimer(FadeTimerHandle, FadeDelegate, 0.02f, true);

	MeshComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
}

void ABaseMonster::UpdateFade()
{
	ElapsedFadeTime += 0.02f;
	float NewAppearance = FMath::Lerp(1.0f, 0.0f, ElapsedFadeTime / 2.0f);
    
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		for (int32 i = 0; i < MeshComp->GetNumMaterials(); ++i)
		{
			if (UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(MeshComp->GetMaterial(i)))
			{
				DynMat->SetScalarParameterValue(TEXT("Appearance"), NewAppearance);
			}
		}
	}
    
	if (ElapsedFadeTime >= 2.0f)
	{
		GetWorldTimerManager().ClearTimer(FadeTimerHandle);
	}
}


void ABaseMonster::OnMonsterStateChanged(EMonsterState Previous, EMonsterState Current)
{
	if (auto* AIC = Cast<ABaseMonsterAIController>(GetController()))
	{
		if (auto* BB = AIC->GetBlackboardComponent())
		{
			BB->SetValueAsInt(FAIKeys::CurrentState, static_cast<int32>(Current));
		}
	}

	// NOTICE :: Test Log
	UE_LOG(LogTemp, Log, TEXT("[%s] OnMonsterStateChanged : State changed from %s to %s."),
		*MyHeader,
		*UEnum::GetValueAsString(Previous),
		*UEnum::GetValueAsString(Current)
	);
}

UBaseInventoryComponent* ABaseMonster::GetPlayerInventory() const
{
	if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		return Pawn->FindComponentByClass<UBaseInventoryComponent>();
	}
	return nullptr;
}

void ABaseMonster::TryDropItems(UBaseInventoryComponent* InventoryComp) const
{
	if (DropItems.IsEmpty())
		return;

	TMap<FName,int32> ItemsToAdd;
	for (const FDropData& DropData : DropItems)
	{
		if (!DropData.DropItemRowName.IsNone() && DropData.DropItemCount > 0)
		{
			ItemsToAdd.FindOrAdd(DropData.DropItemRowName) += DropData.DropItemCount;
		}
	}
	if (ItemsToAdd.IsEmpty())
		return;

	InventoryComp->AddItems(ItemsToAdd);
}

void ABaseMonster::OnTakeDamageEvent(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	FlashRed();
	PlayHitSound();
}

void ABaseMonster::FlashRed()
{
	for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
	{
		if (Mat)
		{
			Mat->SetVectorParameterValue(FName("DamageColor"), FLinearColor::Red);
			Mat->SetScalarParameterValue(FName("DamageFlash"), 10.0f);
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(FlashTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		FlashTimerHandle,
		this,
		&ABaseMonster::ResetFlash,
		FlashDuration,
		false
	);
}

void ABaseMonster::ResetFlash()
{
	for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
	{
		if (Mat)
		{
			Mat->SetScalarParameterValue(FName("DamageFlash"), 0.0f);
		}
	}
}

void ABaseMonster::PlayHitSound()
{
	if (HitSound)
	{
		if (UAudioManager* AudioMgr = GetWorld()->GetGameInstance()->GetSubsystem<UAudioManager>())
		{
			AudioMgr->PlaySFX(HitSound, GetActorLocation(), EConcurrencyType::Impact);
		}
		else
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}
	}
}
