// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MonsterStunWidget.generated.h"

class UMonsterStateComponent;
class URoundProgressBar;
/**
 * 
 */
UCLASS()
class CR4S_API UMonsterStunWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

#pragma region WidgetUpdate & Bind
	void InitializeWidgetForTargetComp(UMonsterStateComponent* TargetComp);
	void ClearBinding();

	UFUNCTION(BlueprintCallable)
	void UpdateStunWidget();
	
#pragma endregion
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<URoundProgressBar> Stun;

	UPROPERTY()
	TWeakObjectPtr<UMonsterStateComponent> CachedTargetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VisibleDuration{10};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPercent{0.2};
private:
	UPROPERTY()
	FTimerHandle WidgetVisibleTimerHandle;
};
