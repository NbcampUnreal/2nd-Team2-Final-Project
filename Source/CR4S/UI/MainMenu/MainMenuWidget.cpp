#include "UI/MainMenu/MainMenuWidget.h"
#include "Game/GameInstance/C4GameInstance.h"
#include "UI/MainMenu/SettingsWidget.h"
#include "UI/MainMenu/CreditsWidget.h"
#include "UI/MainMenu/DifficultyOptionsWidget.h"
#include "UI/MainMenu/GameSaveWidget.h"
#include "UI/MainMenu/GameIntroWidget.h"
#include "UI/Common/ConfirmWidget.h"
#include "UI/Common/ButtonWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Game/System/AudioManager.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayIntroSequence();
}

void UMainMenuWidget::InitMainMenu()
{
	CreateChildWidgets();

	if (PlayGameButton)
	{
		PlayGameButton->OnClicked().AddDynamic(this, &UMainMenuWidget::OnPlayGameButtonClicked);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked().AddDynamic(this, &UMainMenuWidget::OnSettingsButtonClicked);
	}
	if (CreditsButton)
	{
		//CreditsButton->OnClicked().AddDynamic(this, &UMainMenuWidget::OnCreditsButtonClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked().AddDynamic(this, &UMainMenuWidget::OnQuitButtonClicked);
	}

	if (UAudioManager* AudioManager = GetGameInstance()->GetSubsystem<UAudioManager>())
	{
		AudioManager->PlayBGM(MainMenuBGM);
	}

	ShowMenuButtons();
	ShowBackground();
}

void UMainMenuWidget::CreateChildWidgets()
{
	if (!SettingsWidgetInstance && SettingsWidgetClass)
	{
		SettingsWidgetInstance = CreateWidget<USettingsWidget>(GetWorld(), SettingsWidgetClass);
	}
	if (!CreditsWidgetInstance && CreditsWidgetClass)
	{
		CreditsWidgetInstance = CreateWidget<UCreditsWidget>(GetWorld(), CreditsWidgetClass);
	}
	if (!DifficultyOptionsWidgetInstance && DifficultyOptionsWidgetClass)
	{
		DifficultyOptionsWidgetInstance = CreateWidget<UDifficultyOptionsWidget>(GetWorld(), DifficultyOptionsWidgetClass);
	}
	if(!GameSaveWidgetInstance && GameSaveWidgetClass)
	{
		GameSaveWidgetInstance = CreateWidget<UGameSaveWidget>(GetWorld(), GameSaveWidgetClass);
	}
	if (!ConfirmWidgetInstance && ConfirmWidgetClass)
	{
		ConfirmWidgetInstance = CreateWidget<UConfirmWidget>(GetWorld(), ConfirmWidgetClass);
	}

}

void UMainMenuWidget::OnPlayGameButtonClicked()
{
	if (GameSaveWidgetInstance)
	{
		if (!GameSaveWidgetInstance->IsInViewport())
		{
			GameSaveWidgetInstance->AddToViewport();
		}

		HideMenuButtons();
		GameSaveWidgetInstance->MainMenuWidgetRef = this;
		GameSaveWidgetInstance->OpenWindow();
	}
}

void UMainMenuWidget::OnSettingsButtonClicked()
{
	if (SettingsWidgetInstance)
	{
		if (!SettingsWidgetInstance->IsInViewport())
		{
			SettingsWidgetInstance->AddToViewport();	
		}

		HideMenuButtons();
		SettingsWidgetInstance->MainMenuWidgetRef = this;
		SettingsWidgetInstance->HandleOpenWindow();
	}
}

void UMainMenuWidget::OnCreditsButtonClicked()
{
	if (CreditsWidgetInstance)
	{
		CreditsWidgetInstance->AddToViewport(10);
		HideMenuButtons();
	}
}

void UMainMenuWidget::ShowMenuButtons()
{
	PlayAnimation(ShowButtonsAnim);
}


void UMainMenuWidget::HideMenuButtons()
{
	PlayAnimation(ShowButtonsAnim, ShowButtonsAnim->GetEndTime(), 1, EUMGSequencePlayMode::Reverse);
}

void UMainMenuWidget::SetWidgetVisibility(UUserWidget* Widget, ESlateVisibility InVisibility)
{
	if (Widget)
	{
		Widget->SetVisibility(InVisibility);
	}
}

