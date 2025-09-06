#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "TutorialData.generated.h"

USTRUCT(BlueprintType)
struct FTutorialObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;     // "나무를 밴다."

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TargetCount;     // 2

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "Tutorial"))
    FGameplayTag TutorialTag;  // "Tutorial.Item.Tree"
};

USTRUCT(BlueprintType)
struct FTutorialReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;   // "Fur"

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount;   // 4
};

USTRUCT(BlueprintType)
struct FTutorialData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TutorialLevel;   // Lv1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ObjectiveSetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText TutorialName;    // [채집]

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTutorialObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTutorialReward> Rewards;
};
