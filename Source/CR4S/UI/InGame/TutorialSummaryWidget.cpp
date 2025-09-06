#include "UI/InGame/TutorialSummaryWidget.h"
#include "Components/TextBlock.h"

void UTutorialSummaryWidget::SetDescription(const FString& InDescription)
{
	if (Description)
	{
		Description->SetText(FText::FromString(InDescription));
	}
}

void UTutorialSummaryWidget::SetCountText(const FString& CountText)
{
	if (Count)
	{
		Count->SetText(FText::FromString(CountText));
	}
}

void UTutorialSummaryWidget::PlayCreateAnimation()
{
	SetVisibility(ESlateVisibility::Visible);

	if (CreateObjAnim)
	{
		PlayAnimation(CreateObjAnim);
	}
}

void UTutorialSummaryWidget::PlayCompleteAnimation()
{
	if (CompleteObjAnim && !bIsComplete)
	{
		PlayAnimation(CompleteObjAnim);

		bIsComplete = true;
	}
}
