#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MonsterHealthManager.generated.h"


class ABaseMonster;
class UMonsterHPBarWidget;

UCLASS(Blueprintable, BlueprintType)
class CR4S_API UMonsterHealthManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "HP Manager")
	void SetHPBarsVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "HP Manager")
	void RegisterMonster(ABaseMonster* Monster);

	UFUNCTION(BlueprintCallable, Category = "HP Manager")
	void UnregisterMonster(ABaseMonster* Monster);

	UFUNCTION(BlueprintCallable, Category = "HP Manager")
	void RefreshAllHealthBars();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP Manager")
	TSubclassOf<UMonsterHPBarWidget> HPBarWidgetClass;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<ABaseMonster>> RegisteredMonsters;
	
	UPROPERTY()
	TArray<TObjectPtr<UMonsterHPBarWidget>> HPBarWidgets;

protected:
	UPROPERTY(EditAnywhere, Category = "Health Bar Manager")
	FSoftClassPath WidgetBlueprintPath = FSoftClassPath(TEXT("/Game/CR4S/_Blueprint/UI/MonsterAI/WBP_MonsterHPWidget.WBP_MonsterHPWidget_C"));
	
private:
	bool bHealthBarsVisible = false;

	UMonsterHPBarWidget* CreateHealthBarWidget();
	void UpdateHealthBarVisibility();

	UFUNCTION()
	void OnMonsterSpawned(AActor* SpawnedMonster);

	UFUNCTION()
	void OnMonsterDestroyed();
};
