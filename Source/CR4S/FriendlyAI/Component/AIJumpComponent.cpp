#include "AIJumpComponent.h"

#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"

#pragma region Core System Management

UAIJumpComponent::UAIJumpComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UAIJumpComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        OwnerJumpPower = GetJumpPowerFromOwner();
        OwnerMaxSpeed = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
        OwnerJumpDistance = OwnerJumpPower * 2.0f;
        
        AIController = Cast<AAIController>(OwnerCharacter->GetController());
        if (AIController)
        {
            BlackboardComponent = AIController->GetBlackboardComponent();
            
            CrowdFollowingComponent = AIController->FindComponentByClass<UCrowdFollowingComponent>();
            if (!CrowdFollowingComponent)
            {
                CrowdFollowingComponent = NewObject<UCrowdFollowingComponent>(AIController);
                AIController->AddInstanceComponent(CrowdFollowingComponent);
            }
        }
    }
}

void UAIJumpComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DeactivateJumpComponent();
    Super::EndPlay(EndPlayReason);
}

void UAIJumpComponent::UpdateOwnerStats()
{
    if (!OwnerCharacter) return;
    OwnerJumpPower    = GetJumpPowerFromOwner();
    if (UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement())
    {
        OwnerMaxSpeed      = MoveComp->MaxWalkSpeed;
    }
    OwnerJumpDistance = OwnerJumpPower * 2.0f;
}

void UAIJumpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsComponentActive || !OwnerCharacter || !AIController || !BlackboardComponent)
        return;
    
    if (OwnerJumpPower > 0.0f && bShowDetailedDebug)
    {
        FVector WallLocation;
        float WallHeight;
        DetectNearbyWall(OwnerCharacter->GetActorLocation(), WallLocation, WallHeight);
    }

    if (!GetTargetActorFromBlackboard())
        return;
    
    if (HasReachedTarget())
    {
        //ResetJumpState();
        //return;
    }
    
    if (bEnableRotation && CurrentTargetActor)
    {
        FVector TargetDirection = (CurrentTargetActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();
        FRotator TargetRotation = TargetDirection.Rotation();
        FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 270.0f);
        OwnerCharacter->SetActorRotation(NewRotation);
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPathCalculationTime >= PathRecalculationInterval)
    {
        UpdatePathToTarget();
        LastPathCalculationTime = CurrentTime;
    }
    
    if (!bIsJumping)
    {
        ExecuteMovement();
    }
    else
    {
        if (IsGrounded())
        {
            HandleLanding();
        }
    }
}

#pragma region Movement Execution System

void UAIJumpComponent::ExecuteMovement()
{
    if (!OwnerCharacter || CurrentPath.Num() == 0)
    {
        return;
    }
        
    if (CurrentPathIndex >= CurrentPath.Num())
    {
        return;
    }
    
    FJumpPoint CurrentTarget = CurrentPath[CurrentPathIndex];
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
        
    float CheckDistance = TargetReachDistance;
    if (CurrentTargetActor && CurrentTargetActor->IsA<APawn>())
    {
        CheckDistance = 10.0f;
    }
    else
    {
        CheckDistance = 450.0f;
    }

    if (FVector::Dist(CharacterLocation, CurrentTarget.Location) <= CheckDistance)
    {
        CurrentPathIndex++;
        return;
    }

    if (CurrentTarget.bRequiresJump)
    {
        if (FVector::Dist(CharacterLocation, CurrentTarget.JumpStartPos) <= JumpStartDistance)
        {
            ExecuteJump();
        }
        else
        {
            FVector Direction = (CurrentTarget.JumpStartPos - CharacterLocation).GetSafeNormal();
            OwnerCharacter->AddMovementInput(Direction, 1.0f);
        }
    }
    else
    {
        FVector Direction = (CurrentTarget.Location - CharacterLocation).GetSafeNormal();
        OwnerCharacter->AddMovementInput(Direction, 1.0f);
    }
}

void UAIJumpComponent::ExecuteJump()
{
    if (bIsJumping || !OwnerCharacter || CurrentPathIndex >= CurrentPath.Num())
        return;
    
    FJumpPoint CurrentTarget = CurrentPath[CurrentPathIndex];
    FVector JumpVelocity = CurrentTarget.JumpVelocity;
    
    if (!JumpVelocity.IsZero())
    {
        bIsJumping = true;
        OwnerCharacter->LaunchCharacter(JumpVelocity, true, true);
    }
}

