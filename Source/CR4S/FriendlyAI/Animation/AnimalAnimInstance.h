#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FriendlyAI/Component/AnimalIKComponent.h"
#include "AnimalAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class ABaseAnimal;
class UAnimalIKComponent;
struct FIKFootData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIdleMontageEnded, UAnimMontage*, Montage, bool, bInterrupted);

UCLASS()
class CR4S_API UAnimalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION()
	void AnimNotify_AnimalAttackHit();

	UFUNCTION()
	void AnimNotify_AnimalDash();

	UFUNCTION()
	void AnimNotify_AnimalRanged();
	
protected:
	UPROPERTY()
	ABaseAnimal* OwnerAnimal;
	
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovementComponent;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	float GroundSpeed;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	uint8 bShouldMove : 1;

	UPROPERTY(visibleAnywhere, BlueprintReadOnly)
	uint8 bIsFalling : 1;

#pragma region Idle RandomAnim
public:
	UFUNCTION(BlueprintCallable, Category="Idle")
	bool PlayRandomIdleMontage();

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Idle")
	TArray<UAnimMontage*> IdleMontages;
	
	UPROPERTY(BlueprintAssignable, Category="Idle")
	FOnIdleMontageEnded OnIdleMontageEnded;
#pragma endregion

#pragma region IK Data
private:
	void UpdateIKData();
	
protected:
	UPROPERTY()
	class UAnimalIKComponent* AnimalIKComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	TMap<FName, FIKFootData> FootIKData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	FRotator RootRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	bool bIKDataValid = false;

#pragma endregion

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	TArray<FName> KneeBoneNames;
	
	UFUNCTION()
	void AnimNotify_FootPlant_FL_Begin();

	UFUNCTION()
	void AnimNotify_FootPlant_FL_End();

	UFUNCTION()
	void AnimNotify_FootPlant_FR_Begin();

	UFUNCTION()
	void AnimNotify_FootPlant_FR_End();

	UFUNCTION()
	void AnimNotify_FootPlant_BL_Begin();

	UFUNCTION()
	void AnimNotify_FootPlant_BL_End();

	UFUNCTION()
	void AnimNotify_FootPlant_BR_Begin();

	UFUNCTION()
	void AnimNotify_FootPlant_BR_End();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float LeftThighRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float RightThighRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float LeftCalfRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float RightCalfRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float LeftUpperArmRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float RightUpperArmRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float LeftForearmRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float RightForearmRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float FL_Alpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float FR_Alpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float BL_Alpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
	float BR_Alpha = 0.0f;
};
