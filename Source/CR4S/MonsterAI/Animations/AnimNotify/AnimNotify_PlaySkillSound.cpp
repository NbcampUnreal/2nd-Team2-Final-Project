#include "MonsterAI/Animations/AnimNotify/AnimNotify_PlaySkillSound.h"
#include "CR4S.h"

void UAnimNotify_PlaySkillSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!IsValid(MeshComp))
	{
		CR4S_Log(LogMonster, Warning, TEXT("[%s] MeshComp is null"), *GetClass()->GetName());
		return;
	}

	USoundBase* SoundAsset = Sound.LoadSynchronous();
	if (!IsValid(SoundAsset))
	{
		CR4S_Log(LogMonster, Warning, TEXT("[%s] Sound asset is not set"), *GetClass()->GetName());
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!IsValid(World))
	{
		CR4S_Log(LogMonster, Warning, TEXT("[%s] World is null"), *GetClass()->GetName());
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		CR4S_Log(LogMonster, Warning, TEXT("[%s] GameInstance is null"), *GetClass()->GetName());
		return;
	}

	UAudioManager* AudioMgr = GameInstance->GetSubsystem<UAudioManager>();
	if (!AudioMgr)
	{
		CR4S_Log(LogMonster, Warning, TEXT("[%s] AudioManager subsystem not found"), *GetClass()->GetName());
		return;
	}

	AudioMgr->PlaySFX(SoundAsset, MeshComp->GetComponentLocation(), ConcurrencyType, PitchMultiplier, StartTime);
}