void UAIJumpComponent::HandleLanding()
{
    bIsJumping = false;
    CurrentJumpRetryCount = 0;
    
    if (CurrentPathIndex < CurrentPath.Num())
    {
        CurrentPathIndex++;
    }
}

bool UAIJumpComponent::IsGrounded()
{
    if (!OwnerCharacter)
        return false;
        
    return OwnerCharacter->GetCharacterMovement()->IsMovingOnGround();
}

#pragma endregion

#pragma region Jump Physics Calculation

FVector UAIJumpComponent::CalculateOptimalJumpVelocity(const FVector& StartPos, const FVector& EndPos, float JumpPower, const FVector& WallLocation, float WallHeight)
{
    float CapsuleRadius = 42.0f;
    float CapsuleHeight = 88.0f;
    if (OwnerCharacter && OwnerCharacter->GetCapsuleComponent())
    {
        CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
        CapsuleHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
    }
    
    FVector ToTarget = (EndPos - StartPos).GetSafeNormal();
    FVector WallOverPos = WallLocation + ToTarget * 100.0f;
    WallOverPos.Z = WallLocation.Z + WallHeight + 50.0f;
    
    float JumpThreshold = JumpPower * 0.95f;
    if (WallHeight > JumpThreshold)
    {
        return FVector::ZeroVector;
    }
    
    FVector Result = CalculatePreciseJumpVelocity(StartPos, WallOverPos, JumpPower, CapsuleRadius, CapsuleHeight, WallLocation, WallHeight);
    return Result;
}

bool UAIJumpComponent::AnalyzePathObstacles(const FVector& StartPos, const FVector& EndPos, FVector& OutWallLocation, float& OutWallHeight)
{
   FVector Direction = (EndPos - StartPos).GetSafeNormal();
   float PathDistance = FVector::Dist(StartPos, EndPos);
   
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);
    if (CurrentTargetActor)
        Params.AddIgnoredActor(CurrentTargetActor);
    Params.bTraceComplex = true;
    
   FVector TraceStart = StartPos + FVector(0, 0, 10);
   FVector TraceEnd = EndPos;
   
    FHitResult HitResult;
    if (bShowDetailedDebug)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Cyan, false, 1.0f, 0, 2.0f);
    }
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, Params))
   {
       OutWallLocation = HitResult.Location;

       FVector HeightTraceStart = HitResult.Location + FVector(0, 0, OwnerJumpPower);
       FVector HeightTraceEnd = HitResult.Location - FVector(0, 0, 50);
       
        FHitResult HeightHit;
        if (bShowDetailedDebug)
        {
            DrawDebugLine(GetWorld(), HeightTraceStart, HeightTraceEnd, FColor::Magenta, false, 1.0f, 0, 2.0f);
        }
        if (GetWorld()->LineTraceSingleByChannel(HeightHit, HeightTraceStart, HeightTraceEnd, ECC_WorldStatic, Params))
       {
           OutWallHeight = HeightHit.Location.Z - StartPos.Z;
           return true;
       }
   }
   
   return false;
}

FVector UAIJumpComponent::CalculateBasicJump(const FVector& StartPos, const FVector& EndPos, float JumpPower)
{
   FVector Direction = EndPos - StartPos;
   float Distance = Direction.Size2D();
   float HeightDiff = EndPos.Z - StartPos.Z;
   
   if (Distance <= 0.0f) return FVector::ZeroVector;
   
   Direction.Normalize();
   float Gravity = FMath::Abs(GetWorld()->GetGravityZ());
   
   float a = 0.5f * Gravity;
   float b = -JumpPower;
   float c = HeightDiff;

   float Discriminant = b * b - 4.0f * a * c;
   if (Discriminant < 0.0f) return FVector::ZeroVector;

   float TimeToTarget1 = (-b + FMath::Sqrt(Discriminant)) / (2.0f * a);
   float TimeToTarget2 = (-b - FMath::Sqrt(Discriminant)) / (2.0f * a);
   float TimeToTarget = (TimeToTarget1 > 0.0f) ? TimeToTarget1 : TimeToTarget2;

   if (TimeToTarget <= 0.0f) return FVector::ZeroVector;

   float HorizontalVelocity = Distance / TimeToTarget;
   FVector JumpVelocity = Direction * HorizontalVelocity;
   JumpVelocity.Z = JumpPower;
   
   return JumpVelocity;
}

