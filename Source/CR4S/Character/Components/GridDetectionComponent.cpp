#include "Character/Components/GridDetectionComponent.h"
#include "Game/System/SpawnZoneManager.h"
#include "DrawDebugHelpers.h"

UGridDetectionComponent::UGridDetectionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.2f;
}

void UGridDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    SpawnZoneManager = GetWorld()->GetSubsystem<USpawnZoneManager>();

    if (!SpawnZoneManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnZoneManager is nullptr in GridDetectionComponent"));
    }
}

void UGridDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!SpawnZoneManager) return;

    FVector ActorLocation = GetOwner()->GetActorLocation();
    FVector2D ActorLocation2D(ActorLocation.X, ActorLocation.Y);
    FIntPoint CenterCoord = SpawnZoneManager->GetGridCoord(ActorLocation2D);

    if (CenterCoord != PreviousCenterCoord)
    {
        DetectNearbyGrids();
        PreviousCenterCoord = CenterCoord;
    }
}

void UGridDetectionComponent::DetectNearbyGrids()
{
    if (!SpawnZoneManager) return;

    TSet<FIntPoint> CurrentDetectedCoords;

    FVector ActorLocation = GetOwner()->GetActorLocation();
    FVector2D ActorLocation2D(ActorLocation.X, ActorLocation.Y);

    FIntPoint CenterCoord = SpawnZoneManager->GetGridCoord(ActorLocation2D);

    for (int32 DX = -GridDetectionRange; DX <= GridDetectionRange; ++DX)
    {
        for (int32 DY = -GridDetectionRange; DY <= GridDetectionRange; ++DY)
        {
            if (FMath::Abs(DX) + FMath::Abs(DY) > GridDetectionRange)
                continue;

            FIntPoint Coord = FIntPoint(CenterCoord.X + DX, CenterCoord.Y + DY);
            CurrentDetectedCoords.Add(Coord);
#if WITH_EDITOR
            SpawnZoneManager->DrawDebugGridCell(Coord, FColor::Red, false, 15.0f, 20.0f);
#endif
            if (!PreviousDetectedCoords.Contains(Coord))
            {
                SpawnZoneManager->NotifyGridEntered(Coord);
            }
        }
    }

    for (const FIntPoint& OldCoord : PreviousDetectedCoords)
    {
        if (!CurrentDetectedCoords.Contains(OldCoord))
        {
            SpawnZoneManager->NotifyGridLeft(OldCoord);
        }
    }

    PreviousDetectedCoords = CurrentDetectedCoords;
}


