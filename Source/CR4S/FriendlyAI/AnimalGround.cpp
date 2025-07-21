#include "AnimalGround.h"
#include "Component/AIJumpComponent.h"
#include "../Gimmick/Components/InteractableComponent.h"
#include "Component/AnimalIKComponent.h"
#include "Components/SphereComponent.h"
#include "Controller/AnimalAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utility/CombatStatics.h"

AAnimalGround::AAnimalGround()
{
	AIJumpComponent = CreateDefaultSubobject<UAIJumpComponent>(TEXT("AIJumpComponent"));
	AnimalIKComponent = CreateDefaultSubobject<UAnimalIKComponent>(TEXT("AnimalIKComponent"));
}

void AAnimalGround::BeginPlay()
{
	Super::BeginPlay();
	if (UAIJumpComponent* JumpComp = FindComponentByClass<UAIJumpComponent>())
	{
		JumpComp->UpdateOwnerStats();
	}

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (Move)
	{
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 120.f, 0.f);
	}
}

#pragma region Stun
// void AAnimalGround::ApplyStun(float Amount)
// {
// 	Super::ApplyStun(Amount);
//
// 	if (!bIsStunned) return;
//
// 	bUseControllerRotationYaw = false;
// 	GetCharacterMovement()->bOrientRotationToMovement = false;
//
// 	if (GetController())
// 	{
// 		GetController()->StopMovement();
// 	}
//
// 	GetCharacterMovement()->StopMovementImmediately();
// 	GetCharacterMovement()->DisableMovement();
//
// 	FName BoneName = GetMesh()->GetSocketBoneName(TEXT("TS"));
// 	GetMesh()->SetSimulatePhysics(false);
// 	GetMesh()->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
// 	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
// }
//
// void AAnimalGround::RecoverFromStun()
// {
// 	Super::RecoverFromStun();
//
// 	bUseControllerRotationYaw = true;
// 	GetCharacterMovement()->bOrientRotationToMovement = true;
//
// 	GetMesh()->bBlendPhysics = true;
// 	GetMesh()->SetAllBodiesSimulatePhysics(false);
// 	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
//
// 	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
// }
#pragma endregion

#pragma region Attack
float AAnimalGround::PlayAttackMontage()
{
	if (AIJumpComponent)
	{
		AIJumpComponent->DeactivateJumpComponent();
	}
	return Super::PlayAttackMontage();
}

float AAnimalGround::PlayChargeAttackMontage()
{
	if (AIJumpComponent)
	{
		AIJumpComponent->DeactivateJumpComponent();
	}
	return Super::PlayChargeAttackMontage();
}

float AAnimalGround::PlayRangedAttackMontage()
{
	if (AIJumpComponent)
	{
		AIJumpComponent->DeactivateJumpComponent();
	}
	return Super::PlayRangedAttackMontage();
}

void AAnimalGround::ResetAttackFlag()
{
	Super::ResetAttackFlag();
	if (AIJumpComponent)
	{
		AIJumpComponent->ActivateJumpComponent();
	}
}

void AAnimalGround::PerformMeleeAttack()
{
	Super::PerformMeleeAttack();

	if (!IsValid(CurrentTarget)) 
	{
		return;
	}
	
	if (ABaseAnimal* HitAnimal = Cast<ABaseAnimal>(CurrentTarget))
	{
		if (HitAnimal->CurrentState == EAnimalState::Dead)
		{
			if (AAnimalAIController* C = Cast<AAnimalAIController>(GetController()))
			{
				C->OnTargetDied();
			}
			return;
		}
	}

	if (!AttackRange || !AttackRange->IsOverlappingActor(CurrentTarget))
	{
		if (AAnimalAIController* C = Cast<AAnimalAIController>(GetController()))
		{
			C->OnTargetOutOfRange();
		}
		return;
	}

	float Damage = GetCurrentStats().AttackDamage;
	UGameplayStatics::ApplyDamage(CurrentTarget, Damage, GetController(), this, nullptr);

	UCombatStatics::ApplyStun(CurrentTarget, GetCurrentStats().StunAmount);
}

void AAnimalGround::PerformChargeAttack()
{
	// if (StatsRow)
	// {
	// 	LaunchCharacter(GetActorForwardVector() * StatsRow->ChargeSpeed, true, true);
	// 	//GetCharacterMovement()->Velocity = GetActorForwardVector() * StatsRow->ChargeSpeed;
	// } 
}
#pragma endregion
