#include "UI/InGame/GameOverWidget.h"
#include "UI/Common/ButtonWidget.h"
#include "UI/InGame/SurvivalHUD.h"
#include "Game/GameMode/C4BaseInGameMode.h"
#include "Kismet/GameplayStatics.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ToMenuButton)
	{
		ToMenuButton->OnClicked().AddDynamic(this, &UGameOverWidget::OnToMenuButtonClicked);
	}

	if (LoadSaveButton)
	{
		LoadSaveButton->OnClicked().AddDynamic(this, &UGameOverWidget::OnLoadSaveButtonClicked);
	}

}

void UGameOverWidget::HandleGameOver()
{
	PlayAnimation(ShowGameOverAnim);
}

void UGameOverWidget::OnToMenuButtonClicked()
{
	RequestLoadWidget();

	AC4BaseInGameMode* GM = Cast<AC4BaseInGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->ReturnToMenuWithDelay(1.0f);
	}
}

void UGameOverWidget::OnLoadSaveButtonClicked()
{
	RequestLoadWidget();

	AC4BaseInGameMode* GM = Cast<AC4BaseInGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->HandleLoadGame();
	}
}

void UGameOverWidget::RequestLoadWidget()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		ASurvivalHUD* HUD = Cast<ASurvivalHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->ShowLoading();
		}
	}
}
