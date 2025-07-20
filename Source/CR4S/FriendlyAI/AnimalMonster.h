#pragma once

#include "CoreMinimal.h"
#include "FriendlyAI/BaseAnimal.h"
#include "AnimalMonster.generated.h"

class AAnimalMonsterAIController;
class UAIJumpComponent;
class UAnimalIKComponent;

UCLASS()
class CR4S_API AAnimalMonster : public ABaseAnimal
{
	GENERATED_BODY()

#pragma region AActor Override
public:
	AAnimalMonster();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
#pragma endregion

#pragma region Initialization
protected:
	void InitializeMonster();
	void SetInitialTarget();
#pragma endregion

#pragma region Target Management
public:
	UFUNCTION(BlueprintCallable)
	void SetPrimaryTarget(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void SetSecondaryTarget(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void SetBuildingTarget(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void ClearAllTargets();
#pragma endregion

#pragma region Attack
public:
	virtual void PerformMeleeAttack() override;
	virtual void PerformChargeAttack() override;
#pragma endregion

#pragma region Damage
public:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
	                        AController* EventInstigator, AActor* DamageCauser) override;
#pragma endregion

#pragma region Monster Functions
public:
	// UFUNCTION(BlueprintCallable)
	// bool CanReachTarget(AActor* Target) const;

	UFUNCTION(BlueprintCallable)
	AActor* FindNearestDestructibleBuilding() const;

	UFUNCTION(BlueprintCallable)
	bool IsDestructibleBuilding(AActor* Building) const;

	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentPlayer() const;

	UFUNCTION(BlueprintCallable)
	AActor* GetCurrentPlayerTarget() const;

	UFUNCTION(BlueprintCallable)
	TArray<AActor*> FindNearbyAutomatedAI() const;

	UFUNCTION(BlueprintCallable)
	bool IsBuildingDestroyed(AActor* Building) const;
#pragma endregion

#pragma region Debug
public:
	void DrawMonsterDebugVisuals();
#pragma endregion

#pragma region Target Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "Monster|Target")
	AActor* PrimaryTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|Target") 
	AActor* SecondaryTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Monster|Target")
	AActor* BuildingTarget = nullptr;
#pragma endregion

#pragma region Attack Data
protected:
	UPROPERTY(EditAnywhere, Category = "Monster|Attack")
	float PlayerDamage = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Monster|Attack")
	float BuildingDamage = 100.0f;
#pragma endregion

#pragma region State Data
public:
	UPROPERTY(BlueprintReadOnly, Category = "Monster|State")
	bool bIsDestroyingBuilding = false;
#pragma endregion

#pragma region Detection Settings
protected:
	UPROPERTY(EditAnywhere, Category = "Monster|Detection")
	float AutomatedAIDetectionRange = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Monster|Detection")
	float BuildingDetectionRange = 2000.0f;
#pragma endregion

#pragma region Debug Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	uint8 bDrawPathDebug : 1 = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	uint8 bDrawTargetDebug : 1 = 0;
#pragma endregion

#pragma region Internal Functions
private:
	float GetDamageForTarget(AActor* Target) const;
	void ApplyDamageToTarget(AActor* Target);
#pragma endregion

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Jump")
	UAIJumpComponent* AIJumpComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	class UAnimalIKComponent* AnimalIKComponent;

};