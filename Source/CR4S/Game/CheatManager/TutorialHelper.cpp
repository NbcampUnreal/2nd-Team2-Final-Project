#include "Game/CheatManager/TutorialHelper.h"
#include "Game/System/TutorialManager.h"

void UTutorialHelper::IncrementObjective(FGameplayTag TutorialTag)
{
    if (UTutorialManager* TM = GetWorld()->GetSubsystem<UTutorialManager>())
    {
        TM->UpdateObjectiveProgress(TutorialTag);
    }
}
