#include "Game/GameMode/C4SurvivalGameMode.h"
#include "Game/SaveGame/SaveGameManager.h"
#include "UI/InGame/SurvivalHUD.h"
#include "Engine/LevelScriptActor.h"
#include "Kismet/GameplayStatics.h"
#include "CR4S.h"

void AC4SurvivalGameMode::StartPlay()
{
    Super::StartPlay();

    HandleStartGame();
}

void AC4SurvivalGameMode::EndGameSequence()
{
    CR4S_SIMPLE_SCOPE_LOG;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        CR4S_Log(LogGM, Warning, TEXT("PlayerController not found."));
        return;
    }

    ASurvivalHUD* HUD = Cast<ASurvivalHUD>(PC->GetHUD());
    if (HUD)
    {
        CR4S_Log(LogGM, Log, TEXT("Playing ending sequence..."));
        HUD->PlayEndingSequence();
    }
    else
    {
        CR4S_Log(LogGM, Warning, TEXT("HUD cast failed."));
    }
}

void AC4SurvivalGameMode::HandleStartGame()
{
    CR4S_SIMPLE_SCOPE_LOG;

    USaveGameManager* SaveGameManager = GetGameInstance()->GetSubsystem<USaveGameManager>();
    if (!SaveGameManager)
    {
        CR4S_Log(LogGM, Error, TEXT("SaveGameManager is null."));
        return;
    }

    if (!SaveGameManager->IsNewGame())
    {
        CR4S_Log(LogGM, Log, TEXT("Existing save found. Applying save data and starting game."));

        SaveGameManager->ApplyAll();

        if (GameStartSFX)
        {
            CR4S_Log(LogGM, Log, TEXT("Playing game start sound effect."));
            UGameplayStatics::PlaySound2D(this, GameStartSFX);
        }

    }

    else
    {
        CR4S_Log(LogGM, Log, TEXT("New game detected. Showing guide widget."));

        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            ASurvivalHUD* HUD = Cast<ASurvivalHUD>(PC->GetHUD());
            if (HUD)
            {
                HUD->HandleGuideToggle();
            }
            else
            {
                CR4S_Log(LogGM, Warning, TEXT("HUD cast failed."));
            }
        }
        else
        {
            CR4S_Log(LogGM, Warning, TEXT("PlayerController not found."));
        }
    }
}

void AC4SurvivalGameMode::HandleLoadGame()
{
    Super::HandleLoadGame();

    UGameplayStatics::OpenLevel(this, FName("MainWorldMap"));
}