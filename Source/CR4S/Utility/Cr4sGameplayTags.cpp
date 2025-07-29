// Fill out your copyright notice in the Description page of Project Settings.


#include "Cr4sGameplayTags.h"

namespace WeaponTags
{
	UE_DEFINE_GAMEPLAY_TAG(Melee,FName(TEXTVIEW("Weapon.Melee")));
	UE_DEFINE_GAMEPLAY_TAG(Ranged,FName(TEXTVIEW("Weapon.Ranged")));
	
	// Melee
	UE_DEFINE_GAMEPLAY_TAG(Chainsaw, FName(TEXT("Weapon.Melee.Chainsaw")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalSword, FName(TEXT("Weapon.Melee.CrystalSword")));
	UE_DEFINE_GAMEPLAY_TAG(DemolitionGear, FName(TEXT("Weapon.Melee.DemolitionGear")));
	UE_DEFINE_GAMEPLAY_TAG(ShockBat, FName(TEXT("Weapon.Melee.ShockBat")));

	// Ranged
	UE_DEFINE_GAMEPLAY_TAG(CrystalShotgun, FName(TEXT("Weapon.Ranged.CrystalShotgun")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalRifle, FName(TEXT("Weapon.Ranged.CrystalRifle")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalBurstRifle, FName(TEXT("Weapon.Ranged.CrystalBurstRifle")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalGatling, FName(TEXT("Weapon.Ranged.CrystalGatling")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalSMG, FName(TEXT("Weapon.Ranged.CrystalSMG")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalLauncher2, FName(TEXT("Weapon.Ranged.CrystalLauncher2")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalHomingLauncher4, FName(TEXT("Weapon.Ranged.CrystalHomingLauncher4")));
	UE_DEFINE_GAMEPLAY_TAG(CrystalHomingHighSpeed4, FName(TEXT("Weapon.Ranged.CrystalHomingHighSpeed4")));
	UE_DEFINE_GAMEPLAY_TAG(Fireball, FName(TEXT("Weapon.Ranged.Fireball")));
	UE_DEFINE_GAMEPLAY_TAG(HomingFireball, FName(TEXT("Weapon.Ranged.HomingFireball")));
	UE_DEFINE_GAMEPLAY_TAG(IceShotgun, FName(TEXT("Weapon.Ranged.IceShotgun")));
	UE_DEFINE_GAMEPLAY_TAG(ThunderStrike, FName(TEXT("Weapon.Ranged.ThunderStrike")));
	UE_DEFINE_GAMEPLAY_TAG(Comet, FName(TEXT("Weapon.Ranged.Comet")));
}

namespace ToolTags
{
	UE_DEFINE_GAMEPLAY_TAG(Axe,FName(TEXT("Als.OverlayMode.Axe")));
	UE_DEFINE_GAMEPLAY_TAG(PickAxe,FName(TEXT("Als.OverlayMode.PickAxe")));
	UE_DEFINE_GAMEPLAY_TAG(Hammer,FName(TEXT("Als.OverlayMode.Hammer")));
}

namespace CustomLocomotionAction
{
	UE_DEFINE_GAMEPLAY_TAG(Mounted,FName(TEXT("Als.LocomotionAction.Mounted")));
}

namespace RobotParts
{
	UE_DEFINE_GAMEPLAY_TAG(Core,FName(TEXT("RobotParts.Core")));
	UE_DEFINE_GAMEPLAY_TAG(Body,FName(TEXT("RobotParts.Body")));
	UE_DEFINE_GAMEPLAY_TAG(Arm,FName(TEXT("RobotParts.Arm")));
	UE_DEFINE_GAMEPLAY_TAG(Booster,FName(TEXT("RobotParts.Booster")));
	UE_DEFINE_GAMEPLAY_TAG(Leg,FName(TEXT("RobotParts.Leg")));
}

namespace ArmTags
{
	UE_DEFINE_GAMEPLAY_TAG(BalanceHeavy, FName(TEXT("RobotParts.Arm.Balance.Heavy")));
	UE_DEFINE_GAMEPLAY_TAG(MeleeLight, FName(TEXT("RobotParts.Arm.Melee.Light")));
	UE_DEFINE_GAMEPLAY_TAG(RangedStandard, FName(TEXT("RobotParts.Arm.Ranged.Standard")));
}

namespace BodyTags
{
	UE_DEFINE_GAMEPLAY_TAG(BalanceLight, FName(TEXT("RobotParts.Body.Balance.Light")));
	UE_DEFINE_GAMEPLAY_TAG(HumidityStandard, FName(TEXT("RobotParts.Body.Humidity.Standard")));
	UE_DEFINE_GAMEPLAY_TAG(TemperatureStandard, FName(TEXT("RobotParts.Body.Temperature.Standard")));
	UE_DEFINE_GAMEPLAY_TAG(BalanceHeavy, FName(TEXT("RobotParts.Body.Balance.Heavy")));
}

namespace BoosterTags
{
	UE_DEFINE_GAMEPLAY_TAG(Cyclic, FName(TEXT("RobotParts.Booster.Cyclic")));
	UE_DEFINE_GAMEPLAY_TAG(HighPower, FName(TEXT("RobotParts.Booster.HighPower")));
	UE_DEFINE_GAMEPLAY_TAG(LongRanged, FName(TEXT("RobotParts.Booster.LongRanged")));
	UE_DEFINE_GAMEPLAY_TAG(Recovery, FName(TEXT("RobotParts.Booster.Recovery")));
	UE_DEFINE_GAMEPLAY_TAG(Standard, FName(TEXT("RobotParts.Booster.Standard")));
	UE_DEFINE_GAMEPLAY_TAG(Sustained, FName(TEXT("RobotParts.Booster.Sustained")));
}

namespace CoreTags
{
	UE_DEFINE_GAMEPLAY_TAG(Tier1, FName(TEXT("RobotParts.Core.Tier1")));
	UE_DEFINE_GAMEPLAY_TAG(Tier2, FName(TEXT("RobotParts.Core.Tier2")));
	UE_DEFINE_GAMEPLAY_TAG(Tier3, FName(TEXT("RobotParts.Core.Tier3")));
	UE_DEFINE_GAMEPLAY_TAG(Tier4, FName(TEXT("RobotParts.Core.Tier4")));
}

namespace LegTags
{
	UE_DEFINE_GAMEPLAY_TAG(Quadrupedal, FName(TEXT("RobotParts.Leg.Quadrupedal")));
	
	UE_DEFINE_GAMEPLAY_TAG(QuadrupedalLight, FName(TEXT("RobotParts.Leg.Quadrupedal.Light")));
	UE_DEFINE_GAMEPLAY_TAG(BipedalStandard, FName(TEXT("RobotParts.Leg.Bipedal.Standard")));
	UE_DEFINE_GAMEPLAY_TAG(ReverseBipedalLight, FName(TEXT("RobotParts.Leg.ReverseBipedal.Light")));
	UE_DEFINE_GAMEPLAY_TAG(QuadrupedalHeavy, FName(TEXT("RobotParts.Leg.Quadrupedal.Heavy")));
	UE_DEFINE_GAMEPLAY_TAG(ReverseBipedalStandard, FName(TEXT("RobotParts.Leg.ReverseBipedal.Standard")));
}

namespace ItemTags
{
	UE_DEFINE_GAMEPLAY_TAG(Tools, FName(TEXT("Item.Tools")));
	UE_DEFINE_GAMEPLAY_TAG(Consumable, FName(TEXT("Item.Consumable")));
	UE_DEFINE_GAMEPLAY_TAG(HelperBot, FName(TEXT("Item.HelperBot")));
	UE_DEFINE_GAMEPLAY_TAG(RobotParts, FName(TEXT("RobotParts")));
	UE_DEFINE_GAMEPLAY_TAG(Weapon, FName(TEXT("Weapon")));
}

namespace RegionBossTypeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Kamish, FName(TEXT("RegionBossType.Kamish")));
	UE_DEFINE_GAMEPLAY_TAG(LavaBrute, FName(TEXT("RegionBossType.LavaBrute")));
}

namespace KamishSkillTags
{
	UE_DEFINE_GAMEPLAY_TAG(ThunderRush, FName(TEXT("Skill.Kamish.Normal.ThunderRush")));
	UE_DEFINE_GAMEPLAY_TAG(KneelDash, FName(TEXT("Skill.Kamish.Normal.KneelDash")));
	UE_DEFINE_GAMEPLAY_TAG(ThunderLeap, FName(TEXT("Skill.Kamish.Normal.ThunderLeap")));
	UE_DEFINE_GAMEPLAY_TAG(DashAndLeap, FName(TEXT("Skill.Kamish.Normal.DashAndLeap")));
	UE_DEFINE_GAMEPLAY_TAG(FlyingHammer, FName(TEXT("Skill.Kamish.Normal.FlyingHammer")));
	UE_DEFINE_GAMEPLAY_TAG(RetrieveHammer, FName(TEXT("Skill.Kamish.Normal.RetrieveHammer")));
	UE_DEFINE_GAMEPLAY_TAG(RhythmStrike, FName(TEXT("Skill.Kamish.Normal.RhythmStrike")));
	UE_DEFINE_GAMEPLAY_TAG(HeavyStrike, FName(TEXT("Skill.Kamish.Normal.HeavyStrike")));
	UE_DEFINE_GAMEPLAY_TAG(HeavyLeftSwing, FName(TEXT("Skill.Kamish.Normal.HeavyLeftSwing")));
	UE_DEFINE_GAMEPLAY_TAG(HeavyRightSwing, FName(TEXT("Skill.Kamish.Normal.HeavyRightSwing")));
	UE_DEFINE_GAMEPLAY_TAG(LightRightSwing, FName(TEXT("Skill.Kamish.Normal.LightRightSwing")));
	UE_DEFINE_GAMEPLAY_TAG(LightLeftSwing, FName(TEXT("Skill.Kamish.Normal.LightLeftSwing")));
	UE_DEFINE_GAMEPLAY_TAG(LightSlam, FName(TEXT("Skill.Kamish.Normal.LightSlam")));
	UE_DEFINE_GAMEPLAY_TAG(LightUpper, FName(TEXT("Skill.Kamish.Normal.LightUpper")));

	UE_DEFINE_GAMEPLAY_TAG(B_ThunderRush, FName(TEXT("Skill.Kamish.Berserk.ThunderRush")));
	UE_DEFINE_GAMEPLAY_TAG(B_KneelDash, FName(TEXT("Skill.Kamish.Berserk.KneelDash")));
	UE_DEFINE_GAMEPLAY_TAG(B_ThunderLeap, FName(TEXT("Skill.Kamish.Berserk.ThunderLeap")));
	UE_DEFINE_GAMEPLAY_TAG(B_DashAndLeap, FName(TEXT("Skill.Kamish.Berserk.DashAndLeap")));
	UE_DEFINE_GAMEPLAY_TAG(B_FlyingHammer, FName(TEXT("Skill.Kamish.Berserk.FlyingHammer")));
	UE_DEFINE_GAMEPLAY_TAG(B_RetrieveHammer, FName(TEXT("Skill.Kamish.Berserk.RetrieveHammer")));
	UE_DEFINE_GAMEPLAY_TAG(B_RhythmStrike, FName(TEXT("Skill.Kamish.Berserk.RhythmStrike")));
	UE_DEFINE_GAMEPLAY_TAG(B_HeavyStrike, FName(TEXT("Skill.Kamish.Berserk.HeavyStrike")));
	UE_DEFINE_GAMEPLAY_TAG(B_HeavyLeftSwing, FName(TEXT("Skill.Kamish.Berserk.HeavyLeftSwing")));
	UE_DEFINE_GAMEPLAY_TAG(B_HeavyRightSwing, FName(TEXT("Skill.Kamish.Berserk.HeavyRightSwing")));
}

namespace LavaBruteSkillTags
{
	UE_DEFINE_GAMEPLAY_TAG(ThrowingStones, FName(TEXT("Skill.LavaBrute.Normal.ThrowingStones")));
	UE_DEFINE_GAMEPLAY_TAG(EarthRumbling, FName(TEXT("Skill.LavaBrute.Normal.EarthRumbling")));
	UE_DEFINE_GAMEPLAY_TAG(ShoulderTackle, FName(TEXT("Skill.LavaBrute.Normal.ShoulderTackle")));
	UE_DEFINE_GAMEPLAY_TAG(LavaBreath, FName(TEXT("Skill.LavaBrute.Normal.LavaBreath")));
	UE_DEFINE_GAMEPLAY_TAG(CreateLava, FName(TEXT("Skill.LavaBrute.Normal.CreateLava")));
	UE_DEFINE_GAMEPLAY_TAG(GroundSmash, FName(TEXT("Skill.LavaBrute.Normal.GroundSmash")));
	UE_DEFINE_GAMEPLAY_TAG(SmashTwice, FName(TEXT("Skill.LavaBrute.Normal.SmashTwice")));
	UE_DEFINE_GAMEPLAY_TAG(HeavyPushLA, FName(TEXT("Skill.LavaBrute.Normal.HeavyPushLA")));
	UE_DEFINE_GAMEPLAY_TAG(HeavySlapHand, FName(TEXT("Skill.LavaBrute.Normal.HeavySlapHand")));
	UE_DEFINE_GAMEPLAY_TAG(HeavySmash, FName(TEXT("Skill.LavaBrute.Normal.HeavySmash")));
	UE_DEFINE_GAMEPLAY_TAG(LightSpike, FName(TEXT("Skill.LavaBrute.Normal.LightSpike")));
	UE_DEFINE_GAMEPLAY_TAG(LightSwingLA, FName(TEXT("Skill.LavaBrute.Normal.LightSwingLA")));
	UE_DEFINE_GAMEPLAY_TAG(LightSwingRA, FName(TEXT("Skill.LavaBrute.Normal.LightSwingRA")));
}