float UAIJumpComponent::CalculateSafeStartDistance(float CapsuleRadius, float WallHeight, float JumpPower)
{
   float BaseDistance = CapsuleRadius * 3.0f;
   float HeightFactor = (WallHeight / JumpPower) * CapsuleRadius * 2.0f;
   float PowerFactor = JumpPower * 0.08f;
   
   return BaseDistance + HeightFactor + PowerFactor;
}

FVector UAIJumpComponent::CalculateOptimalStartPosition(const FVector& StartPos, const FVector& WallLocation, float SafeDistance)
{
    FVector ToWall = (WallLocation - StartPos).GetSafeNormal2D();   
    FVector OptimalPos = WallLocation - ToWall * SafeDistance;
    OptimalPos.Z = StartPos.Z;
    return OptimalPos;
}

FVector UAIJumpComponent::CalculatePreciseJumpVelocity(const FVector& StartPos, const FVector& EndPos, float JumpPower, float CapsuleRadius, float CapsuleHeight, const FVector& WallLocation, float WallHeight)
{      
   FVector Direction = EndPos - StartPos;
   float Distance = Direction.Size2D();
   float HeightDiff = EndPos.Z - StartPos.Z;
    
   if (Distance <= 0.0f) 
   {
       return FVector::ZeroVector;
   }

   Direction.Normalize();
   float Gravity = FMath::Abs(GetWorld()->GetGravityZ());

   float RequiredVelocity = FMath::Sqrt(2.0f * Gravity * (HeightDiff + 10.0f));
   RequiredVelocity = FMath::Min(RequiredVelocity, JumpPower);

   float a = 0.5f * Gravity;
   float b = -RequiredVelocity;
   float c = HeightDiff;

   float Discriminant = b * b - 4.0f * a * c;
   if (Discriminant < 0.0f)
   {
       return FVector::ZeroVector;
   }

   float TimeToTarget1 = (-b + FMath::Sqrt(Discriminant)) / (2.0f * a);
   float TimeToTarget2 = (-b - FMath::Sqrt(Discriminant)) / (2.0f * a);
   float TimeToTarget = (TimeToTarget1 > 0.0f) ? TimeToTarget1 : TimeToTarget2;

   if (TimeToTarget <= 0.0f)
   {
       return FVector::ZeroVector;
   }

   float HorizontalVelocity = Distance / TimeToTarget;
   FVector JumpVelocity = Direction * HorizontalVelocity;
   JumpVelocity.Z = RequiredVelocity;

   if (ValidateTrajectoryWithCapsule(StartPos, JumpVelocity, CapsuleRadius, CapsuleHeight, WallLocation, WallHeight))
   {
       return JumpVelocity;
   }

   return FVector::ZeroVector;
}

bool UAIJumpComponent::ValidateTrajectoryWithCapsule(const FVector& StartPos, const FVector& Velocity,
                                                     float CapsuleRadius, float CapsuleHeight,
                                                     const FVector& WallLocation, float WallHeight)
{
  float Gravity = GetWorld()->GetGravityZ();
  float TimeStep = 0.02f;
  float MaxTime = 2.0f;
  
  FVector CurrentPos = StartPos;
  FVector CurrentVel = Velocity;
  
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(OwnerCharacter);
  if (CurrentTargetActor)
      Params.AddIgnoredActor(CurrentTargetActor);
  
  for (float Time = 0.0f; Time <= MaxTime; Time += TimeStep)
  {
      FVector NextPos = CurrentPos + CurrentVel * TimeStep;
      FVector CapsuleCenter = NextPos + FVector(0, 0, CapsuleHeight * 0.5f);
            
      FVector SweepStart = CurrentPos + FVector(0, 0, CapsuleHeight * 0.5f);
      FVector SweepEnd = NextPos + FVector(0, 0, CapsuleHeight * 0.5f);
      if (bShowDetailedDebug)
      {
          DrawDebugCapsule(GetWorld(), SweepStart, CapsuleHeight * 0.5f, CapsuleRadius, FQuat::Identity, FColor::Orange, false, 1.0f, 0, 1.0f);
          DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Orange, false, 1.0f, 0, 1.0f);
      }
      
      FHitResult SweepHitResult;
      if (GetWorld()->SweepSingleByChannel(SweepHitResult, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldStatic, 
                                         FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHeight * 0.5f), Params))
      {
          AActor* HitActor = SweepHitResult.GetActor();
          float WallTopZ = WallLocation.Z + WallHeight;
          float CollisionZ = SweepHitResult.Location.Z;
                    
          if (CollisionZ >= WallTopZ - 10.0f)
          {
              continue;
          }
          return false;
      }
      
      CurrentPos = NextPos;
      CurrentVel.Z += Gravity * TimeStep;
      
      if (CurrentPos.Z <= StartPos.Z - 100.0f) 
      {
          break;
      }
  }
  return true;
}

