#include "Game/GameMode/C4CineGameMode.h"
#include "Engine/LevelScriptActor.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h" 
#include "UI/Common/LoadingWidget.h"
#include "CR4S.h"

void AC4CineGameMode::StartPlay()
{
    Super::StartPlay();

    PlayCinematic();
}

void AC4CineGameMode::PlayCinematic()
{
    ALevelScriptActor* LevelScript = GetWorld()->GetLevelScriptActor();
    if (LevelScript)
    {
        if (UFunction* CustomEvent = LevelScript->FindFunction(FName("OnLevelLoaded")))
        {
            LevelScript->ProcessEvent(CustomEvent, nullptr);
        }
    }
}

void AC4CineGameMode::NotifyCinematicFinished()
{
    if (LoadingWidgetClass)
    {
        ULoadingWidget* LoadingWidget = CreateWidget<ULoadingWidget>(GetWorld(), LoadingWidgetClass);
        if (LoadingWidget)
        {
            LoadingWidget->AddToViewport();
            LoadingWidget->HideImage();
        }
    }
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AC4CineGameMode::GoToMainMap, 1.0f, false);
}

void AC4CineGameMode::GoToMainMap()
{
    UGameplayStatics::OpenLevel(this, FName("MainWorldMap"));
}
