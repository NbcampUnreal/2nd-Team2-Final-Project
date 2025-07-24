#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterAI/Data/MonsterAttributeRow.h"
#include "MonsterAttributeComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterHPChanged, float, CurrentHP, float, MaxHP);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CR4S_API UMonsterAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMonsterAttributeComponent();

#pragma region MonsterAttribute
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void InitializeMonsterAttribute(const FName MonsterID);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ApplyHeal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool IsDead() const;

	/* Broadcasts on monster death */
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnMonsterDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnMonsterHPChanged OnHPChanged;
	
	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }
	FORCEINLINE void SetCurrentHP(const float NewHealth);
	FORCEINLINE float GetMaxHP() const { return CurrentAttribute.MaxHP; }
	FORCEINLINE void RecoverToFull() { CurrentHP = CurrentAttribute.MaxHP; }

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	FMonsterAttributeRow GetMonsterAttribute() const { return CurrentAttribute; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FMonsterAttributeRow CurrentAttribute;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float CurrentHP;
#pragma endregion

protected:
	virtual void BeginPlay() override;

private:
	void ChangeHP(float Delta);
	FString MyHeader;
	
};
