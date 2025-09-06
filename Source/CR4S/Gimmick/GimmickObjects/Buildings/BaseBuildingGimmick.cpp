#include "BaseBuildingGimmick.h"

#include "Character/Components/EnvironmentalStatusComponent.h"
#include "Components/WidgetComponent.h"
#include "Gimmick/Components/ObjectShakeComponent.h"
#include "Gimmick/UI/BuildingDurabilityWidget.h"
#include "Gimmick/Manager/ItemGimmickSubsystem.h"


ABaseBuildingGimmick::ABaseBuildingGimmick()
	: PrevPlayTime(-1)
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	BuildingDurabilityWidgetComponent
		= CreateDefaultSubobject<UWidgetComponent>(TEXT("BuildingDurabilityWidgetComponent"));
	BuildingDurabilityWidgetComponent->SetupAttachment(RootComponent);

	Tags.Emplace("House");

	ShakeComponent = CreateDefaultSubobject<UObjectShakeComponent>(TEXT("ShakeComponent"));
	EnvironmentalStatusComponent = CreateDefaultSubobject<UEnvironmentalStatusComponent>(
		TEXT("EnvironmentalStatusComponent"));
}

void ABaseBuildingGimmick::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		ItemGimmickSubsystem = World->GetSubsystem<UItemGimmickSubsystem>();
	}

	if (IsValid(BuildingDurabilityWidgetComponent))
	{
		BuildingDurabilityWidgetInstance = Cast<UBuildingDurabilityWidget>(
			BuildingDurabilityWidgetComponent->GetUserWidgetObject());

		if (IsValid(BuildingDurabilityWidgetInstance))
		{
			BuildingDurabilityWidgetInstance->InitWidget(this);
			ToggleDurabilityWidget(false);
		}
	}
}

void ABaseBuildingGimmick::ToggleDurabilityWidget(const bool bIsOpen)
{
	if (IsValid(BuildingDurabilityWidgetComponent))
	{
		BuildingDurabilityWidgetComponent->SetVisibility(bIsOpen);
	}
}

void ABaseBuildingGimmick::InitEnvironmentalStatus(const FBuildingEnvironmentalStatus& InEnvironmentalStatus)
{
	if (IsValid(EnvironmentalStatusComponent))
	{
		EnvironmentalStatusComponent->SetMaxTemperature(InEnvironmentalStatus.MaxTemperature);
		EnvironmentalStatusComponent->SetMinTemperature(InEnvironmentalStatus.MinTemperature);
		EnvironmentalStatusComponent->SetMaxHumidity(InEnvironmentalStatus.MaxHumidity);
		EnvironmentalStatusComponent->SetMinHumidity(InEnvironmentalStatus.MinHumidity);
	}
}

void ABaseBuildingGimmick::StartBurn()
{
	PrevPlayTime = -1;

	UWorldTimeManager* TimeManager = GetWorld()->GetSubsystem<UWorldTimeManager>();
	if (IsValid(TimeManager) &&
		!TimeManager->OnWorldTimeUpdated.IsAlreadyBound(this, &ThisClass::UpdateWorldTime))
	{
		TimeManager->OnWorldTimeUpdated.AddUniqueDynamic(this, &ThisClass::UpdateWorldTime);
	}
}

void ABaseBuildingGimmick::EndBurn()
{
	UWorldTimeManager* TimeManager = GetWorld()->GetSubsystem<UWorldTimeManager>();
	if (IsValid(TimeManager) &&
		TimeManager->OnWorldTimeUpdated.IsAlreadyBound(this, &ThisClass::UpdateWorldTime))
	{
		TimeManager->OnWorldTimeUpdated.RemoveDynamic(this, &ThisClass::UpdateWorldTime);
	}
}

void ABaseBuildingGimmick::UpdateWorldTime(const int64 NewPlayTime)
{
	if (PrevPlayTime < 0)
	{
		PrevPlayTime = NewPlayTime;
		return;
	}

	const int64 DeltaInt = NewPlayTime - PrevPlayTime;
	PrevPlayTime = NewPlayTime;

	if (DeltaInt <= 0)
	{
		return;
	}

	const float ActualSeconds = static_cast<float>(DeltaInt);

	ApplyBurn(ActualSeconds);
}

void ABaseBuildingGimmick::ApplyBurn_Implementation(float ActualSeconds)
{
}

FGameplayTag ABaseBuildingGimmick::GetTutorialTagByRowName(FName RowName) const
{
	if (RowName.IsNone())
	{
		return FGameplayTag();
	}

	FString TagString = FString::Printf(TEXT("Tutorial.Building.%s"), *RowName.ToString());
	return FGameplayTag::RequestGameplayTag(FName(*TagString), false);
}

void ABaseBuildingGimmick::NotifyTutorial(const FGameplayTag& TutorialTag)
{
	ITutorialNotifiable::NotifyObjective(this, TutorialTag);
}