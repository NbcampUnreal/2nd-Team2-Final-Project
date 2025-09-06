#include "UI/InGame/TutorialWidget.h"
#include "UI/InGame/TutorialSummaryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

void UTutorialWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UTutorialManager* TM = GetWorld()->GetSubsystem<UTutorialManager>())
    {
        TM->OnTutorialProgressUpdated.AddDynamic(this, &UTutorialWidget::UpdateTutorial);

        UpdateTutorial(TM->GetActiveTutorial());
    }
}

void UTutorialWidget::UpdateTutorial(const FActiveTutorial& CurrentTutorial)
{
    if (!TutorialContent) return;

    if (CurrentTutorial.Data.ObjectiveSetID == CurrentObjectiveSetID)
    {
        for (const auto& Obj : CurrentTutorial.Data.Objectives)
        {
            if (UTutorialSummaryWidget** FoundWidget = ObjectiveWidgets.Find(Obj.TutorialTag))
            {
                int32 CurrentCount = 0;
                if (CurrentTutorial.Progress.Contains(Obj.TutorialTag))
                {
                    CurrentCount = CurrentTutorial.Progress[Obj.TutorialTag];
                }

                FString CountText = FString::Printf(TEXT("%d / %d"), CurrentCount, Obj.TargetCount);
                (*FoundWidget)->SetCountText(CountText);

                if (CurrentCount >= Obj.TargetCount)
                {
                    (*FoundWidget)->PlayCompleteAnimation();
                }
            }
        }
        return;
    }

    ClearTutorials();
    ObjectiveWidgets.Empty();
    CurrentObjectiveSetID = CurrentTutorial.Data.ObjectiveSetID;

    if (StepNumber)
    {
        StepNumber->SetText(FText::AsNumber(CurrentTutorial.Data.TutorialLevel));
    }

    for (const auto& Obj : CurrentTutorial.Data.Objectives)
    {
        int32 CurrentCount = 0;
        if (CurrentTutorial.Progress.Contains(Obj.TutorialTag))
        {
            CurrentCount = CurrentTutorial.Progress[Obj.TutorialTag];
        }

        FString CountText = FString::Printf(TEXT("%d / %d"), CurrentCount, Obj.TargetCount);

        UTutorialSummaryWidget* Widget = AddObjective(Obj.Description.ToString(), CountText);
        if (Widget)
        {
            ObjectiveWidgets.Add(Obj.TutorialTag, Widget);
        }
    }
}

UTutorialSummaryWidget* UTutorialWidget::AddObjective(const FString& Description, const FString& CountText)
{
    if (!TutorialSummaryWidgetClass || !TutorialContent)
    {
        return nullptr;
    }

    UTutorialSummaryWidget* TutorialSummaryWidget = CreateWidget<UTutorialSummaryWidget>(GetWorld(), TutorialSummaryWidgetClass);
    if (TutorialSummaryWidget)
    {
        TutorialSummaryWidget->SetVisibility(ESlateVisibility::Hidden);
        TutorialSummaryWidget->SetDescription(Description);
        TutorialSummaryWidget->SetCountText(CountText);
        TutorialContent->AddChildToVerticalBox(TutorialSummaryWidget);
        TutorialSummaryWidget->PlayCreateAnimation();
    }
    return TutorialSummaryWidget;
}

void UTutorialWidget::ClearTutorials()
{
    TutorialContent->ClearChildren();
}

void UTutorialWidget::NativeDestruct()
{
    Super::NativeDestruct();
}
