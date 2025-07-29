#include "MonsterAI/Data/RegionBossPatternDataAsset.h"
#include "CR4s/CR4S.h"

void URegionBossPatternDataAsset::PostLoad()
{
	Super::PostLoad();

	UE_LOG(LogMonster, Warning, TEXT("[RegionBossPatternDataAsset] PostLoad called for %s"), *GetName());
	
	for (FRegionPatternData& Pattern : PatternList)
	{
		for (FPhaseSkillSequence& Sequence : Pattern.PhaseSequences)
		{
			Sequence.SkillIndices.Reset();

			for (const FGameplayTag& SkillTag : Sequence.SkillTags)
			{
				int32 Index = ConvertSkillTagToIndex(SkillTag, Pattern.BossTypeTag);
				if (Index >= 0)
				{
					Sequence.SkillIndices.Add(Index);
				}
			}

			if (Sequence.SkillIndices.Num() == 0)
			{
				UE_LOG(LogMonster, Error, TEXT("[RegionBossPatternDataAsset] SkillIndices is empty!"));
			}
		}
	}
}

const TArray<int32>* URegionBossPatternDataAsset::GetSkillSequence(uint8 PatternID, EBossPhase Phase) const
{
	// Find Pattern Data
	const FRegionPatternData* Pattern = PatternList.FindByPredicate(
		[PatternID](const FRegionPatternData& Data)
		{
			return Data.PatternID == PatternID;
		});
	
	if (!Pattern)
	{
		UE_LOG(LogMonster, Warning, TEXT("[RegionBossPatternDataAsset] PatternID %d not found."), PatternID);
		return nullptr;
	}
	
	// Find Skill Sequence for Current Phase
	const FPhaseSkillSequence* Sequence = Pattern->PhaseSequences.FindByPredicate(
		[Phase](const FPhaseSkillSequence& Seq)
		{
			return Seq.Phase == Phase;
		});
	
	if (!Sequence)
	{
		UE_LOG(LogMonster, Warning, TEXT("[RegionBossPatternDataAsset] Phase %d not found in Pattern %d."), static_cast<uint8>(Phase), PatternID);
		return nullptr;
	}
	
	return &Sequence->SkillIndices;
}

int32 URegionBossPatternDataAsset::ConvertSkillTagToIndex(const FGameplayTag& SkillTag,
	const FGameplayTag& BossTypeTag) const
{
	if (BossTypeTag.MatchesTag(FGameplayTag::RequestGameplayTag("RegionBossType.Kamish")))
	{
		static const TMap<FGameplayTag, int32> KamishSkillMap = {
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.ThunderRush"), 0},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.KneelDash"), 1},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.ThunderLeap"), 2},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.FlyingHammer"), 3},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.RetrieveHammer"), 4},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.RhythmStrike"), 5},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.HeavyStrike"), 6},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.HeavyLeftSwing"), 7},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.HeavyRightSwing"), 8},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.LightRightSwing"), 9},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.LightLeftSwing"), 10},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.LightSlam"), 11},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.LightUpper"), 12},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Normal.DashAndLeap"), 13},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.ThunderRush"), 14},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.KneelDash"), 15},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.ThunderLeap"), 16},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.DashAndLeap"), 17},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.FlyingHammer"), 18},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.RetrieveHammer"), 19},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.RhythmStrike"), 20},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.HeavyStrike"), 21},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.HeavyLeftSwing"), 22},
			{FGameplayTag::RequestGameplayTag("Skill.Kamish.Berserk.HeavyRightSwing"), 23}
		};

		if (const int32* Index = KamishSkillMap.Find(SkillTag))
		{
			return *Index;
		}
	}
	else if (BossTypeTag.MatchesTag(FGameplayTag::RequestGameplayTag("RegionBossType.LavaBrute")))
	{
		static const TMap<FGameplayTag, int32> LavaBruteSkillMap = {
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.ThrowingStones"), 0},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.EarthRumbling"), 1},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.ShoulderTackle"), 2},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.LavaBreath"), 3},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.CreateLava"), 4},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.GroundSmash"), 5},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.SmashTwice"), 6},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.HeavyPushLA"), 7},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.HeavySlapHand"), 8},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.HeavySmash"), 9},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.LightSpike"), 10},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.LightSwingLA"), 11},
			{FGameplayTag::RequestGameplayTag("Skill.LavaBrute.Normal.LightSwingRA"), 12}
		};

		if (const int32* Index = LavaBruteSkillMap.Find(SkillTag))
		{
			return *Index;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[RegionBossPatternDataAsset] SkillTag %s not found."), *SkillTag.ToString());
	return -1;
}
