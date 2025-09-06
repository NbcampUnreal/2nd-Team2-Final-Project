#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GameplayTagContainer.h"
#include "C4CheatManager.generated.h"

UCLASS()
class CR4S_API UC4CheatManager : public UCheatManager
{
	GENERATED_BODY()

#pragma region Time Manipulation Commands
public:
	UFUNCTION(Exec)
	void AddMinute(int32 Amount);
	UFUNCTION(Exec)
	void AddDay(int32 Amount);
#pragma endregion

#pragma region Monster Commands
public:
	UFUNCTION(Exec)
	void SetMonstersHP(const float InHealth);

	UFUNCTION(Exec)
	void ShowMonsterHPBar(const bool bShow);

#pragma endregion

#pragma region Item And Gimmick
public:
	UFUNCTION(Exec)
	void AddItem(FName RowName, int32 Count) const;

	UFUNCTION(Exec)
	void SetCraftingFreeMode(const bool bIsCraftingMode);

#pragma endregion

#pragma region Animal Commands
public:
	UFUNCTION(Exec)
	void KillAllAnimals() const;

	UFUNCTION(Exec)
	void StunAllAnimals(const float StunDuration) const;

	UFUNCTION(Exec)
	void SpawnAnimal(const FString& AnimalName, int32 Count = 1) const;

	UFUNCTION(Exec)
	void SpawnAnimalMonster(const FString& AnimalName, int32 Count = 1) const;
	
#pragma endregion

#pragma region Save
public:
	UFUNCTION(Exec)
	void SaveNow();
	UFUNCTION(Exec)
	void LoadGame();

#pragma endregion

#pragma region InvincibleMode
public:
	UFUNCTION(Exec)
	void SetInvincibleMode(const bool bInvincibleMode) const;
	
#pragma endregion
	
#pragma region Tutorial Commands
public:
	UFUNCTION(Exec)
	void IncrementObjective(FGameplayTag TutorialTag);

#pragma endregion
#pragma region Helper Deceleration
protected:
	UPROPERTY()
	TObjectPtr<class UTimeCheatHelper> TimeHelper;
	UPROPERTY()
	TObjectPtr<class UItemGimmickHelper> ItemGimmickHelper;
	UPROPERTY()
	TObjectPtr<class USaveGameHelper> SaveGameHelper;
	UPROPERTY()
	TObjectPtr<class UCharacterCheatHelper> CharacterCheatHelper;
	UPROPERTY()
	TObjectPtr<class UTutorialHelper> TutorialHelper;

	UPROPERTY()
	TObjectPtr<class UAnimalCheatHelper> AnimalHelper;
	virtual void InitCheatManager() override;

#pragma endregion

	
};
