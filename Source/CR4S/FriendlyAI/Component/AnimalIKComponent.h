#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "AnimalIKComponent.generated.h"

USTRUCT(BlueprintType)
struct FIKFootData
{
   GENERATED_BODY()

   UPROPERTY(BlueprintReadOnly)
   FVector EffectorLocation = FVector::ZeroVector;

   UPROPERTY(BlueprintReadOnly)
   FRotator EffectorRotation = FRotator::ZeroRotator;

   UPROPERTY(BlueprintReadOnly)
   float Alpha = 0.0f;

   UPROPERTY(BlueprintReadOnly)
   bool bIsValidHit = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CR4S_API UAnimalIKComponent : public UActorComponent
{
   GENERATED_BODY()

public:
   UAnimalIKComponent();
   virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
   
protected:
   virtual void BeginPlay() override;

private:
   void UpdateIK();
   void FindFootBones();
   void CalculateRootRotation();
   void CalculateJointRotations();
   void CalculateJointPositions();
   void DrawDebugInfo();

   FVector PerformFootTrace(const FName& BoneName);
   
public:   
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
   class UAnimalIKSettings* IKSettings = nullptr;
   
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
   bool bEnableIK = true;
   
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
   bool bShowDebug = false;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   TMap<FName, FIKFootData> FootIKData;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   FRotator RootRotation = FRotator::ZeroRotator;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   TArray<FName> CurrentKneeBoneNames;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float LeftThighRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float RightThighRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float LeftCalfRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float RightCalfRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float LeftUpperArmRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float RightUpperArmRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float LeftForearmRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float RightForearmRotation = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   FVector LeftThighPosition = FVector::ZeroVector;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   FVector RightThighPosition = FVector::ZeroVector;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   FVector LeftUpperArmPosition = FVector::ZeroVector;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   FVector RightUpperArmPosition = FVector::ZeroVector;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float FL_Alpha = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float FR_Alpha = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float BL_Alpha = 0.0f;

   UPROPERTY(BlueprintReadOnly, Category = "IK Data")
   float BR_Alpha = 0.0f;

private:
   UPROPERTY()
   class ACharacter* OwnerCharacter;

   UPROPERTY()
   class USkeletalMeshComponent* SkeletalMeshComponent;

   TArray<FName> FootBoneNames;
   
   FTimerHandle IKUpdateTimerHandle;
};