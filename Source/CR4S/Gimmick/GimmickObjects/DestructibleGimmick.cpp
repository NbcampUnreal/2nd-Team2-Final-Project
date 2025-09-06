#include "DestructibleGimmick.h"

#include "BaseDestructObject.h"
#include "CR4S.h"
#include "Game/System/AudioManager.h"
#include "Gimmick/Components/DestructibleComponent.h"
#include "Gimmick/Components/ObjectShakeComponent.h"
#include "Gimmick/Data/GimmickData.h"
#include "Gimmick/Manager/ItemGimmickSubsystem.h"
#include "Inventory/Components/PlayerInventoryComponent.h"

ADestructibleGimmick::ADestructibleGimmick()
	: DestroyDelay(1.f)

{
	PrimaryActorTick.bCanEverTick = false;

	DestructibleComponent = CreateDefaultSubobject<UDestructibleComponent>(TEXT("DestructibleComponent"));
	ShakeComponent = CreateDefaultSubobject<UObjectShakeComponent>(TEXT("ShakeComponent"));
}

void ADestructibleGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (CR4S_VALIDATE(LogGimmick, IsValid(DestructibleComponent)))
	{
		DestructibleComponent->OnTakeDamage.BindDynamic(this, &ThisClass::OnGimmickTakeDamage);
		DestructibleComponent->OnDestroy.BindDynamic(this, &ThisClass::OnGimmickDestroy);

		if (CR4S_VALIDATE(LogGimmick, IsValid(ItemGimmickSubsystem)))
		{
			if (const FGimmickInfoData* GimmickInfoData = ItemGimmickSubsystem->
				FindGimmickInfoData(GetGimmickDataRowName()))
			{
				DestructibleComponent->SetMaxHealth(GimmickInfoData->GimmickMaxHealth);
				DestructibleComponent->SetCurrentHealth(GimmickInfoData->GimmickMaxHealth);

				TakeDamageSound = GimmickInfoData->TakeDamageSound;
				DestroySound = GimmickInfoData->DestroySound;

				InitializeThresholds();
			}
		}
	}
}

float ADestructibleGimmick::TakeDamage(const float DamageAmount, struct FDamageEvent const& DamageEvent,
                                       AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!CR4S_VALIDATE(LogGimmick, IsValid(DestructibleComponent)))
	{
		return 0.f;
	}

	const UPlayerInventoryComponent* PlayerInventoryComponent
		= DamageCauser->FindComponentByClass<UPlayerInventoryComponent>();
	if (IsValid(PlayerInventoryComponent))
	{
		bool bApplyBonusDamage = false;
		int32 RequiredHits = MaxHitCount;
		GetRequiredHitsForTool(PlayerInventoryComponent->GetHeldToolTag(), bApplyBonusDamage, RequiredHits);
		if (bApplyBonusDamage)
		{
			Damage = CalculatedDamage(RequiredHits);
		}
	}

	DestructibleComponent->TakeDamage(DamageCauser, Damage);

	return Damage;
}

void ADestructibleGimmick::GetResources(const AActor* InventoryOwnerActor) const
{
	if (!CR4S_VALIDATE(LogGimmick, IsValid(InventoryOwnerActor)) ||
		!CR4S_VALIDATE(LogGimmick, IsValid(ItemGimmickSubsystem)) ||
		!YieldResources.IsValidIndex(NextThresholdIndex))
	{
		return;
	}

	UBaseInventoryComponent* InventorySystem
		= InventoryOwnerActor->FindComponentByClass<UBaseInventoryComponent>();
	if (!CR4S_VALIDATE(LogGimmick, IsValid(InventorySystem)))
	{
		ItemGimmickSubsystem->SpawnItemPouch(this, YieldResources[NextThresholdIndex]);

		return;
	}

	InventorySystem->AddItems(YieldResources[NextThresholdIndex]);
}

void ADestructibleGimmick::LoadGimmickSaveGameData_Implementation(const FGimmickSaveGameData& GimmickSaveGameData)
{
	Super::LoadGimmickSaveGameData_Implementation(GimmickSaveGameData);

	if (IsValid(ShakeComponent))
	{
		ShakeComponent->SetOriginalLocation(GetActorLocation());
	}
}

void ADestructibleGimmick::GetGimmickHealthData(bool& bOutSuccess, float& OutCurrentHealth, float& OutMaxHealth) const
{
	if (IsValid(DestructibleComponent))
	{
		bOutSuccess = true;
		OutCurrentHealth = DestructibleComponent->GetCurrentHealth();
		OutMaxHealth = DestructibleComponent->GetMaxHealth();
	}
	else
	{
		bOutSuccess = false;
	}
}

