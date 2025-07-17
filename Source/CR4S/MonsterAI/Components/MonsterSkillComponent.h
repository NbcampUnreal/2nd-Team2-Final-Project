#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CR4S/MonsterAI/Data/MonsterSkillData.h"
#include "MonsterSkillComponent.generated.h"

class USkillDataSubsystem;
class UPrimitiveComponent;
class AHiemsIceSpike;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CR4S_API UMonsterSkillComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Constructor
public:	
	UMonsterSkillComponent();

#pragma endregion

#pragma region Initialize
public:
	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	void InitializeMonsterSkills(const FName MonsterID);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma endregion

#pragma region Skill Usage
public:
	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	void UseSkill(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	bool IsSkillReady(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	float GetSkillRange(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	TArray<int32> GetAvailableSkillIndices() const;

	UFUNCTION(BlueprintCallable)
	const FMonsterSkillData& GetCurrentSkillData() const;

	UFUNCTION(BlueprintCallable)
	const FMonsterSkillData& GetSkillData(int32 Index) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Monster|Skill")
	int32 CurrentSkillIndex = INDEX_NONE;

protected:
	UFUNCTION()
	void OnSkillActorSpawned(AActor* NewActor);
	void CancelAllSkills();
	
	FDelegateHandle SpawnHandle;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> ActiveSkillActors;

#pragma endregion

#pragma region Helpers
private:
	UAnimInstance* GetAnimInstance() const;
	void ResetCooldown(int32 Index);

#pragma endregion

#pragma region Properties
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Skill")
	TArray<FMonsterSkillData> SkillList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster|Skill")
	TArray<bool> bSkillReady;

protected:
	TArray<FTimerHandle> SkillCooldownTimers;

#pragma endregion


#pragma region Skill Collision Settings

public:
	UFUNCTION(BlueprintCallable, Category = "Monster|Skill")
	void SetAttackCollisionEnabled(bool bEnable);
	static void RestoreOriginalCollisionProfile(UPrimitiveComponent* Collider);

	UFUNCTION()
	void OnAttackHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<TObjectPtr<UPrimitiveComponent>> BodyColliders;

	UPROPERTY()
	TSet<AActor*> AlreadyHitActors;

private:
	bool bIsAttackActive = false;

#pragma endregion
		
private:
	FString MyHeader;

};
