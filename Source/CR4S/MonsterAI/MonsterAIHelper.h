#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h" 

class UBehaviorTreeComponent;
class ABaseMonster;

namespace MonsterAIHelper
{
	CR4S_API ABaseMonster* GetControlledMonster(UBehaviorTreeComponent& OwnerComp);
	CR4S_API void SetMonstersHP(const UWorld* World, const float InHealth);
	CR4S_API void ShowMonsterHPBars(const UWorld* World, const bool bShow);
}