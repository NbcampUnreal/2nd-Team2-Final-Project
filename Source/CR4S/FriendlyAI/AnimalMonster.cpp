#include "AnimalMonster.h"
#include "FriendlyAI/Controller/AnimalMonsterAIController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Character/Characters/PlayerCharacter.h"
#include "Character/Characters/ModularRobot.h"
#include "Gimmick/GimmickObjects/Buildings/BaseBuildingGimmick.h"
#include "Components/SphereComponent.h"
#include "FriendlyAI/Component/AnimalRangedAttackComponent.h"
#include "Engine/World.h"
#include "FriendlyAI/BaseHelperBot.h"
#include "FriendlyAI/Component/AIJumpComponent.h"
#include "DrawDebugHelpers.h"
#include "Component/AnimalIKComponent.h"
#include "Utility/CombatStatics.h"

#pragma region AActor Override
AAnimalMonster::AAnimalMonster()
{
    PrimaryActorTick.bCanEverTick = true;
    
    AIControllerClass = AAnimalMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AIJumpComponent = CreateDefaultSubobject<UAIJumpComponent>(TEXT("AIJumpComponent"));
    AnimalIKComponent = CreateDefaultSubobject<UAnimalIKComponent>(TEXT("AnimalIKComponent"));
}

void AAnimalMonster::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMonster();

    //Temporary
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->bOrientRotationToMovement = true;
        MovementComp->RotationRate = FRotator(0.f, 180.f, 0.f);
        MovementComp->bUseRVOAvoidance = true;
        MovementComp->MaxAcceleration = 1024.f;
        MovementComp->BrakingDecelerationWalking = 1024.f;
    }
}

void AAnimalMonster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bDrawTargetDebug || bDrawPathDebug)
    {
        DrawMonsterDebugVisuals();
    }
}
#pragma endregion

#pragma region Initialization
void AAnimalMonster::InitializeMonster()
{
    SetInitialTarget();
    
    bUseControllerRotationYaw = false;
    
    if (AttackRange)
    {
        AttackRange->SetCollisionProfileName(TEXT("Trigger"));
    }
}

void AAnimalMonster::SetInitialTarget()
{
    AActor* PlayerTarget = GetCurrentPlayerTarget();
    if (PlayerTarget)
    {
        SetPrimaryTarget(PlayerTarget);
        CurrentTarget = PlayerTarget;
        
        if (AAnimalMonsterAIController* MonsterAI = Cast<AAnimalMonsterAIController>(GetController()))
        {
            MonsterAI->SetTargetActor(PlayerTarget);
        }
    }
}
#pragma endregion

#pragma region Target Management
void AAnimalMonster::SetPrimaryTarget(AActor* Target)
{
    PrimaryTarget = Target;
}

void AAnimalMonster::SetSecondaryTarget(AActor* Target)
{
    SecondaryTarget = Target;
}

void AAnimalMonster::SetBuildingTarget(AActor* Target)
{
    BuildingTarget = Target;
    bIsDestroyingBuilding = IsValid(Target);
}

void AAnimalMonster::ClearAllTargets()
{
    PrimaryTarget = nullptr;
    SecondaryTarget = nullptr;
    BuildingTarget = nullptr;
    CurrentTarget = nullptr;
    bIsDestroyingBuilding = false;
}
#pragma endregion

#pragma region Attack
void AAnimalMonster::PerformMeleeAttack()
{
    Super::PerformMeleeAttack();
    
    if (!IsValid(CurrentTarget)) return;
    
    if (!AttackRange || !AttackRange->IsOverlappingActor(CurrentTarget))
    {
        if (AAnimalMonsterAIController* MonsterAI = Cast<AAnimalMonsterAIController>(GetController()))
        {
            MonsterAI->OnTargetOutOfRange();
        }
        return;
    }
    
    ApplyDamageToTarget(CurrentTarget);

    UCombatStatics::ApplyStun(CurrentTarget, GetCurrentStats().StunAmount);
}

void AAnimalMonster::PerformChargeAttack()
{
    if (!bCanCharge || bIsChargeOnCooldown || !IsValid(CurrentTarget)) return;
    
    ApplyDamageToTarget(CurrentTarget);
}

#pragma endregion

#pragma region Damage
float AAnimalMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
                                AController* EventInstigator, AActor* DamageCauser)
{
    if (AAnimalMonster* OtherMonster = Cast<AAnimalMonster>(DamageCauser))
    {
        return 0.f;
    }

    float ActualDamage = DamageAmount;
    if (ActualDamage <= 0.f)
    {
        return 0.f;
    }

    ShowHitEffect(DamageCauser);
    
    CurrentHealth -= ActualDamage;
    if (CurrentHealth <= 0.f)
    {
        CurrentHealth = 0.f;
        SetAnimalState(EAnimalState::Dead);
        Die();
    }
    return ActualDamage;
}
#pragma endregion

#pragma region Monster Functions
// bool AAnimalMonster::CanReachTarget(AActor* Target) const
// {
//     if (!IsValid(Target)) return false;
//
//     UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
//     if (!NavSystem) return false;
//
//     FPathFindingQuery Query;
//     Query.StartLocation = GetActorLocation();
//     Query.EndLocation = Target->GetActorLocation();
//     Query.NavData = NavSystem->GetDefaultNavDataInstance();
//
//     FPathFindingResult Result = NavSystem->FindPathSync(Query);
//     return Result.IsSuccessful() && Result.Path.IsValid();
// }

