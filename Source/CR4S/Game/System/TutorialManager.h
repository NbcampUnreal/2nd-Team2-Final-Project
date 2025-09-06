#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "Game/Data/TutorialData.h"
#include "GameplayTagContainer.h"
#include "TutorialManager.generated.h"

USTRUCT(BlueprintType)
struct FActiveTutorial
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FTutorialData Data;

    // FName ¡æ FGameplayTag ·Î º¯°æ
    UPROPERTY(BlueprintReadOnly)
    TMap<FGameplayTag, int32> Progress;

    bool IsCompleted() const
    {
        for (const auto& Obj : Data.Objectives)
        {
            if (!Progress.Contains(Obj.TutorialTag)) return false;
            if (Progress[Obj.TutorialTag] < Obj.TargetCount) return false;
        }
        return true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTutorialProgressUpdated, const FActiveTutorial&, CurrentTutorial);

UCLASS()
class CR4S_API UTutorialManager : public UWorldSubsystem
{
    GENERATED_BODY()

#pragma region Subsystem Setup

protected:
    UTutorialManager();

    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

#pragma endregion

#pragma region Getters and Checkers
public:
    FORCEINLINE FActiveTutorial GetActiveTutorial() const { return CurrentTutorial; }

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool IsTutorialActive(const FGameplayTag& TutorialTag) const;

#pragma endregion 

#pragma region Tutorial Management

public:
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial(FName RowName);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void UpdateObjectiveProgress(FGameplayTag TutorialTag);

private:
    UPROPERTY()
    FActiveTutorial CurrentTutorial;

    UPROPERTY()
    UDataTable* TutorialQuestTable;

#pragma endregion

#pragma region Delegates
public:
    UPROPERTY(BlueprintAssignable, Category = "Tutorial")
    FTutorialProgressUpdated OnTutorialProgressUpdated;

#pragma endregion
};
