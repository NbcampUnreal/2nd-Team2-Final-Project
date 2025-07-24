#pragma once

#include "GameFramework/GameModeBase.h"
#include "C4BaseInGameMode.generated.h"

UCLASS()
class CR4S_API AC4BaseInGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void ReturnToMenuWithDelay(float Delay);

	void ReturnToMenu();

	virtual void HandleLoadGame();

	FTimerHandle ReturnToMenuTimerHandle;

};
