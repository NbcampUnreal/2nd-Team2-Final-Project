#include "Game/GameMode/C4DemoGameMode.h"
#include "Game/System/ObjectiveManager.h"
#include "Game/SaveGame/SaveGameManager.h"
#include "Kismet/GameplayStatics.h"
#include "CR4S.h"

void AC4DemoGameMode::StartPlay()
{
    Super::StartPlay();

    ObjectiveManager = GetWorld()->GetSubsystem<UObjectiveManager>();
}

void AC4DemoGameMode::HandleStartDemo()
{
    USaveGameManager* SaveGameManager = GetGameInstance()->GetSubsystem<USaveGameManager>();
    if (!SaveGameManager)
    {
        CR4S_Log(LogGM, Error, TEXT("SaveGameManager is null."));
        return;
    }

	// Load the demo save game
}

void AC4DemoGameMode::HandleLoadGame()
{
    Super::HandleLoadGame();

    UGameplayStatics::OpenLevel(this, FName("UserTestLevel"));
}
