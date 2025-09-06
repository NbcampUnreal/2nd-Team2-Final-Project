#pragma once

#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Game/System/TutorialManager.h"
#include "TutorialNotifiable.generated.h"

UINTERFACE(MinimalAPI)
class UTutorialNotifiable : public UInterface
{
    GENERATED_BODY()
};

/**
 * ITutorialNotifiable
 * - Interface for objects that can report tutorial progress
 * - Works with GameplayTags to identify tutorial objectives
 */
class CR4S_API ITutorialNotifiable
{
    GENERATED_BODY()

public:
    /**
     * Returns the tutorial tag associated with this object.
     * Must be implemented by any actor/component that reports tutorial progress.
     * @return FGameplayTag representing the tutorial objective.
     */
    virtual FGameplayTag GetTutorialTag() const { return FGameplayTag(); }

    /**
     * Safely notify TutorialManager about a tutorial objective.
     * This is a static helper, so it can be called without an instance of the interface.
     * @param WorldContext - Object used to retrieve UWorld.
     * @param TutorialTag - Gameplay tag representing the tutorial objective.
     *                    Should be valid (non-empty) to avoid crashes.
     */
    static void NotifyObjective(UObject* WorldContext, const FGameplayTag& TutorialTag)
    {
        if (!IsValid(WorldContext)) { return; }

        UWorld* World = WorldContext->GetWorld();
        if (!IsValid(World)) { return; }

        if (UTutorialManager* TM = World->GetSubsystem<UTutorialManager>())
        {
            if (TM->IsTutorialActive(TutorialTag))
            {
                TM->UpdateObjectiveProgress(TutorialTag);
            }
        }
    }
};