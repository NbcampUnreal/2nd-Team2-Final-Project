#pragma once

#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButtonWidget;

UCLASS()
class CR4S_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()
	
#pragma region Constructors & Initiailizers

	virtual void NativeConstruct() override;

#pragma endregion

#pragma region Widget Bindings
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButtonWidget> ToMenuButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButtonWidget> LoadSaveButton;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowGameOverAnim;

#pragma endregion

public: 
	void HandleGameOver();

protected:
	UFUNCTION()
	void OnToMenuButtonClicked();
	UFUNCTION()
	void OnLoadSaveButtonClicked();

private:
	void RequestLoadWidget();
};
