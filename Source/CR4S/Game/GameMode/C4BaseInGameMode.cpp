#include "Game/GameMode/C4BaseInGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "CR4S.h"

void AC4BaseInGameMode::ReturnToMenuWithDelay(float Delay)
{
    CR4S_Log(LogGM, Log, TEXT("ReturnToMenuWithDelay called. Returning to MenuLevel in %.2f seconds."), Delay);

    GetWorld()->GetTimerManager().SetTimer(ReturnToMenuTimerHandle, this, &AC4BaseInGameMode::ReturnToMenu, Delay, false);
}

void AC4BaseInGameMode::ReturnToMenu()
{
    CR4S_Log(LogGM, Log, TEXT("Opening MenuLevel..."));
    UGameplayStatics::OpenLevel(this, FName("MenuLevel"));
}

void AC4BaseInGameMode::HandleLoadGame()
{
	CR4S_Log(LogGM, Log, TEXT("HandleLoadGame called. Restarting game."));
}