void ADestructibleGimmick::SetGimmickHealthData(const float NewCurrentHealth, const float NewMaxHealth)
{
	if (IsValid(DestructibleComponent))
	{
		DestructibleComponent->SetMaxHealth(NewMaxHealth);
		DestructibleComponent->SetCurrentHealth(NewCurrentHealth);
	}
}

void ADestructibleGimmick::OnGimmickTakeDamage(AActor* DamageCauser, const float DamageAmount,
                                               const float CurrentHealth)
{
	PlaySFX(TakeDamageSound, GetActorLocation(), EConcurrencyType::Default);

	CR4S_Log(LogGimmick, Warning, TEXT("Gimmick is damaged / DamageAmount: %.1f / CurrentHealth: %.1f"), DamageAmount,
	         CurrentHealth);

	if (IsValid(ShakeComponent))
	{
		ShakeComponent->Shake();
	}

	while (NextThresholdIndex < HealthThresholds.Num() && CurrentHealth <= HealthThresholds[NextThresholdIndex])
	{
		GetResources(DamageCauser);
		NextThresholdIndex++;
	}
}

void ADestructibleGimmick::OnGimmickDestroy(AActor* DamageCauser)
{
	PlaySFX(DestroySound, GetActorLocation(), EConcurrencyType::Default);

	GetResources(DamageCauser);

	ITutorialNotifiable::NotifyObjective(this, GetTutorialTag());

	Destroy();

	if (CR4S_VALIDATE(LogGimmick, DestructObjectClass))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride
			= ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(GetActorLocation());
		SpawnTransform.SetRotation(GetActorRotation().Quaternion());
		SpawnTransform.SetScale3D(GetActorScale());

		GetWorld()->SpawnActor<ABaseDestructObject>(DestructObjectClass,
		                                            SpawnTransform,
		                                            SpawnParameters);
	}
}

void ADestructibleGimmick::InitializeThresholds()
{
	if (!CR4S_VALIDATE(LogGimmick, IsValid(DestructibleComponent)))
	{
		return;
	}

	if (const FGimmickInfoData* GimmickInfoData = GetGimmickInfoData())
	{
		for (const auto& [HitToolTag, HitCount] : GimmickInfoData->HitCounts)
		{
			MaxHitCount = FMath::Max(MaxHitCount, HitCount);
		}

		HealthThresholds.Reset();
		YieldResources.Reset();

		for (int32 Step = 1; Step <= MaxHitCount; Step++)
		{
			int32 Threshold = FMath::CeilToInt(
				DestructibleComponent->GetMaxHealth() * (MaxHitCount - Step) / MaxHitCount);
			HealthThresholds.Add(Threshold);

			TMap<FName, int32> Resources;
			for (const FResourceItemData& Resource : GimmickInfoData->Resources)
			{
				const int32 BaseYield = Resource.MaxCount / MaxHitCount;
				const int32 Yield = Step < MaxHitCount
					                    ? BaseYield
					                    : Resource.MaxCount - BaseYield * (MaxHitCount - 1);

				Resources.FindOrAdd(Resource.RowName) = Yield;
			}

			YieldResources.Add(Resources);
		}
	}
}

void ADestructibleGimmick::GetRequiredHitsForTool(const FGameplayTag& HeldToolTag, bool& bApplyBonusDamage,
                                                  int32& OutHitCount) const
{
	bApplyBonusDamage = false;

	if (const FGimmickInfoData* GimmickInfoData = GetGimmickInfoData())
	{
		for (const auto& [HitToolTag, HitCount] : GimmickInfoData->HitCounts)
		{
			if (HeldToolTag.MatchesTag(HitToolTag))
			{
				bApplyBonusDamage = true;
				OutHitCount = HitCount;
				return;
			}
		}
	}
}

int32 ADestructibleGimmick::CalculatedDamage(const int32 RequiredHits) const
{
	if (!IsValid(DestructibleComponent))
	{
		return 0;
	}

	return FMath::CeilToInt(DestructibleComponent->GetMaxHealth() / RequiredHits);
}

FGameplayTag ADestructibleGimmick::GetTutorialTag() const
{
	FName RowName = GetGimmickDataRowName();
	if (RowName.IsNone() || RowName == FName("Tree"))
	{
		return FGameplayTag();
	}
	//
	FString TagString = FString::Printf(TEXT("Tutorial.Gimmick.%s"), *RowName.ToString());
	return FGameplayTag::RequestGameplayTag(FName(*TagString), false);
}
