#include "MonsterStateComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "CR4S.h"
#include "MonsterAnimComponent.h"
#include "Character/Components/BaseStatusComponent.h"
#include "Game/System/AudioManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MonsterAI/BaseMonster.h"
#include "MonsterAI/Data/MonsterAttributeRow.h"
#include "Perception/AIPerceptionComponent.h"

UMonsterStateComponent::UMonsterStateComponent()
	: MyHeader(TEXT("MonsterStateComp"))
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	CurrentState = EMonsterState::Idle;
	PreviousState = EMonsterState::Idle;
	StateElapsedTime = 0.f;
}

void UMonsterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register initial state
	StateDuration.Add(CurrentState, 0.f);

	// NOTICE :: Test Log
	UE_LOG(LogTemp, Log, TEXT("[%s] BeginPlay : CurrentState is %s."),
		*MyHeader,
		*UEnum::GetValueAsString(CurrentState)
	);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			if (UBaseStatusComponent* StatusComp = PlayerPawn->FindComponentByClass<UBaseStatusComponent>())
			{
				StatusComp->OnDeathState.AddUObject(this, &UMonsterStateComponent::HandlePlayerDeath);
			}
		}
	}
}

void UMonsterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Accumulate time spent in current state
	StateElapsedTime += DeltaTime;
	StateDuration.FindOrAdd(CurrentState) += DeltaTime;

	if (!bIsStunned && bCanRecover && CurrentStun > 0.f)
	{
		RecoveryElapsedTime = FMath::Clamp(RecoveryElapsedTime + DeltaTime, 0.f, StunRecoveryRampUpTime);
		
		float RecoveryAlpha = (StunRecoveryRampUpTime > 0.f) ? (RecoveryElapsedTime / StunRecoveryRampUpTime) : 1.f;
		float CurrentRecoveryRate = FMath::Lerp(StunRecoveryMin, StunRecoveryMax, RecoveryAlpha);
		
		CurrentStun = FMath::Clamp(CurrentStun - CurrentRecoveryRate * DeltaTime, 0.f, MaxStun);
	}
}

void UMonsterStateComponent::SetState(EMonsterState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	PreviousState = CurrentState;
	CurrentState = NewState;
	StateElapsedTime = 0.f;

	// Notice :: Test Log
	if (const UEnum* EnumPtr = StaticEnum<EMonsterState>())
	{
		const FString PrevName = EnumPtr->GetNameStringByValue(static_cast<int64>(PreviousState));
		const FString CurrName = EnumPtr->GetNameStringByValue(static_cast<int64>(CurrentState));
		CR4S_Log(LogMonster, Warning, TEXT("[%s] State changed: %s → %s"), *GetClass()->GetName(), *PrevName, *CurrName);
	}
	
	// Broadcast delegate when state changes
	OnStateChanged.Broadcast(PreviousState, CurrentState);
}

float UMonsterStateComponent::GetStateDuration(EMonsterState State) const
{
	if (const float* Found = StateDuration.Find(State))
	{
		return *Found;
	}

	return 0.f;
}

void UMonsterStateComponent::ForceInterrupt()
{
	// Force reset to Idle state
	SetState(EMonsterState::Idle);
}

void UMonsterStateComponent::CheckPhaseTransition(float CurrentHP, float MaxHP)
{
	if (!PhaseDataAsset) return;

	const float Ratio = CurrentHP / MaxHP;

	for (const FPhaseTransitionCondition& Condition : PhaseDataAsset->PhaseConditions)
	{
		if (Condition.TargetPhase <= CurrentPhase) continue;
		if (Ratio <= Condition.HPThresholdRatio)
		{
			CurrentSpeedMultiplier = Condition.SpeedMultiplier;
			CurrentDamageMultiplier = Condition.DamageMultiplier;

			SetPhase(Condition.TargetPhase);
			break;
		}
	}
}

void UMonsterStateComponent::SetPhase(EBossPhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	CurrentPhase = NewPhase;
	OnPhaseChanged.Broadcast(NewPhase);
}

