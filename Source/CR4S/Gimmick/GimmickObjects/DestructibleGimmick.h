#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Gimmick/GimmickObjects/BaseGimmick.h"
#include "Game/Interface/TutorialNotifiable.h"
#include "DestructibleGimmick.generated.h"

class UObjectShakeComponent;
class ABaseDestructObject;
class UDestructibleComponent;

UCLASS(BlueprintType)
class CR4S_API ADestructibleGimmick : public ABaseGimmick, public ITutorialNotifiable
{
	GENERATED_BODY()

#pragma region ABaseGimmick Override
	
public:
	ADestructibleGimmick();

	virtual void BeginPlay() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetResources(const AActor* InventoryOwnerActor) const override;

	virtual void LoadGimmickSaveGameData_Implementation(const FGimmickSaveGameData& GimmickSaveGameData) override;
	
#pragma endregion
	
#pragma region UDestructibleComponent

public:
	UFUNCTION(BlueprintPure, Category = "DestructibleComponent|Health")
	void GetGimmickHealthData(bool& bOutSuccess, float& OutCurrentHealth, float& OutMaxHealth) const;
	UFUNCTION(BlueprintCallable, Category = "DestructibleComponent|Health")
	void SetGimmickHealthData(const float NewCurrentHealth, const float NewMaxHealth);
	
	FORCEINLINE void SetDestroyDelay(const float NewDelay) { DestroyDelay = NewDelay; }
	
protected:
	UFUNCTION()
	virtual void OnGimmickTakeDamage(AActor* DamageCauser, float DamageAmount, float CurrentHealth);
	UFUNCTION()
	virtual void OnGimmickDestroy(AActor* DamageCauser);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDestructibleComponent> DestructibleComponent;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Destroy")
	TSubclassOf<ABaseDestructObject> DestructObjectClass;
	
	UPROPERTY(EditAnywhere, Category = "Destroy", meta = (ClampMin = 0.0))
	float DestroyDelay;
	
#pragma endregion

#pragma region BonusDamage

private:
	void InitializeThresholds();

	void GetRequiredHitsForTool(const FGameplayTag& HeldToolTag, bool& bApplyBonusDamage, int32& OutHitCount) const;
	int32 CalculatedDamage(int32 RequiredHits) const;
	
	UPROPERTY(VisibleAnywhere, Category = "BonusDamage")
	int32 MaxHitCount;
	UPROPERTY(VisibleAnywhere, Category = "BonusDamage")
	int32 NextThresholdIndex;
	
	TArray<int32> HealthThresholds;
	
	TArray<TMap<FName, int32>> YieldResources;
	
#pragma endregion

#pragma region ShakeComponent

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UObjectShakeComponent> ShakeComponent;
	
#pragma endregion

#pragma region Sound

private:
	UPROPERTY()
	TObjectPtr<USoundBase> TakeDamageSound;
	UPROPERTY()
	TObjectPtr<USoundBase> DestroySound;
	
#pragma endregion
	
#pragma region ITutorialNotifiable

public:
	FGameplayTag GetTutorialTag() const override;

#pragma endregion
};