AActor* AAnimalMonster::FindNearestDestructibleBuilding() const
{
    TArray<AActor*> FoundBuildings;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseBuildingGimmick::StaticClass(), FoundBuildings);

    AActor* NearestBuilding = nullptr;
    float NearestDistance = BuildingDetectionRange;

    FVector MyLocation = GetActorLocation();
    
    for (AActor* Building : FoundBuildings)
    {
        if (!IsValid(Building) || IsBuildingDestroyed(Building)) continue;

        float Distance = FVector::Dist(MyLocation, Building->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestBuilding = Building;
        }
    }

    return NearestBuilding;
}

bool AAnimalMonster::IsDestructibleBuilding(AActor* Building) const
{
    return IsValid(Building) && Building->IsA<ABaseBuildingGimmick>();
}

AActor* AAnimalMonster::GetCurrentPlayer() const
{
    return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

AActor* AAnimalMonster::GetCurrentPlayerTarget() const
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(GetCurrentPlayer());
    if (!IsValid(Player)) return nullptr;
    
    TArray<AActor*> FoundRobots;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AModularRobot::StaticClass(), FoundRobots);

    for (AActor* RobotActor : FoundRobots)
    {
        if (AModularRobot* Robot = Cast<AModularRobot>(RobotActor))
        {
            APlayerCharacter* MountedChar = Robot->GetMountedCharacter();            
            if (MountedChar == Player)
            {
                return Robot;
            }
        }
    }
    return Player;
}

TArray<AActor*> AAnimalMonster::FindNearbyAutomatedAI() const
{
    TArray<AActor*> AutomatedAI;
    TArray<AActor*> FoundHelperBots;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseHelperBot::StaticClass(), FoundHelperBots);

    FVector MyLocation = GetActorLocation();

    for (AActor* HelperBotActor : FoundHelperBots)
    {
        float Distance = FVector::Dist(MyLocation, HelperBotActor->GetActorLocation());
        if (Distance <= AutomatedAIDetectionRange)
        {
            AutomatedAI.Add(HelperBotActor);
        }
    }

    return AutomatedAI;
}

bool AAnimalMonster::IsBuildingDestroyed(AActor* Building) const
{
    if (ABaseBuildingGimmick* BuildingGimmick = Cast<ABaseBuildingGimmick>(Building))
    {
        return BuildingGimmick->GetDurabilityRatio() <= 0.0f;
    }
    return false;
}
#pragma endregion

#pragma region Debug
void AAnimalMonster::DrawMonsterDebugVisuals()
{
    if (!GetWorld()) return;

    FVector MyLocation = GetActorLocation();

    if (bDrawTargetDebug)
    {
        // Primary Target
        if (IsValid(PrimaryTarget))
        {
            DrawDebugLine(GetWorld(), MyLocation, PrimaryTarget->GetActorLocation(), 
                         FColor::Red, false, -1.f, 0, 3.f);
            DrawDebugString(GetWorld(), PrimaryTarget->GetActorLocation() + FVector(0, 0, 100), 
                           TEXT("PRIMARY"), nullptr, FColor::Red, -1.f);
        }

        // Secondary Target
        if (IsValid(SecondaryTarget))
        {
            DrawDebugLine(GetWorld(), MyLocation, SecondaryTarget->GetActorLocation(), 
                         FColor::Blue, false, -1.f, 0, 2.f);
            DrawDebugString(GetWorld(), SecondaryTarget->GetActorLocation() + FVector(0, 0, 100), 
                           TEXT("SECONDARY"), nullptr, FColor::Blue, -1.f);
        }

        // Building Target
        if (IsValid(BuildingTarget))
        {
            DrawDebugLine(GetWorld(), MyLocation, BuildingTarget->GetActorLocation(), 
                         FColor::Orange, false, -1.f, 0, 2.f);
            DrawDebugString(GetWorld(), BuildingTarget->GetActorLocation() + FVector(0, 0, 100), 
                           TEXT("BUILDING"), nullptr, FColor::Orange, -1.f);
        }
    }

    if (bDrawPathDebug)
    {
        // Detection Range
        DrawDebugSphere(GetWorld(), MyLocation, AutomatedAIDetectionRange, 16, 
                       FColor::Cyan, false, -1.f, 0, 1.f);
        DrawDebugSphere(GetWorld(), MyLocation, BuildingDetectionRange, 16, 
                       FColor::Yellow, false, -1.f, 0, 1.f);
    }
}
#pragma endregion

#pragma region Internal Functions
float AAnimalMonster::GetDamageForTarget(AActor* Target) const
{
    if (Cast<APlayerCharacter>(Target) || Cast<AModularRobot>(Target))
    {
        return 1000.0f;
    }
    else if (Cast<ABaseBuildingGimmick>(Target))
    {
        return 1000000.0f;
    }
    
    return PlayerDamage;
}

void AAnimalMonster::ApplyDamageToTarget(AActor* Target)
{
    if (!IsValid(Target)) return;

    float Damage = GetDamageForTarget(Target);
    UGameplayStatics::ApplyDamage(Target, Damage, GetController(), this, nullptr);
}
#pragma endregion