#pragma once

#include "Game/GameMode/C4BaseInGameMode.h"
#include "C4DemoGameMode.generated.h"

class UObjectiveManager;

UCLASS()
class CR4S_API AC4DemoGameMode : public AC4BaseInGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void StartPlay() override;

	TObjectPtr<UObjectiveManager> ObjectiveManager;

	virtual void HandleLoadGame() override;

	void HandleStartDemo();
};
