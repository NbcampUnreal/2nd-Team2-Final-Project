#include "Game/System/WorldTimeManager.h"
#include "Game/System/SeasonManager.h"
#include "Game/System/EnvironmentManager.h"
#include "UI/InGame/SurvivalHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/Text.h"

bool UWorldTimeManager::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		return false;
	}

	if (World->GetName() == TEXT("MenuLevel"))
	{
		return false;
	}

	if (!World->IsGameWorld())
	{
		return false;
	}

	return true;
}

void UWorldTimeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UWorldTimeManager::OnPostWorldInit);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &UWorldTimeManager::OnWorldCleanup);
}

void UWorldTimeManager::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE)
	{
		return;
	}

	SeasonManager = GetWorld()->GetSubsystem<USeasonManager>();
	if (!SeasonManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("SeasonManager is null"));
	}

	EnvironmentManager = Cast<AEnvironmentManager>(UGameplayStatics::GetActorOfClass(World, AEnvironmentManager::StaticClass()));
	if (!EnvironmentManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldTimeManager: EnvironmentManager not found"));
	}
}

void UWorldTimeManager::StartWorldTime()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(TimeUpdateHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(TimeUpdateHandle, this, &UWorldTimeManager::UpdateTime, 1.0f / WorldTimeMultiplier, true);
	}
}

void UWorldTimeManager::SetWorldTime(FWorldTimeData NewTimeData)
{
	CurrentTimeData = NewTimeData;
	TotalPlayTime = NewTimeData.Day * DayCycleLength * 60 + NewTimeData.Minute * 60 + NewTimeData.Second;

	AdvanceSkyTime(CurrentTimeData.Minute, CurrentTimeData.Second);
	UpdateTimeWidget();
	OnWorldTimeUpdated.Broadcast(TotalPlayTime);
}

void UWorldTimeManager::UpdateTime()
{
	TotalPlayTime += 1;
	CurrentTimeData.Second += 1;
	if (CurrentTimeData.Second >= 60)
	{
		CurrentTimeData.Second = 0;
		CurrentTimeData.Minute += 1;

		OnMinuteUpdated.Broadcast(CurrentTimeData.Minute, DayCycleLength);

		if (CurrentTimeData.Minute >= DayCycleLength)
		{
			CurrentTimeData.Minute = 0;
			CurrentTimeData.Day += 1;

			SeasonManager->HandleDayChange();
		}
	}

	UpdateTimeWidget();
	OnWorldTimeUpdated.Broadcast(TotalPlayTime);
	AdvanceSkyTime(CurrentTimeData.Minute, CurrentTimeData.Second);
}

void UWorldTimeManager::PauseTime()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().PauseTimer(TimeUpdateHandle);
		UE_LOG(LogTemp, Warning, TEXT("WorldTimeManager: Time paused"));
	}
}

void UWorldTimeManager::ResumeTime()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().UnPauseTimer(TimeUpdateHandle);
		UE_LOG(LogTemp, Warning, TEXT("WorldTimeManager: Time resumed"));
	}
}

void UWorldTimeManager::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	// Cleanup logic if needed
	UE_LOG(LogTemp, Warning, TEXT("WorldTimeManager: OnWorldCleanup called"));
	World->GetTimerManager().ClearTimer(TimeUpdateHandle);
}

void UWorldTimeManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Warning, TEXT("WorldTimeManager: Deinitialize called"));
}

void UWorldTimeManager::ModifyTime(int32 Day, int32 Minute)
{
	TotalPlayTime += Day * DayCycleLength * 60 + Minute * 60; // Convert to seconds
	UE_LOG(LogTemp, Warning, TEXT("WorldTimeManager: Total Play Time modified to %lld seconds"), TotalPlayTime);

	int32 PassedDays = Day;

	CurrentTimeData.Minute += Minute;
	OnMinuteUpdated.Broadcast(CurrentTimeData.Minute, DayCycleLength);

	while (CurrentTimeData.Minute >= DayCycleLength)
	{
		CurrentTimeData.Minute -= DayCycleLength;
		PassedDays++;
	}

	for (int32 i = 0; i < PassedDays; ++i)
	{
		CurrentTimeData.Day += 1;
		SeasonManager->HandleDayChange();
	}

	UpdateTimeWidget();
	OnWorldTimeUpdated.Broadcast(TotalPlayTime);
	AdvanceSkyTime(CurrentTimeData.Minute, CurrentTimeData.Second);
}


void UWorldTimeManager::UpdateTimeWidget()
{
	if (!GetWorld()) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	ASurvivalHUD* InGameHUD = Cast<ASurvivalHUD>(PC->GetHUD());
	if (!InGameHUD) return;

	InGameHUD->GetInGameWidget()->UpdateTimeWidget(GetCurrentTimeData());
}


void UWorldTimeManager::AdvanceSkyTime(int32 Min, int32 Sec)
{
	if (!EnvironmentManager) {return;}

	const int32 TotalSecondsInDay = DayCycleLength * 60;
	const int32 CurrentSeconds = Min * 60 + Sec;

	const float TimeRatio = static_cast<float>(CurrentSeconds) / static_cast<float>(TotalSecondsInDay);
	const int32 TimeOfDay = FMath::Clamp(static_cast<int32>(TimeRatio * 2400.0f), 0, 2400);

	EnvironmentManager->SetSkyTime(TimeOfDay);
}

FString UWorldTimeManager::CalculateSecondToStr(int64 Time) const
{
	int32 Hours = Time / 3600;
	int32 Minutes = (Time % 3600) / 60;
	int32 Seconds = Time % 60;

	return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
}