void UMonsterStateComponent::AddStun(float StunAmount)
{
	if (bIsStunned) return;
	
	CurrentStun = FMath::Clamp(CurrentStun + StunAmount, 0.f, MaxStun);
	CR4S_Log(LogMonster, Log, TEXT("[Stun] Add Stun - CurrentStun=%.2f / MaxStun=%.2f"), CurrentStun, MaxStun);

	bCanRecover = false;
	GetWorld()->GetTimerManager().ClearTimer(RecoveryDelayTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		RecoveryDelayTimerHandle,
		[this]() { bCanRecover = true; },
		StunChargeStartDelay,
		false
	);
	
	if (CurrentStun >= MaxStun)
	{
		bIsStunned = true;
		CurrentStun = MaxStun;
		
		SetState(EMonsterState::Stunned);
		OnStunStarted.Broadcast();
		
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
			if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
				Movement->DisableMovement();
            	
		if (ABaseMonster* Monster = Cast<ABaseMonster>(GetOwner()))
		{
			if (Monster->GetMonsterAnimComp())
			{
				Monster->GetMonsterAnimComp()->StopAllMontages();
				Monster->GetMonsterAnimComp()->PlayStunnedMontage();
			}
		}

		GetWorld()->GetTimerManager().ClearTimer(RecoveryDelayTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(StunRecoveryTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			StunRecoveryTimerHandle,
			this,
			&UMonsterStateComponent::RemoveStunDebuff,
			StunningTime,
			false
		);
	}
}

void UMonsterStateComponent::RemoveStunDebuff()
{
	if (!bIsStunned) return;

	bIsStunned = false;
	CurrentStun = 0.f;
	RecoveryElapsedTime = 0.f;
	
	SetState(PreviousState);
	
	OnStunEnded.Broadcast();
	
	GetWorld()->GetTimerManager().ClearTimer(StunRecoveryTimerHandle);
	
	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
		if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
			Movement->SetMovementMode(EMovementMode::MOVE_Walking);

	CR4S_Log(LogMonster, Log, TEXT("[Stun] Clear Stun - CurrentStun=%.2f / MaxStun=%.2f"), CurrentStun, MaxStun);
}

void UMonsterStateComponent::InitializeStunData(const FMonsterAttributeRow& Data)
{
	MaxStun                = Data.MaxStun;
	StunningTime           = Data.StunningTime;
	StunDamageMultiplier   = Data.StunDamageMultiplier;
	StunChargeStartDelay   = Data.StunChargeStartDelay;
	StunRecoveryMin        = Data.StunRecoveryMin;
	StunRecoveryMax        = Data.StunRecoveryMax;
	StunRecoveryRampUpTime = Data.StunRecoveryRampUpTime;

	CR4S_Log(LogMonster, Log, TEXT("[%s] InitializeStunData: MaxStun=%.2f, StunDamageMultiplier=%.2f, StunChargeStartDelay=%.2f, StunRecoveryMin=%.2f, StunRecoveryMax=%.2f, StunRecoveryRampUpTime=%.2f"),
	   *MyHeader,
	   MaxStun,
	   StunDamageMultiplier,
	   StunChargeStartDelay,
	   StunRecoveryMin,
	   StunRecoveryMax,
	   StunRecoveryRampUpTime
   );
}


void UMonsterStateComponent::HandlePlayerDeath()
{
	SetState(EMonsterState::Idle);
	
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
		{
			MoveComp->DisableMovement();
		}
	}

	if (AController* C = Cast<AController>(GetOwner()->GetInstigatorController()))
	{
		if (AAIController* AICon = Cast<AAIController>(C))
		{
			if (UBrainComponent* Brain = AICon->GetBrainComponent())
			{
				Brain->StopLogic("PlayerDead");
			}
			if (UAIPerceptionComponent* Percep = AICon->FindComponentByClass<UAIPerceptionComponent>())
			{
				Percep->Deactivate();
			}
		}
	}
	
	if (ABaseMonster* Monster = Cast<ABaseMonster>(GetOwner()))
	{
		if (Monster->GetMonsterAnimComp())
		{
			Monster->GetMonsterAnimComp()->StopAllMontages();
		}
	}

	if (UAudioManager* AudioMgr = GetWorld()->GetGameInstance()->GetSubsystem<UAudioManager>())
	{
		AudioMgr->StopBGM();
	}
	
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}