#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterAI/Data/MonsterEnum.h"
#include "MonsterAI/Data/BossPhaseDataAsset.h"
#include "MonsterStateComponent.generated.h"

struct FMonsterAttributeRow;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMonsterStateChanged, EMonsterState, PreviousState, EMonsterState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EBossPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunChanged);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CR4S_API UMonsterStateComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Constructor & Override

public:	
	UMonsterStateComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion

#pragma region State Interface

public:
	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	void SetState(EMonsterState NewState);
	
	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	FORCEINLINE EMonsterState GetCurrentState() const { return CurrentState; }
	
	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	FORCEINLINE bool IsInState(EMonsterState State) const { return CurrentState == State; }

	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	FORCEINLINE float GetTimeInCurrentState() const { return StateElapsedTime; }

	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	float GetStateDuration(EMonsterState State) const;

	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	void ForceInterrupt();

	/* Broadcast delegate when state changes */
	UPROPERTY(BlueprintAssignable, Category = "Monster|State")
	FOnMonsterStateChanged OnStateChanged;

#pragma endregion

#pragma region State Data

private:
	EMonsterState CurrentState;
	EMonsterState PreviousState;
	float StateElapsedTime;
	TMap<EMonsterState, float> StateDuration;

#pragma endregion

#pragma region Boss Phase

public:
	UFUNCTION()
	void CheckPhaseTransition(float CurrentHP, float MaxHP);

	void SetPhase(EBossPhase NewPhase);
	FORCEINLINE EBossPhase GetCurrentPhase() const { return CurrentPhase; }
	FORCEINLINE float GetCurrentSpeedMultiplier() const { return CurrentSpeedMultiplier; }
	FORCEINLINE float GetCurrentDamageMultiplier() const { return CurrentDamageMultiplier; }

	UPROPERTY(EditAnywhere, Category = "Phase")
	TObjectPtr<UBossPhaseDataAsset> PhaseDataAsset;

	UPROPERTY(BlueprintAssignable, Category = "Monster|State")
	FOnPhaseChanged OnPhaseChanged;

protected:
	UPROPERTY(BlueprintReadOnly)
	EBossPhase CurrentPhase = EBossPhase::Normal;

	UPROPERTY(BlueprintReadOnly, Category = "Phase")
	float CurrentSpeedMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Phase")
	float CurrentDamageMultiplier = 1.0f;

#pragma endregion

#pragma region Stun
public:
	UFUNCTION(BlueprintCallable, Category="Monster|State")
	void InitializeStunData(const FMonsterAttributeRow& Data);
	void HandlePlayerDeath();

	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	void AddStun(float StunAmount);
    
	UFUNCTION(BlueprintCallable, Category = "Monster|State")
	void RemoveStunDebuff();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Monster|State")
	bool IsStunned() const { return bIsStunned; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Monster|State")
	float GetCurrentStunAmount() const { return MaxStun > 0.f ? CurrentStun / MaxStun : 0.f; }

	UPROPERTY(BlueprintAssignable)
	FOnStunStarted OnStunStarted;

	UPROPERTY(BlueprintAssignable)
	FOnStunEnded   OnStunEnded;

	UPROPERTY(BlueprintAssignable)
	FOnStunChanged OnStunChangedDelegate;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float MaxStun = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float StunningTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float StunDamageMultiplier = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float StunChargeStartDelay = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float StunRecoveryMin = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float StunRecoveryMax = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|State")
	float StunRecoveryRampUpTime = 3.f;
	UPROPERTY(BlueprintReadOnly, Category = "Monster|State")
	float CurrentStun = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "Monster|State")
	bool bIsStunned = false;

private:
	float RecoveryElapsedTime = 0.f;
	bool bCanRecover = false;
	
	FTimerHandle StunRecoveryTimerHandle;
	FTimerHandle RecoveryDelayTimerHandle;
	
#pragma endregion

private:
	FString MyHeader;

};