#pragma endregion

#pragma region Path Calculation System

void UAIJumpComponent::UpdatePathToTarget()
{
    if (!CurrentTargetActor || !OwnerCharacter)
    {
        return;
    }
       
    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector TargetLocation = CurrentTargetActor->GetActorLocation();
   
    TArray<FJumpPoint> NewPath;
   
    bool bPathFound = CalculateOptimalPath(StartLocation, TargetLocation, NewPath);
   
    if (bPathFound)
    {
        CurrentPath = NewPath;
        CurrentPathIndex = 0;
        CurrentDestination = CurrentPath.Num() > 0 ? CurrentPath[0].Location : TargetLocation;
    }
}

bool UAIJumpComponent::CalculateOptimalPath(const FVector& StartLocation, const FVector& TargetLocation, TArray<FJumpPoint>& OutPath)
{
    OutPath.Empty();
    
    float JumpPower = OwnerJumpPower;
    
    FVector WallLocation;
    float WallHeight;
    bool bWallDetected = DetectNearbyWall(StartLocation, WallLocation, WallHeight);
    
    FVector CyanWallLocation;
    float CyanWallHeight;
    bool bDirectPathHasObstacles = AnalyzePathObstacles(StartLocation, TargetLocation, CyanWallLocation, CyanWallHeight);
    
    if (IsDirectPathPossible(StartLocation, TargetLocation, JumpPower))
    {
        FJumpPoint StartPoint;
        StartPoint.Location = StartLocation;
        StartPoint.bRequiresJump = false;
        
        FJumpPoint EndPoint;
        EndPoint.Location = TargetLocation;
        EndPoint.bRequiresJump = bDirectPathHasObstacles || bWallDetected;
        
        if (EndPoint.bRequiresJump && bWallDetected)
        {
            float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
            float SafeDistance = CalculateSafeStartDistance(CapsuleRadius, WallHeight, OwnerJumpPower);
            FVector OptimalStartPos = CalculateOptimalStartPosition(StartLocation, WallLocation, SafeDistance);
            
            FVector JumpVelocity = CalculateOptimalJumpVelocity(OptimalStartPos, TargetLocation, JumpPower, WallLocation, WallHeight);
            if (JumpVelocity.IsZero())
            {
                return false;
            }
            
            EndPoint.JumpVelocity = JumpVelocity;
            EndPoint.JumpStartPos = OptimalStartPos;
        }
        
        OutPath.Add(StartPoint);
        OutPath.Add(EndPoint);
        return true;
    }
    
    if (FindNavigationPath(StartLocation, TargetLocation, OutPath))
    {
        return true;
    }
    
    if (CreateComplexJumpPath(StartLocation, TargetLocation, OutPath))
    {
        return true;
    }
    
    return false;
}

bool UAIJumpComponent::IsDirectPathPossible(const FVector& StartPos, const FVector& EndPos, float JumpPower)
{
    FVector WallLocation;
    float WallHeight;
    bool bHasObstacles = AnalyzePathObstacles(StartPos, EndPos, WallLocation, WallHeight);

    if (bHasObstacles)
    {
        if (CanJumpOverObstacle(StartPos, WallLocation, JumpPower))
        {
            return true;
        }
        return false;
    }
    return true;
}

