#include "AnimalIKComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "FriendlyAI/Data/AnimalIKSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UAnimalIKComponent::UAnimalIKComponent()
{
   PrimaryComponentTick.bCanEverTick = true;
}

void UAnimalIKComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!IKSettings)
    {
        return;
    }

    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        return;
    }

    SkeletalMeshComponent = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComponent)
    {
        return;
    }

    FindFootBones();

    CurrentKneeBoneNames = IKSettings->ThighBoneNames;
        
    UpdateIK();
    
    GetWorld()->GetTimerManager().SetTimer(
        IKUpdateTimerHandle,
        this,
        &UAnimalIKComponent::UpdateIK,
        0.1f,
        true
    );
}

void UAnimalIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
   Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

   if (bShowDebug)
   {
       DrawDebugInfo();
   }
}

void UAnimalIKComponent::FindFootBones()
{
   if (!SkeletalMeshComponent || !IKSettings) return;

   FootBoneNames.Empty();
   FootIKData.Empty();

   for (const FName& BoneName : IKSettings->FootBoneNames)
   {
       if (SkeletalMeshComponent->GetBoneIndex(BoneName) != INDEX_NONE)
       {
           FootBoneNames.Add(BoneName);
           FootIKData.Add(BoneName, FIKFootData());
       }
   }
}

void UAnimalIKComponent::UpdateIK()
{
    if (!bEnableIK || !OwnerCharacter || !SkeletalMeshComponent || !IKSettings) return;
    
    if (OwnerCharacter->GetCharacterMovement() && OwnerCharacter->GetCharacterMovement()->IsFalling())
    {
        for (auto& FootPair : FootIKData)
        {
            FootPair.Value.Alpha = 0.0f;
        }
        return;
    }

    for (const FName& FootBoneName : FootBoneNames)
    {
       FVector HitLocation = PerformFootTrace(FootBoneName);
       
       if (FootIKData.Contains(FootBoneName))
       {
           FIKFootData& FootData = FootIKData[FootBoneName];
           
           if (HitLocation != FVector::ZeroVector)
           {
               FootData.bIsValidHit = true;
               FootData.EffectorLocation = FMath::VInterpTo(
                   FootData.EffectorLocation,
                   HitLocation,
                   0.1f,
                   IKSettings->IKInterpSpeed
               );
    
               FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(FootBoneName);
               FVector SurfaceNormal = FVector::UpVector;
    
               FootData.EffectorRotation = FMath::RInterpTo(
                   FootData.EffectorRotation,
                   UKismetMathLibrary::MakeRotFromZX(SurfaceNormal, OwnerCharacter->GetActorForwardVector()),
                   0.1f,
                   IKSettings->IKInterpSpeed
               );
           }
           else
           {
               FootData.bIsValidHit = false;
           }
       }
   }

   CalculateRootRotation();
   CalculateJointRotations();
   CalculateJointPositions();

    if (FootIKData.Contains(IKSettings->FootBoneNames[0]))
        FootIKData[IKSettings->FootBoneNames[0]].Alpha = FL_Alpha;
    if (FootIKData.Contains(IKSettings->FootBoneNames[1]))
        FootIKData[IKSettings->FootBoneNames[1]].Alpha = FR_Alpha;
    if (FootIKData.Contains(IKSettings->FootBoneNames[2]))
        FootIKData[IKSettings->FootBoneNames[2]].Alpha = BL_Alpha;
    if (FootIKData.Contains(IKSettings->FootBoneNames[3]))
        FootIKData[IKSettings->FootBoneNames[3]].Alpha = BR_Alpha;
}

FVector UAnimalIKComponent::PerformFootTrace(const FName& BoneName)
{
   if (!SkeletalMeshComponent || !IKSettings) return FVector::ZeroVector;

   FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(BoneName);
   FVector TraceStart = FootWorldLocation + FVector(0, 0, 50.0f);
   FVector TraceEnd = FootWorldLocation - FVector(0, 0, IKSettings->TraceDistance);

   FHitResult HitResult;
   FCollisionQueryParams QueryParams;
   QueryParams.AddIgnoredActor(OwnerCharacter);

   bool bHit = GetWorld()->LineTraceSingleByChannel(
       HitResult,
       TraceStart,
       TraceEnd,
       ECC_Visibility,
       QueryParams
   );

   if (bHit)
   {
       float HeightDifference = FootWorldLocation.Z - HitResult.Location.Z;

       if (FMath::Abs(HeightDifference) <= IKSettings->MaxFootHeight)
       {
           return HitResult.Location + FVector(0, 0, IKSettings->FootHeightOffset);
       }
   }

   return FVector::ZeroVector;
}

