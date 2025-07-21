#pragma once

#include "Components/ActorComponent.h"
#include "GridDetectionComponent.generated.h"

class USpawnZoneManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CR4S_API UGridDetectionComponent : public UActorComponent
{
    GENERATED_BODY()

#pragma region Lifecycle Functions
public:
    UGridDetectionComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion

#pragma region GridDetection
public:
    //GetCurrentDection

    UPROPERTY(EditAnywhere, Category = "Detection")
    int32 GridDetectionRange = 2;

    void DetectNearbyGrids();

protected:
    UPROPERTY()
    TObjectPtr<USpawnZoneManager> SpawnZoneManager;

private:
    FIntPoint PreviousCenterCoord = FIntPoint(ForceInit);
    TSet<FIntPoint> PreviousDetectedCoords;

#pragma endregion
};
