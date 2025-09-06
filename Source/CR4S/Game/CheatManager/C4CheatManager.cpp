#include "Game/CheatManager/C4CheatManager.h"
#include "CharacterCheatHelper.h"
#include "EngineUtils.h"
#include "Game/CheatManager/TimeCheatHelper.h"
#include "Game/CheatManager/SaveGameHelper.h"
#include "Game/CheatManager/TutorialHelper.h"
#include "MonsterAI/MonsterAIHelper.h"
#include "Game/CheatManager/AnimalCheatHelper.h"
#include "CR4S.h"
#include "ItemGimmickHelper.h"
#include "Inventory/Components/PlayerInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MonsterAI/MonsterAIHelper.h"

void UC4CheatManager::InitCheatManager()
{
	Super::InitCheatManager();

	//Create Helper and Bind
	TimeHelper = NewObject<UTimeCheatHelper>(this);

	ItemGimmickHelper = NewObject<UItemGimmickHelper>(this);
	SaveGameHelper = NewObject<USaveGameHelper>(this);
	CharacterCheatHelper = NewObject<UCharacterCheatHelper>(this);
	TutorialHelper = NewObject<UTutorialHelper>(this);
	AnimalHelper = NewObject<UAnimalCheatHelper>(this);
}

void UC4CheatManager::SetInvincibleMode(const bool bInvincibleMode) const
{
	if (CharacterCheatHelper)
	{
		CharacterCheatHelper->SetInvincibleMode(bInvincibleMode);
	}
}

void UC4CheatManager::IncrementObjective(FGameplayTag TutorialTag)
{
	TutorialHelper->IncrementObjective(TutorialTag);
}

void UC4CheatManager::SaveNow()
{
	if (SaveGameHelper)
	{
		SaveGameHelper->SaveNow();
	}
}

void UC4CheatManager::LoadGame()
{
	if (SaveGameHelper)
	{
		SaveGameHelper->LoadGame();
	}
}

void UC4CheatManager::AddItem(const FName RowName, const int32 Count) const
{
	if (CR4S_VALIDATE(LogCheatManager, IsValid(ItemGimmickHelper)))
	{
		ItemGimmickHelper->AddItem(RowName, Count);
	}
}

void UC4CheatManager::SetCraftingFreeMode(const bool bIsCraftingMode)
{
	const UWorld* World = GetWorld();
	if (IsValid(World))
	{
		const APlayerController* Controller = World->GetFirstPlayerController();
		if (IsValid(Controller))
		{
			const APawn* Pawn = Controller->GetPawn();
			if (IsValid(Pawn))
			{
				UPlayerInventoryComponent* PlayerInventory = Pawn->FindComponentByClass<UPlayerInventoryComponent>();
				if (IsValid(PlayerInventory))
				{
					PlayerInventory->ChangeCraftingFreeMode(bIsCraftingMode);
				}
			}
		}
	}
}

void UC4CheatManager::AddMinute(int32 Amount)
{
	if (TimeHelper)
	{
		//Acutal Logic for the Cheat Command processed in Helper
		TimeHelper->AddMinute(Amount);
	}
}

void UC4CheatManager::AddDay(int32 Amount)
{
	if (TimeHelper)
	{
		TimeHelper->AddDay(Amount);
	}
}

void UC4CheatManager::SetMonstersHP(const float InHealth)
{
	MonsterAIHelper::SetMonstersHP(GetWorld(), InHealth);
}

void UC4CheatManager::ShowMonsterHPBar(const bool bShow)
{
	MonsterAIHelper::ShowMonsterHPBars(GetWorld(), bShow);
}

void UC4CheatManager::KillAllAnimals() const
{
	if (AnimalHelper)
	{
		AnimalHelper->KillAllAnimals();
	}
}

void UC4CheatManager::StunAllAnimals(const float StunDuration) const
{
	if (AnimalHelper)
	{
		AnimalHelper->StunAllAnimals(StunDuration);
	}
}

void UC4CheatManager::SpawnAnimal(const FString& AnimalName, int32 Count) const
{
	if (AnimalHelper)
	{
		AnimalHelper->SpawnAnimal(AnimalName, Count);
	}
}

void UC4CheatManager::SpawnAnimalMonster(const FString& MonsterName, int32 Count) const
{
	if (AnimalHelper)
	{
		AnimalHelper->SpawnAnimalMonster(MonsterName, Count);
	}
}