#pragma region Terrain Analysis System

bool UAIJumpComponent::CanJumpOverObstacle(const FVector& StartPos, const FVector& EndPos, float JumpPower)
{
    float Distance = FVector::Dist2D(StartPos, EndPos);
    if (Distance > OwnerJumpDistance)
        return false;
    
    float HeightDiff = EndPos.Z - StartPos.Z;
    if (HeightDiff > OwnerJumpPower)
        return false;
    
    return true;
}

bool UAIJumpComponent::IsObstacleJumpable(const FVector& StartPos, const FVector& ObstaclePos, float JumpPower)
{
    float ObstacleHeight = CalculateObstacleHeight(StartPos, ObstaclePos);
    
    return ObstacleHeight <= OwnerJumpPower; 
}

float UAIJumpComponent::CalculateObstacleHeight(const FVector& StartPos, const FVector& EndPos)
{
    return FMath::Abs(EndPos.Z - StartPos.Z);
}

#pragma endregion

bool UAIJumpComponent::FindNavigationPath(const FVector& StartLocation, const FVector& TargetLocation, TArray<FJumpPoint>& OutPath)
{
   OutPath.Empty();
    
   UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
   if (!NavSystem)
   {
       return false;
   }
   
   if (!OwnerCharacter)
   {
       return false;
   }
   
   UNavigationPath* NavPath = NavSystem->FindPathToLocationSynchronously(GetWorld(), StartLocation, TargetLocation, OwnerCharacter);
   
   if (!NavPath)
   {
       return false;
   }
   
   if (!NavPath->IsValid())
   {
       return false;
   }
    
   const TArray<FNavPathPoint>& PathPoints = NavPath->GetPath()->GetPathPoints();
   
    for (int32 i = 0; i < PathPoints.Num(); i++)
    {
        FJumpPoint Point;
        Point.Location = PathPoints[i].Location;
        
        if (i == 0)
        {
            Point.Location = StartLocation;
        }
        else
        {
            FHitResult GroundHit;
            FVector RayStart = Point.Location + FVector(0, 0, 1000.0f);
            FVector RayEnd = Point.Location - FVector(0, 0, 1000.0f);
        
            FCollisionQueryParams TraceParams;
            TraceParams.AddIgnoredActor(OwnerCharacter);
            if (CurrentTargetActor)
                TraceParams.AddIgnoredActor(CurrentTargetActor);
        
            if (GetWorld()->LineTraceSingleByChannel(GroundHit, RayStart, RayEnd, ECC_WorldStatic, TraceParams))
            {
                Point.Location.Z = GroundHit.Location.Z;
            }
            else
            {
                Point.Location.Z = StartLocation.Z;
            }
        }
    
        Point.bRequiresJump = false;
        
       if (i < PathPoints.Num() - 1)
       {
           FVector WallLocation;
           float WallHeight;
           if (AnalyzePathObstacles(Point.Location, PathPoints[i + 1].Location, WallLocation, WallHeight))
           {
               if (WallHeight <= OwnerJumpPower && CanJumpOverObstacle(Point.Location, PathPoints[i + 1].Location, OwnerJumpPower))
               {
                   Point.bRequiresJump = true;
                   Point.RequiredJumpPower = OwnerJumpPower;
                   
                   float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
                   float SafeDistance = CalculateSafeStartDistance(CapsuleRadius, WallHeight, OwnerJumpPower);
                   Point.JumpStartPos = CalculateOptimalStartPosition(Point.Location, WallLocation, SafeDistance);
                   Point.JumpVelocity = CalculateOptimalJumpVelocity(Point.JumpStartPos, PathPoints[i + 1].Location, OwnerJumpPower, WallLocation, WallHeight);
               }
           }
       }
       
       OutPath.Add(Point);
   }
   
   return OutPath.Num() > 0;
}

bool UAIJumpComponent::CreateComplexJumpPath(const FVector& StartLocation, const FVector& TargetLocation, TArray<FJumpPoint>& OutPath)
{
    return FindNavigationPath(StartLocation, TargetLocation, OutPath);
}

float UAIJumpComponent::GetJumpPowerFromOwner()
{
    if (!OwnerCharacter)
        return 0.0f;
        
    UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
    if (MovementComp)
    {
        return MovementComp->JumpZVelocity;
    }
    
    return 0.0f;
}

