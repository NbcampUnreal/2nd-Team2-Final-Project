#pragma once

#include "Blueprint/UserWidget.h"
#include "Game/System/TutorialManager.h"
#include "GameplayTagContainer.h"   // Ãß°¡
#include "TutorialWidget.generated.h"

class UTutorialSummaryWidget;
class UTextBlock;
class UVerticalBox;

UCLASS()
class CR4S_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

#pragma region Overrides

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

#pragma endregion

#pragma region Child Widgets

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UTutorialSummaryWidget> TutorialSummaryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StepNumber;
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* TutorialContent;

#pragma endregion

#pragma region Tutorial Widget Updates

	UFUNCTION()
	void UpdateTutorial(const FActiveTutorial& CurrentTutorial);

	UTutorialSummaryWidget* AddObjective(const FString& Description, const FString& CountText);

	void ClearTutorials();

private:
	int32 CurrentObjectiveSetID;

	TMap<FGameplayTag, UTutorialSummaryWidget*> ObjectiveWidgets;

#pragma endregion
};
