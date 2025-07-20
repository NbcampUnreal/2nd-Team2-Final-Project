#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Game/Interface/Spawnable.h"
#include "Data/MonsterEnum.h"
#include "Utility/StunnableInterface.h"
#include "BaseMonster.generated.h"

class UBaseInventoryComponent;
class UMonsterAttributeComponent;
class UMonsterSkillComponent;
class UMonsterStateComponent;
class UMonsterAnimComponent;
class UBehaviorTree;

USTRUCT(BlueprintType)
struct FDropData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drops")
	FName DropItemRowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drops", meta=(ClampMin="1"))
	int32 DropItemCount = 1;
};

UCLASS()
class CR4S_API ABaseMonster : public ACharacter, public ISpawnable, public IStunnableInterface
{
	GENERATED_BODY()


#pragma region ACharacter Override

public:
	ABaseMonster();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

#pragma endregion

#pragma region BaseMonster Components

public:
	FORCEINLINE UMonsterAnimComponent* GetMonsterAnimComp() const { return AnimComponent;	}
	FORCEINLINE UMonsterSkillComponent* GetMonsterSkillComp() const { return SkillComponent;	}
	FORCEINLINE UMonsterAttributeComponent* GetMonsterAttribute() const { return AttributeComponent;	}
	FORCEINLINE UMonsterStateComponent* GetMonsterStateComp() const { return StateComponent;	}
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Component")
	TObjectPtr<UMonsterAnimComponent> AnimComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Component")
	TObjectPtr<UMonsterSkillComponent> SkillComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Component")
	TObjectPtr<UMonsterAttributeComponent> AttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Component")
	TObjectPtr<UMonsterStateComponent> StateComponent;

#pragma endregion

#pragma region Monster AI

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

#pragma endregion

#pragma region SKill Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	virtual void UseSkill(int32 SkillIndex);

	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	virtual int32 SelectSkillIndex();

#pragma endregion

#pragma region Monster Behavior - Death

public:
	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	bool IsDead() const;
	
	void StartFadeOut();
	void UpdateFade();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|State")
 	bool bIsDead = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster|ItemDrops")
	TArray<FDropData> DropItems;

private:
	UBaseInventoryComponent* GetPlayerInventory() const;
	void TryDropItems(UBaseInventoryComponent* Inv) const;
	
	FTimerHandle FadeTimerHandle;
	float ElapsedFadeTime = 0.f;

#pragma endregion

#pragma region Monster Components
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Monster|Attribute")
	FName MonsterID;

#pragma endregion

#pragma region Monster Event Handler

public:
	UFUNCTION()
	virtual void HandleDeath();

protected:
	UFUNCTION()
	virtual void OnMonsterStateChanged(EMonsterState Previous, EMonsterState Current);
	
	virtual FOnSpawnableDestroyed* GetOnSpawnableDestroyedDelegate() override { return &OnDied; }
	
	FOnSpawnableDestroyed OnDied;

#pragma endregion

#pragma region Accumulate Damage

public:
	FORCEINLINE float GetAccumulatedDamage() const { return AccumulatedDamage; }
	FORCEINLINE void AddAccumulatedDamage(float Damage) { AccumulatedDamage += Damage; }
	FORCEINLINE void ClearAccumulatedDamage() { AccumulatedDamage = 0.f; }

private:
	UPROPERTY()
	float AccumulatedDamage = 0.f;

#pragma endregion

#pragma region Stun
protected:
	virtual void TakeStun_Implementation(const float StunAmount) override;

#pragma endregion

#pragma region TakeDamage Effect
protected:
	UFUNCTION()
	void OnTakeDamageEvent(
		AActor* DamagedActor,
		float Damage,
		const UDamageType* DamageType,
		AController* InstigatedBy,
		AActor* DamageCauser
	);

	void FlashRed();
	void ResetFlash();
	void PlayHitSound();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Hit")
	TObjectPtr<USoundBase> HitSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Hit")
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Hit")
	float FlashDuration = 0.2f;

	FTimerHandle FlashTimerHandle;
	
#pragma endregion
	
private:
	FString MyHeader;

};
