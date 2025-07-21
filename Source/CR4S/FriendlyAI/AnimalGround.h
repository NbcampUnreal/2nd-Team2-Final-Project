#pragma once
#include "CoreMinimal.h"
#include "BaseAnimal.h"
#include "AnimalGround.generated.h"

class UAIJumpComponent;
class UAnimalIKComponent;

UCLASS()
class CR4S_API AAnimalGround : public ABaseAnimal
{
	GENERATED_BODY()
	
public:
	AAnimalGround();
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	UAIJumpComponent* AIJumpComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	class UAnimalIKComponent* AnimalIKComponent;

#pragma region Attack
	virtual float PlayAttackMontage() override;
	virtual float PlayChargeAttackMontage() override;
	virtual float PlayRangedAttackMontage() override;
	virtual void ResetAttackFlag() override;
	virtual void PerformMeleeAttack() override;
	virtual void PerformChargeAttack() override;
	
#pragma endregion
};
