#include "Game/System/TutorialManager.h"
#include "CR4S.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"

UTutorialManager::UTutorialManager()
{
    static ConstructorHelpers::FObjectFinder<UDataTable> DT_TutorialQuest(
        TEXT("/Game/CR4S/_Data/Game/DT_Tutorial.DT_Tutorial")
    );

    if (DT_TutorialQuest.Succeeded())
    {
        TutorialQuestTable = DT_TutorialQuest.Object;
        UE_LOG(LogTutorial, Log, TEXT("Tutorial DataTable loaded successfully."));
    }
    else
    {
        UE_LOG(LogTutorial, Warning, TEXT("Failed to load Tutorial DataTable."));
    }
}

bool UTutorialManager::ShouldCreateSubsystem(UObject* Outer) const
{
    UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;

    if (World->GetName() == TEXT("MenuLevel")) return false;
    if (!World->IsGameWorld()) return false;

    return true;
}

void UTutorialManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (!TutorialQuestTable)
    {
        UE_LOG(LogTutorial, Warning, TEXT("TutorialQuestTable is not found!"));
    }

    UE_LOG(LogTutorial, Log, TEXT("TutorialManager initialized."));
    StartTutorial("T001");
}

void UTutorialManager::StartTutorial(FName RowName)
{
    if (!TutorialQuestTable) return;

    static const FString Context(TEXT("TutorialQuest"));
    if (FTutorialData* QuestRow = TutorialQuestTable->FindRow<FTutorialData>(RowName, Context))
    {
        CurrentTutorial.Data = *QuestRow;
        CurrentTutorial.Progress.Empty();

        for (const auto& Obj : QuestRow->Objectives)
        {
            CurrentTutorial.Progress.Add(Obj.TutorialTag, 0);
            UE_LOG(LogTutorial, Log, TEXT("Objective added: %s (TargetCount: %d)"),
                *Obj.TutorialTag.ToString(), Obj.TargetCount);
        }

        UE_LOG(LogTutorial, Log, TEXT("Started Tutorial: %s (Level: %d, ObjectiveSetID: %d)"),
            *QuestRow->TutorialName.ToString(), QuestRow->TutorialLevel, QuestRow->ObjectiveSetID);

        OnTutorialProgressUpdated.Broadcast(CurrentTutorial);
    }
    else
    {
        UE_LOG(LogTutorial, Warning, TEXT("Tutorial Row not found! RowName: %s"), *RowName.ToString());
    }
}

void UTutorialManager::UpdateObjectiveProgress(FGameplayTag TutorialTag)
{
    if (!CurrentTutorial.Progress.Contains(TutorialTag))
    {
        UE_LOG(LogTutorial, Warning, TEXT("Attempted to update non-existing Objective: %s"),
            *TutorialTag.ToString());
        return;
    }

    CurrentTutorial.Progress[TutorialTag]++;
    UE_LOG(LogTutorial, Log, TEXT("Objective progressed: %s -> %d"),
        *TutorialTag.ToString(), CurrentTutorial.Progress[TutorialTag]);

    for (const auto& Obj : CurrentTutorial.Data.Objectives)
    {
        if (Obj.TutorialTag == TutorialTag && CurrentTutorial.Progress[TutorialTag] > Obj.TargetCount)
        {
            CurrentTutorial.Progress[TutorialTag] = Obj.TargetCount;
            UE_LOG(LogTutorial, Log, TEXT("Objective capped at TargetCount: %s -> %d"),
                *TutorialTag.ToString(), Obj.TargetCount);
        }
    }

    OnTutorialProgressUpdated.Broadcast(CurrentTutorial);

    if (CurrentTutorial.IsCompleted())
    {
        UE_LOG(LogTutorial, Log, TEXT("Tutorial Completed: %s (Level: %d, ObjectiveSetID: %d)"),
            *CurrentTutorial.Data.TutorialName.ToString(),
            CurrentTutorial.Data.TutorialLevel,
            CurrentTutorial.Data.ObjectiveSetID);

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                FName NextRowName = FName(*FString::Printf(TEXT("T%03d"),
                    CurrentTutorial.Data.ObjectiveSetID + 1));
                UE_LOG(LogTutorial, Log, TEXT("Attempting to start next tutorial: %s"),
                    *NextRowName.ToString());

                static const FString Context(TEXT("TutorialQuest"));
                if (FTutorialData* NextRow = TutorialQuestTable->FindRow<FTutorialData>(NextRowName, Context))
                {
                    StartTutorial(NextRowName);
                }
                else
                {
                    UE_LOG(LogTutorial, Log, TEXT("No next tutorial found. All tutorials completed."));
                }
            }, 2.0f, false);
    }
}


bool UTutorialManager::IsTutorialActive(const FGameplayTag& TutorialTag) const
{
    if (!CurrentTutorial.Data.Objectives.Num())
    {
        return false;
    }

    for (const auto& Obj : CurrentTutorial.Data.Objectives)
    {
        if (Obj.TutorialTag == TutorialTag)
        {
            return true;
        }
    }

    return false;
}

void UTutorialManager::Deinitialize()
{
    UE_LOG(LogTutorial, Log, TEXT("TutorialManager deinitialized."));
    Super::Deinitialize();
}
