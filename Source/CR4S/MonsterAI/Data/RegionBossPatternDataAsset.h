#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MonsterEnum.h"
#include "RegionBossPatternDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FPhaseSkillSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EBossPhase Phase = EBossPhase::Normal;

	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> SkillTags;

	UPROPERTY(EditAnywhere)
	TArray<int32> SkillIndices;
};

USTRUCT(BlueprintType)
struct FRegionPatternData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint8 PatternID = 0;

	UPROPERTY(EditAnywhere)
	FGameplayTag BossTypeTag;
	
	UPROPERTY(EditAnywhere)
	TArray<FPhaseSkillSequence> PhaseSequences;

	UPROPERTY(EditAnywhere)
	float PatternWeight = 1.f;

	UPROPERTY(EditAnywhere)
	FVector2D DistanceRange = FVector2D(0.f, 99999.f);
};

UCLASS()
class CR4S_API URegionBossPatternDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRegionPatternData> PatternList;

	virtual void PostLoad() override;
	const TArray<int32>* GetSkillSequence(uint8 PatternID, EBossPhase Phase) const;

private:
	int32 ConvertSkillTagToIndex(const FGameplayTag& SkillTag, const FGameplayTag& BossTypeTag) const;
	
};
