#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AnimalIKSettings.generated.h"

UCLASS(BlueprintType)
class CR4S_API UAnimalIKSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    TArray<FName> FootBoneNames = {
        TEXT("VB VB_FL"),
        TEXT("VB VB_FR"), 
        TEXT("VB VB_BL"),
        TEXT("VB VB_BR")
    };
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    TArray<FName> ThighBoneNames = {
        TEXT("thigh_l"),
        TEXT("thigh_r")
    };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    TArray<FName> CalfBoneNames = {
        TEXT("calf_l"),
        TEXT("calf_r")
    };
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    TArray<FName> UpperArmBoneNames = {
        TEXT("upperarm_l"),
        TEXT("upperarm_r")
    };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    TArray<FName> ForearmBoneNames = {
        TEXT("forearm_l"),
        TEXT("forearm_r")
    };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    FName PelvisBoneName = TEXT("pelvis");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bones")
    FName RootBoneName = TEXT("root");
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IKAlpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float IKInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float TraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float MaxFootHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float FootHeightOffset = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float MaxRootRotation = 15.0f;
};