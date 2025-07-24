#pragma once

#include "Game/GameMode/C4BaseInGameMode.h"
#include "C4SurvivalGameMode.generated.h"

UCLASS()
class CR4S_API AC4SurvivalGameMode : public AC4BaseInGameMode
{
	GENERATED_BODY()
	
protected:

	UFUNCTION(BlueprintCallable)
	void EndGameSequence();

	virtual void StartPlay() override;

	virtual void HandleLoadGame() override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* GameStartSFX;

	UPROPERTY()
	bool bIsTransitioningLevel = false;

private:

	void HandleStartGame();

};
