#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MonsterHPBarWidget.generated.h"

class UTextBlock;
class UMonsterAttributeComponent;
class UProgressBar;
class ABaseMonster;

UCLASS()
class CR4S_API UMonsterHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Monster HPBar")
	void BindToMonster(ABaseMonster* Monster);

	UFUNCTION(BlueprintCallable, Category = "Monster HPBar")
	void UnbindFromMonster();

	UFUNCTION()
	void OnMonsterDeath();

	UFUNCTION()
	void OnMonsterChangedHPBar(float CurrentHP, float MaxHP);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;

private:
	UFUNCTION(BlueprintCallable, Category = "Monster HPBar")
	void UpdateHPBar(float GetCurrentHP, float GetMaxHP);

	UPROPERTY()
	TWeakObjectPtr<ABaseMonster> BoundMonster;

	UPROPERTY()
	TWeakObjectPtr<UMonsterAttributeComponent> BoundAttributeComponent;
};
