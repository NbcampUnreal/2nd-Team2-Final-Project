#include "BTTask_AnimalIdle.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "FriendlyAI/Animation/AnimalAnimInstance.h"

UBTTask_AnimalIdle::UBTTask_AnimalIdle()
{
    NodeName = TEXT("Animal Idle");
    bNotifyTaskFinished = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_AnimalIdle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedOwnerComp = &OwnerComp;

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    ACharacter* Character = Cast<ACharacter>(Pawn);
    if (!Character) return EBTNodeResult::Failed;

    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (!Mesh) return EBTNodeResult::Failed;

    AnimalAnimInst = Cast<UAnimalAnimInstance>(Mesh->GetAnimInstance());
    if (!AnimalAnimInst) return EBTNodeResult::Failed;

    AnimalAnimInst->OnIdleMontageEnded.AddDynamic(
        this, &UBTTask_AnimalIdle::OnIdleMontageFinished);

    if (AnimalAnimInst->PlayRandomIdleMontage())
    {
        CurrentMontage = AnimalAnimInst->GetCurrentActiveMontage();
        //AnimalAnimInst->OnIdleMontageEnded.AddDynamic(this, &UBTTask_AnimalIdle::OnIdleMontageFinished);
        return EBTNodeResult::InProgress;
    }

    AnimalAnimInst->OnIdleMontageEnded.RemoveDynamic(
        this, &UBTTask_AnimalIdle::OnIdleMontageFinished);

    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_AnimalIdle::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AnimalAnimInst && CurrentMontage)
    {
        FName Section = AnimalAnimInst->Montage_GetCurrentSection(CurrentMontage);
        if (Section != TEXT("SitDown"))
        {
            AnimalAnimInst->Montage_JumpToSection(TEXT("StandUp"), CurrentMontage);
            return EBTNodeResult::InProgress;
        }
    }
    return EBTNodeResult::Aborted;
}

void UBTTask_AnimalIdle::OnIdleMontageFinished(UAnimMontage* Montage, bool bInterrupted)
{
    if (CachedOwnerComp)
    {
        FinishLatentTask(*CachedOwnerComp, bInterrupted ? EBTNodeResult::Aborted : EBTNodeResult::Succeeded);
    }

    if (AnimalAnimInst)
    {
        AnimalAnimInst->OnIdleMontageEnded.RemoveDynamic(
            this, &UBTTask_AnimalIdle::OnIdleMontageFinished);
    }
}
