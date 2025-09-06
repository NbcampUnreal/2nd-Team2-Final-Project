#pragma once

#include "Blueprint/UserWidget.h"
#include "TutorialSummaryWidget.generated.h"

class UTextBlock;

UCLASS()
class CR4S_API UTutorialSummaryWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Summary Data Setters
public:
	void SetDescription(const FString& InDescription);
	void SetCountText(const FString& CountText);

	void PlayCreateAnimation();
	void PlayCompleteAnimation();
#pragma endregion

#pragma region Widget Bindings
protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Description;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Count;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CreateObjAnim;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CompleteObjAnim;

#pragma endregion

public:
	bool bIsComplete = false;
};
