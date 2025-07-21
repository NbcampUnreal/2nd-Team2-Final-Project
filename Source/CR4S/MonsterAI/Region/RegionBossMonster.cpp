#include "RegionBossMonster.h"
#include "MonsterAI/Controller/RegionBossMonsterAIController.h"
#include "MonsterAI/Region/PatrolRoute.h"
#include "MonsterAI/Region/CombatRangeVisualizer.h"
#include "MonsterAI/Data/MonsterAIKeyNames.h"
#include "MonsterAI/Components/MonsterStateComponent.h"
#include "MonsterAI/Components/MonsterAttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationInvokerComponent.h"
#include "CR4S.h"

ARegionBossMonster::ARegionBossMonster()
	: MyHeader(TEXT("RegionBossMonster"))
{
	PrimaryActorTick.bCanEverTick = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bEnablePhysicsInteraction = false;
		MoveComp->bPushForceScaledToMass = false;
		MoveComp->PushForceFactor = 0.0f;
	}
	
	NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
	NavInvoker->SetGenerationRadii(NavGenerationRadius, NavRemovalRadius);
	NavInvoker->SetCanEverAffectNavigation(true);
}

void ARegionBossMonster::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMesh->SetCollisionObjectType(ECC_Pawn);
		SkeletalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	
		SkeletalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		SkeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SkeletalMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
		SkeletalMesh->SetGenerateOverlapEvents(true);
	}
	
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CapsuleComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		CapsuleComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
		CapsuleComp->SetGenerateOverlapEvents(true);
	}
	
	if (UMonsterStateComponent* StateComp = FindComponentByClass<UMonsterStateComponent>())
	{
		StateComp->SetState(EMonsterState::Patrol);
		StateComp->SetPhase(EBossPhase::Normal);
		StateComp->OnPhaseChanged.AddUniqueDynamic(this, &ARegionBossMonster::HandlePhaseChanged);
	}

	SetCombatStartLocation();
}

void ARegionBossMonster::SetCombatStartLocation()
{
	CombatStartLocation = GetActorLocation();

	ARegionBossMonsterAIController* AIC = Cast<ARegionBossMonsterAIController>(GetController());
	if (!IsValid(AIC))	return;

	if (auto* BB = AIC->GetBlackboardComponent())
	{
		BB->SetValueAsVector(FRegionBossAIKeys::CombatStartLocation, CombatStartLocation);
	}
}

bool ARegionBossMonster::IsOutsideCombatRange(float Tolerance) const
{
	const ARegionBossMonsterAIController* AIC = Cast<ARegionBossMonsterAIController>(GetController());
	if (!IsValid(AIC)) return false;

	const UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!IsValid(BB)) return false;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(FAIKeys::TargetActor));
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RegionBoss] ReturnCheck: Target is invalid"));
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation();
	const float DistanceSqr = FVector::DistSquared(CombatStartLocation, TargetLocation);
	const float Limit = FMath::Square(CombatRange + Tolerance);

	return DistanceSqr > Limit;
}

void ARegionBossMonster::ShowCombatRange()
{
	if (!RangeVisualizer && RangeVisualizerClass)
	{
		RangeVisualizer = GetWorld()->SpawnActor<ACombatRangeVisualizer>(RangeVisualizerClass);
		if (!RangeVisualizer) return;

		FHitResult Hit;
		FVector TraceStart = GetCombatStartLocation() + FVector(0, 0, 500);
		FVector TraceEnd = GetCombatStartLocation() - FVector(0, 0, 1000);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		FVector GroundLocation = GetCombatStartLocation();
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			GroundLocation = Hit.ImpactPoint;
		}

		RangeVisualizer->InitializeVisualizer(GroundLocation - FVector(0, 0, 10), CombatRange, CombatRangeVisualizerHeight);
	}
}

void ARegionBossMonster::HideCombatRange()
{
	if (RangeVisualizer)
	{
		RangeVisualizer->Destroy();
		RangeVisualizer = nullptr;
	}
}

FVector ARegionBossMonster::GetNextPatrolLocation()
{
	if (!PatrolRouteActor) return GetActorLocation();

	int32 PointCount = PatrolRouteActor->GetNumberOfPoints();
	if (PointCount == 0) return GetActorLocation();

	FVector NextPoint = PatrolRouteActor->GetPointAtIndex(CurrentPatrolIndex);
	CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PointCount;

	return NextPoint;
}

void ARegionBossMonster::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bJumpFromNotify)
	{
		FHitResult GroundHit;
		const FVector Start = GetActorLocation();
		const FVector End = Start - FVector(0.f, 0.f, 2000.f);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_Visibility, Params))
		{
			SetActorLocation(GroundHit.ImpactPoint);
			UE_LOG(LogTemp, Log, TEXT("[JumpFix] Forced ground snap."));
		}
	}

	GetCharacterMovement()->StopMovementImmediately();
	bJumpFromNotify = false;
}

void ARegionBossMonster::OnMonsterStateChanged(EMonsterState Previous, EMonsterState Current)	 
{
	Super::OnMonsterStateChanged(Previous, Current);

	const FMonsterAttributeRow& Attribute = AttributeComponent->GetMonsterAttribute();
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	float DesiredSpeed = Attribute.MoveSpeed;
	switch (Current)
	{
	case EMonsterState::Patrol: 
		DesiredSpeed *= 0.25f;
		break;
	case EMonsterState::Alert:
		DesiredSpeed *= 0.4f;  
		break;
	case EMonsterState::Combat:
		if (Previous != EMonsterState::Stunned)
		{
			SetCombatStartLocation();
			ShowCombatRange();
			AttributeComponent->InitializeMonsterAttribute(MonsterID);
		}
		break;
	case EMonsterState::Return:
		if (Previous == EMonsterState::Combat)
		{
			HideCombatRange();
		}
		break;
	}

	if (!FMath::IsNearlyEqual(MoveComp->MaxWalkSpeed, DesiredSpeed, 1.f))
	{
		MoveComp->MaxWalkSpeed = DesiredSpeed;
	}
}

void ARegionBossMonster::HandlePhaseChanged(EBossPhase NewPhase)
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsEnum(FRegionBossAIKeys::CurrentPhase, static_cast<int32>(NewPhase));
			BB->SetValueAsInt(FRegionBossAIKeys::CurrentPatternID, 0);
			BB->SetValueAsInt(FRegionBossAIKeys::PatternStepIndex, 0);
		}
	}

	if (!AttributeComponent || !StateComponent) return;

	if (NewPhase == EBossPhase::Normal)
	{
		AttributeComponent->InitializeMonsterAttribute(MonsterID);
	}
	else
	{
		const float BaseSpeed = AttributeComponent->GetMonsterAttribute().MoveSpeed;
		const float SpeedMultiplier = StateComponent->GetCurrentSpeedMultiplier();

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = BaseSpeed * SpeedMultiplier;
		}
	}

	UE_LOG(LogMonster, Log, TEXT("[%s] HandlePhaseChanged : Current phase changed to %s"), *MyHeader,
		*UEnum::GetDisplayValueAsText(NewPhase).ToString());
}