#pragma endregion

#pragma region Target Tracking System

bool UAIJumpComponent::GetTargetActorFromBlackboard()
{
    if (!BlackboardComponent)
        return false;
        
    AActor* NewTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(FName("TargetActor")));
    
    if (NewTarget != CurrentTargetActor)
    {
        CurrentTargetActor = NewTarget;
        if (CurrentTargetActor)
        {
            LastPathCalculationTime = GetWorld()->GetTimeSeconds();
        }
    }
    
    return CurrentTargetActor != nullptr;
}

bool UAIJumpComponent::HasReachedTarget()
{
    if (!CurrentTargetActor || !OwnerCharacter)
    {
        return false;
    }
   
    float CheckDistance = TargetReachDistance;
      
    float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTargetActor->GetActorLocation());
    // UE_LOG(LogTemp, Warning, TEXT("HasReachedTarget: Distance=%.2f, CheckDistance=%.2f, Target=%s"), 
    //        Distance, CheckDistance, *CurrentTargetActor->GetName());
  
    bool bReached = Distance <= CheckDistance;
    // if (bReached)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("HasReachedTarget: TARGET REACHED!"));
    // }
  
    return bReached;
}

#pragma endregion

void UAIJumpComponent::ActivateJumpComponent(float InTargetReachDistance)
{
    SetComponentTickEnabled(true);
    bIsComponentActive = true;
    TargetReachDistance = InTargetReachDistance;
    ResetJumpState();
}

void UAIJumpComponent::DeactivateJumpComponent()
{
    SetComponentTickEnabled(false);
    bIsComponentActive = false;
    ResetJumpState();
}

void UAIJumpComponent::ResetJumpState()
{
    bIsJumping = false;
    CurrentJumpRetryCount = 0;
    CurrentPath.Empty();
    CurrentPathIndex = 0;
    CurrentDestination = FVector::ZeroVector;
}

#pragma endregion

bool UAIJumpComponent::DetectNearbyWall(const FVector& StartPos, FVector& OutWallLocation, float& OutWallHeight)
{
    FVector ForwardDirection = OwnerCharacter->GetActorForwardVector();
    ForwardDirection.Z = 0.0f;
    ForwardDirection = ForwardDirection.GetSafeNormal();
    
    float CapsuleRadius = 42.0f;
    if (OwnerCharacter && OwnerCharacter->GetCapsuleComponent())
    {
        CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
    }
    
    float ScanDistance = OwnerJumpPower * 2.5f;
        
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerCharacter);
    if (CurrentTargetActor)
        Params.AddIgnoredActor(CurrentTargetActor);
    Params.bTraceComplex = true;
    
    FVector TraceStart = StartPos + FVector(0, 0, 10);
    FVector TraceEnd = TraceStart + ForwardDirection * ScanDistance;
    
    FHitResult HitResult;
    bool bHitSomething = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, Params);
    if (bShowDetailedDebug)
    {
        DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 0.1f, 0, 2.0f);
    }
    
    if (bHitSomething)
    {
        AActor* HitActor = HitResult.GetActor();
        
        OutWallLocation = HitResult.Location;
        
        FVector ToWall = (HitResult.Location - StartPos).GetSafeNormal();
        FVector BeyondWall = HitResult.Location + ToWall * 100.0f;
        FVector HeightTraceStart = BeyondWall + FVector(0, 0, OwnerJumpPower);
        FVector HeightTraceEnd = BeyondWall - FVector(0, 0, 50.0f);
        
        FHitResult TopHit;
        bool bFoundTop = GetWorld()->LineTraceSingleByChannel(TopHit, HeightTraceStart, HeightTraceEnd, ECC_WorldStatic, Params);

        if (bFoundTop)
        {
            OutWallHeight = TopHit.Location.Z - StartPos.Z;
            return true;
        }
        
        FHitResult HeightHit;
        bool bHeightHit = GetWorld()->LineTraceSingleByChannel(HeightHit, HeightTraceStart, HeightTraceEnd, ECC_WorldStatic, Params);
        if (bHeightHit)
        {
            OutWallHeight = HeightHit.Location.Z - StartPos.Z;
            return true;
        }
    }
    return false;
}