void UAnimalIKComponent::CalculateRootRotation()
{
    if (!SkeletalMeshComponent || !IKSettings) return;

    FVector FLPos = FVector::ZeroVector;
    FVector FRPos = FVector::ZeroVector; 
    FVector BLPos = FVector::ZeroVector;
    FVector BRPos = FVector::ZeroVector;
    bool bValidFoots = true;

    if (FootIKData.Contains(IKSettings->FootBoneNames[0]) && FootIKData[IKSettings->FootBoneNames[0]].bIsValidHit)
        FLPos = FootIKData[IKSettings->FootBoneNames[0]].EffectorLocation;
    else bValidFoots = false;

    if (FootIKData.Contains(IKSettings->FootBoneNames[1]) && FootIKData[IKSettings->FootBoneNames[1]].bIsValidHit)
        FRPos = FootIKData[IKSettings->FootBoneNames[1]].EffectorLocation;
    else bValidFoots = false;

    if (FootIKData.Contains(IKSettings->FootBoneNames[2]) && FootIKData[IKSettings->FootBoneNames[2]].bIsValidHit)
        BLPos = FootIKData[IKSettings->FootBoneNames[2]].EffectorLocation;
    else bValidFoots = false;

    if (FootIKData.Contains(IKSettings->FootBoneNames[3]) && FootIKData[IKSettings->FootBoneNames[3]].bIsValidHit)
        BRPos = FootIKData[IKSettings->FootBoneNames[3]].EffectorLocation;
    else bValidFoots = false;

    if (!bValidFoots) return;

    float FrontAvgHeight = (FLPos.Z + FRPos.Z) / 2.0f;
    float BackAvgHeight = (BLPos.Z + BRPos.Z) / 2.0f;
    float LeftAvgHeight = (FLPos.Z + BLPos.Z) / 2.0f;
    float RightAvgHeight = (FRPos.Z + BRPos.Z) / 2.0f;
    
    float Pitch = FMath::Atan2(RightAvgHeight- LeftAvgHeight, 100.0f) * (180.0f / PI);
    float Roll = FMath::Atan2(FrontAvgHeight - BackAvgHeight, 100.0f) * (180.0f / PI);
    
    Pitch = FMath::Clamp(Pitch, -IKSettings->MaxRootRotation, IKSettings->MaxRootRotation);
    Roll = FMath::Clamp(Roll, -IKSettings->MaxRootRotation, IKSettings->MaxRootRotation);

    FRotator TargetRotation = FRotator(Pitch, 0.0f, Roll);
    RootRotation = FMath::RInterpTo(RootRotation, TargetRotation, 0.1f, IKSettings->IKInterpSpeed);
}

void UAnimalIKComponent::CalculateJointRotations()
{
    if (!SkeletalMeshComponent || !IKSettings) return;
    
    FVector LeftThighHit = PerformFootTrace(IKSettings->ThighBoneNames[0]);
    FVector RightThighHit = PerformFootTrace(IKSettings->ThighBoneNames[1]);
    FVector LeftUpperArmHit = PerformFootTrace(IKSettings->CalfBoneNames[0]);
    FVector RightUpperArmHit = PerformFootTrace(IKSettings->CalfBoneNames[1]);
    
    float SideAngle = 0.0f;
    if (LeftThighHit != FVector::ZeroVector && RightThighHit != FVector::ZeroVector)
    {
        float HeightDiff = LeftThighHit.Z - RightThighHit.Z;
        SideAngle = FMath::Atan2(HeightDiff, 100.0f) * (180.0f / PI);
    }
    
    float ForwardAngle = 0.0f;
    if (LeftUpperArmHit != FVector::ZeroVector && LeftThighHit != FVector::ZeroVector)
    {
        float HeightDiff = LeftUpperArmHit.Z - LeftThighHit.Z;
        ForwardAngle = FMath::Atan2(HeightDiff, 100.0f) * (180.0f / PI);
    }

    LeftThighRotation = 0.0f;
    RightThighRotation = 0.0f;
    LeftCalfRotation = 0.0f;
    RightCalfRotation = 0.0f;
    LeftUpperArmRotation = 0.0f;
    RightUpperArmRotation = 0.0f;
    LeftForearmRotation = 0.0f;
    RightForearmRotation = 0.0f;
    
    if (SideAngle < 0)
    {
        float AbsAngle = FMath::Abs(SideAngle);
        LeftThighRotation = AbsAngle;
        LeftCalfRotation = -AbsAngle * 3.0f;
        LeftUpperArmRotation = AbsAngle;
        LeftForearmRotation = -AbsAngle * 3.0f;
    }

    if (SideAngle > 0)
    {
        float AbsAngle = FMath::Abs(SideAngle);
        RightThighRotation = AbsAngle;
        RightCalfRotation = -AbsAngle * 3.0f;
        RightUpperArmRotation = AbsAngle;
        RightForearmRotation = -AbsAngle * 3.0f;
    }
}

void UAnimalIKComponent::CalculateJointPositions()
{
    if (!SkeletalMeshComponent || !IKSettings) return;

    LeftThighPosition = SkeletalMeshComponent->GetBoneLocation(IKSettings->ThighBoneNames[0]);
    RightThighPosition = SkeletalMeshComponent->GetBoneLocation(IKSettings->ThighBoneNames[1]);
    LeftUpperArmPosition = SkeletalMeshComponent->GetBoneLocation(IKSettings->UpperArmBoneNames[0]);
    RightUpperArmPosition = SkeletalMeshComponent->GetBoneLocation(IKSettings->UpperArmBoneNames[1]);
}

void UAnimalIKComponent::DrawDebugInfo()
{
   if (!GetWorld() || !SkeletalMeshComponent || !IKSettings) return;

   for (const FName& FootBoneName : FootBoneNames)
   {
       FVector FootWorldLocation = SkeletalMeshComponent->GetBoneLocation(FootBoneName);
       FVector TraceStart = FootWorldLocation + FVector(0, 0, 50.0f);
       FVector TraceEnd = FootWorldLocation - FVector(0, 0, IKSettings->TraceDistance);

       DrawDebugLine(
           GetWorld(),
           TraceStart,
           TraceEnd,
           FColor::Yellow,
           false,
           -1.0f,
           0,
           2.0f
       );
       
       if (FootIKData.Contains(FootBoneName))
       {
           const FIKFootData& FootData = FootIKData[FootBoneName];
           if (FootData.bIsValidHit)
           {
               DrawDebugSphere(
                   GetWorld(),
                   FootData.EffectorLocation,
                   5.0f,
                   12,
                   FColor::Green,
                   false,
                   -1.0f,
                   0,
                   2.0f
               );
           }
       }
   }
}