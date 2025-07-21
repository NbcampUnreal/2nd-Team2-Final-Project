#include "AnimalAnimInstance.h"
#include "../BaseAnimal.h"
#include "FriendlyAI/Component/AnimalIKComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAnimalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(GetOwningActor());
	if (IsValid(OwnerCharacter))
	{
		OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
		OwnerAnimal = Cast<ABaseAnimal>(OwnerCharacter);
	}

	if (OwnerAnimal)
	{
		AnimalIKComponent = OwnerAnimal->FindComponentByClass<UAnimalIKComponent>();
	}
}

void UAnimalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* PawnOwner = TryGetPawnOwner();
	if (!PawnOwner)
	{
		GroundSpeed = 0.f;
		bShouldMove = false;
		return;
	}

	OwnerCharacter = Cast<ABaseAnimal>(PawnOwner);
	if (!OwnerCharacter)
	{
		GroundSpeed = 0.f;
		bShouldMove = false;
		return;
	}

	OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!OwnerCharacterMovementComponent)
	{
		GroundSpeed = 0.f;
		bShouldMove = false;
		return;
	}

	FVector HorizontalVelocity = OwnerCharacter->GetVelocity();
	HorizontalVelocity.Z = 0.f;
	GroundSpeed = HorizontalVelocity.Size();
	bShouldMove = GroundSpeed > 3.f && !OwnerCharacterMovementComponent->IsFalling();

	UpdateIKData();
}

void UAnimalAnimInstance::AnimNotify_AnimalAttackHit()
{
	if (OwnerAnimal)
	{
		OwnerAnimal->PerformMeleeAttack();
	}
}

void UAnimalAnimInstance::AnimNotify_AnimalDash()
{
	if (OwnerAnimal)
	{
		OwnerAnimal->PerformChargeAttack();
	}
}

void UAnimalAnimInstance::AnimNotify_AnimalRanged()
{
	if (OwnerAnimal)
	{
		OwnerAnimal->PerformRangedAttack();
	}
}

bool UAnimalAnimInstance::PlayRandomIdleMontage()
{
	if (Montage_IsPlaying(nullptr))
	{
		Montage_Stop(0.2f);
	}
    
	if (IdleMontages.Num() == 0) 
	{
		return false;
	}

	int32 Index = UKismetMathLibrary::RandomIntegerInRange(0, IdleMontages.Num() - 1);
	UAnimMontage* Chosen = IdleMontages[Index];

	if (!Chosen) 
	{
		return false;
	}

	float Duration = Montage_Play(Chosen);
    
	if (Duration > 0.f)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UAnimalAnimInstance::HandleMontageEnded);
        
		Montage_SetEndDelegate(EndDelegate, Chosen);
		return true;
	}
	return false;
}

void UAnimalAnimInstance::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{          
	OnIdleMontageEnded.Broadcast(Montage, bInterrupted);
}

void UAnimalAnimInstance::UpdateIKData()
{
	if (!AnimalIKComponent)
	{
		return;
	}
	
	if (!OwnerCharacter)
	{
		return;
	}
	
	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
	{
		return;
	}
	
	FName IKBoneName = TEXT("DeerDoe_-R-Finger0");
	int32 BoneIndex = MeshComp->GetBoneIndex(IKBoneName);
	if (AnimalIKComponent && AnimalIKComponent->bEnableIK)
	{
		FootIKData = AnimalIKComponent->FootIKData;
		RootRotation = AnimalIKComponent->RootRotation;
		KneeBoneNames = AnimalIKComponent->CurrentKneeBoneNames;
		bIKDataValid = true;
		
		FName TargetKey = TEXT("VB VB_FR");
		if (FootIKData.Contains(TargetKey))
		{
			const FIKFootData& RightFront = FootIKData[TargetKey];
		}

		if (KneeBoneNames.IsValidIndex(1))
		{
			FName KneeBone = KneeBoneNames[1];
			int32 KneeBoneIndex = MeshComp->GetBoneIndex(KneeBone);
			
			if (KneeBoneIndex != INDEX_NONE)
			{
				FVector KneeLocation = MeshComp->GetSocketLocation(KneeBone);
			}
		}

		FName RootBone = TEXT("DeerDoe_-Pelvis");
		FName MidBone = KneeBoneNames.IsValidIndex(1) ? KneeBoneNames[1] : TEXT("UNKNOWN");
		FName EndBone = IKBoneName;
		
		int32 RootIndex = MeshComp->GetBoneIndex(RootBone);
		int32 MidIndex = MeshComp->GetBoneIndex(MidBone);
		int32 EndIndex = MeshComp->GetBoneIndex(EndBone);
		
		if (EndIndex != INDEX_NONE)
		{
			FVector CurrentBoneLocation = MeshComp->GetSocketLocation(EndBone);
			
			if (FootIKData.Contains(TargetKey))
			{
				FVector TargetLocation = FootIKData[TargetKey].EffectorLocation;
				float Distance = FVector::Dist(CurrentBoneLocation, TargetLocation);
			}
		}
		
		LeftThighRotation = AnimalIKComponent->LeftThighRotation;
		
		AnimalIKComponent->FL_Alpha = FL_Alpha;
		AnimalIKComponent->FR_Alpha = FR_Alpha;
		AnimalIKComponent->BL_Alpha = BL_Alpha;
		AnimalIKComponent->BR_Alpha = BR_Alpha;
		RightThighRotation = AnimalIKComponent->RightThighRotation;
		LeftCalfRotation = AnimalIKComponent->LeftCalfRotation;
		RightCalfRotation = AnimalIKComponent->RightCalfRotation;
		LeftUpperArmRotation = AnimalIKComponent->LeftUpperArmRotation;
		RightUpperArmRotation = AnimalIKComponent->RightUpperArmRotation;
		LeftForearmRotation = AnimalIKComponent->LeftForearmRotation;
		RightForearmRotation = AnimalIKComponent->RightForearmRotation;
	}
	else
	{
		FootIKData.Empty();
		RootRotation = FRotator::ZeroRotator;
		KneeBoneNames.Empty();
		bIKDataValid = false;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_FL_Begin()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->FL_Alpha = 1.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_FL_End()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->FL_Alpha = 0.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_FR_Begin()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->FR_Alpha = 1.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_FR_End()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->FR_Alpha = 0.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_BL_Begin()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->BL_Alpha = 1.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_BL_End()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->BL_Alpha = 0.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_BR_Begin()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->BR_Alpha = 1.0f;
	}
}

void UAnimalAnimInstance::AnimNotify_FootPlant_BR_End()
{
	if (AnimalIKComponent)
	{
		AnimalIKComponent->BR_Alpha = 0.0f;
	}
}