void UMainMenuWidget::OnQuitButtonClicked()
{
	if (GEngine)
	{
		GEngine->GameViewport->GetWindow()->RequestDestroyWindow();
	}
}

void UMainMenuWidget::PlayIntroSequence()
{
	UC4GameInstance* MyGI = GetGameInstance<UC4GameInstance>();
	if (MyGI && !MyGI->bHasShownIntro)
	{
		MyGI->bHasShownIntro = true;

		if (IntroWidgetClass)
		{
			UGameIntroWidget* IntroWidget = CreateWidget<UGameIntroWidget>(this, IntroWidgetClass);
			if (IntroWidget)
			{
				IntroWidget->AddToViewport(100);
				IntroWidget->OnIntroFinished.BindUObject(this, &UMainMenuWidget::InitMainMenu);
			}
		}
	}
	else
	{
		InitMainMenu();
	}
}

void UMainMenuWidget::ShowBackground()
{
	if (BackgroundImages.Num() > 0)
	{
		UMaterialInterface* Material = Cast<UMaterialInterface>(BackgroundImage->Brush.GetResourceObject());
		if (Material)
		{
			BackgroundMID = UMaterialInstanceDynamic::Create(Material, this);
			BackgroundImage->SetBrushFromMaterial(BackgroundMID);

			BackgroundMID->SetTextureParameterValue("TextureA", BackgroundImages[0]);
			BackgroundMID->SetTextureParameterValue("TextureB", BackgroundImages[0]);
			BackgroundMID->SetScalarParameterValue("DissolveAlpha", 0.0f);
		}

		GetWorld()->GetTimerManager().SetTimer(
			DissolveTimerHandle, this,
			&UMainMenuWidget::ShowNextBackground,
			5.0f, true
		);
	}
}

void UMainMenuWidget::ShowNextBackground()
{
	if (BackgroundImages.Num() == 0 || !BackgroundMID) return;

	// ���� Ÿ�̸� Ŭ����
	GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);

	int32 NextIndex = (CurrentIndex + 1) % BackgroundImages.Num();

	BackgroundMID->SetTextureParameterValue("TextureA", BackgroundImages[CurrentIndex]);
	BackgroundMID->SetTextureParameterValue("TextureB", BackgroundImages[NextIndex]);

	float Duration = 1.5f; // ������ �ð�
	float Step = 0.05f;

	// ���� ���� ElapsedTime�� ��� ������ �ٲٰų� static ���� ����
	static float ElapsedTime = 0.0f;
	ElapsedTime = 0.0f; // ������ ���� �� �ʱ�ȭ

	FTimerDelegate DissolveStepDelegate;
	DissolveStepDelegate.BindLambda([this, Step, Duration]() mutable {
		ElapsedTime += Step;
		float Alpha = FMath::Clamp(ElapsedTime / Duration, 0.0f, 1.0f);
		if (BackgroundMID)
		{
			BackgroundMID->SetScalarParameterValue("DissolveAlpha", Alpha);
		}
		});

	// ������ ���� Ÿ�̸� ����
	GetWorld()->GetTimerManager().SetTimer(
		DissolveTimerHandle,
		DissolveStepDelegate,
		Step,
		true
	);

	// �����갡 ���� �� ������ Ÿ�̸� �ڵ鷯
	FTimerHandle StopTimer;
	GetWorld()->GetTimerManager().SetTimer(
		StopTimer,
		[this, NextIndex, Duration]() {
			// ������ Ÿ�̸� ����
			GetWorld()->GetTimerManager().ClearTimer(DissolveTimerHandle);

			if (BackgroundImages.IsValidIndex(NextIndex) && BackgroundMID)
			{
				BackgroundMID->SetTextureParameterValue("TextureA", BackgroundImages[NextIndex]);
				BackgroundMID->SetTextureParameterValue("TextureB", BackgroundImages[NextIndex]);
				BackgroundMID->SetScalarParameterValue("DissolveAlpha", 0.0f);
			}

			// �ε��� ����
			CurrentIndex = NextIndex;

			// ���⼭ �ٽ� 5�� �ڿ� ShowNextBackground ȣ���ϵ��� Ÿ�̸� �缳��
			GetWorld()->GetTimerManager().SetTimer(
				DissolveTimerHandle,
				this,
				&UMainMenuWidget::ShowNextBackground,
				5.0f,
				false
			);
		},
		Duration,
		false
	);
}



