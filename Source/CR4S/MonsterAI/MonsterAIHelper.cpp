#include "MonsterAI/MonsterAIHelper.h"
#include "AIController.h"
#include "CR4S.h"
#include "EngineUtils.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/MonsterAttributeComponent.h"
#include "Manager/MonsterHealthManager.h"
#include "MonsterAI/BaseMonster.h"

ABaseMonster* MonsterAIHelper::GetControlledMonster(UBehaviorTreeComponent& OwnerComp)
{
	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		return Cast<ABaseMonster>(AIC->GetPawn());
	}

	return nullptr;
}

void MonsterAIHelper::SetMonstersHP(const UWorld* World, const float InHealth)
{
	if (!World) return;

	int32 Count = 0;
	for (TActorIterator<ABaseMonster> It(World); It; ++It)
	{
		const ABaseMonster* Monster = *It;
		if (UMonsterAttributeComponent* AttrComp = Monster->FindComponentByClass<UMonsterAttributeComponent>())
		{
			AttrComp->SetCurrentHP(InHealth);
			++Count;
		}
	}

	CR4S_Log(LogDa, Log, TEXT("[MonsterAIHelper] %d monsters' health set to %.1f"), Count, InHealth);
}

void MonsterAIHelper::ShowMonsterHPBars(const UWorld* World, const bool bShow)
{
	if (!World) return;

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (UMonsterHealthManager* HealthBarManager = GameInstance->GetSubsystem<UMonsterHealthManager>())
		{
			HealthBarManager->SetHPBarsVisible(bShow);
			
			const FString StatusText = bShow ? TEXT("enabled") : TEXT("disabled");
			CR4S_Log(LogMonster, Log, TEXT("[MonsterAIHelper] Monster health bars %s"), *StatusText);
		}
	}
}