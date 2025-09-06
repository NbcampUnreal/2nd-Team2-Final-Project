#pragma once

#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "TutorialHelper.generated.h"

UCLASS()
class CR4S_API UTutorialHelper : public UObject
{
	GENERATED_BODY()
	
public:
	void IncrementObjective(FGameplayTag TutorialTag);
};
