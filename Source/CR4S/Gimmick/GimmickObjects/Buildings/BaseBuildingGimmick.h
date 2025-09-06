#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Interface/TutorialNotifiable.h"
#include "GameplayTagContainer.h"
#include "BaseBuildingGimmick.generated.h"

class UEnvironmentalStatusComponent;
class UObjectShakeComponent;
class UItemGimmickSubsystem;
class UWidgetComponent;
class UBuildingDurabilityWidget;

USTRUCT(BlueprintType)
struct FBuildingEnvironmentalStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTemperature = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinTemperature = -30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHumidity = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHumidity = 0.f;
};

UCLASS(BlueprintType, Blueprintable)
class CR4S_API ABaseBuildingGimmick : public AActor, public ITutorialNotifiable
{
	GENERATED_BODY()

#pragma region AActor Override

public:
	ABaseBuildingGimmick();

	virtual void BeginPlay() override;

#pragma endregion

#pragma region Components

private:
	UPROPERTY(VisibleAnywhere, Category = "BuildingGimmick|Components")
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditDefaultsOnly, Category = "BuildingGimmick|Components")
	TObjectPtr<UWidgetComponent> BuildingDurabilityWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildingGimmick|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UObjectShakeComponent> ShakeComponent;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildingGimmick|Components",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnvironmentalStatusComponent> EnvironmentalStatusComponent;

#pragma endregion

#pragma region BuildingGimmick

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "BuildingGimmick|Durability")
	void OnGetResources(AActor* InventoryOwnerActor);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "BuildingGimmick|Durability")
	void OnRepairBuilding(const float AddDurability);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "BuildingGimmick|Durability")
	float GetDurabilityRatio() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BuildingGimmick")
	TObjectPtr<UItemGimmickSubsystem> ItemGimmickSubsystem;

#pragma endregion

#pragma region DurabilityWidget

protected:
	UFUNCTION(BlueprintCallable, Category = "BuildingGimmick|DurabilityWidget")
	void ToggleDurabilityWidget(bool bIsOpen);

private:
	UPROPERTY(BlueprintReadOnly, Category = "BuildingGimmick|DurabilityWidget", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBuildingDurabilityWidget> BuildingDurabilityWidgetInstance;

#pragma endregion

#pragma region EnvironmentalStatus

protected:
	UFUNCTION(BlueprintCallable, Category = "BuildingGimmick|EnvironmentalStatus")
	void InitEnvironmentalStatus(const FBuildingEnvironmentalStatus& InEnvironmentalStatus);

#pragma endregion

#pragma region Burn

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BuildingGimmick|Burn")
	void ApplyBurn(float ActualSeconds);
	
private:
	UFUNCTION(BlueprintCallable, Category = "BuildingGimmick|Burn")
	void StartBurn();
	UFUNCTION(BlueprintCallable, Category = "BuildingGimmick|Burn")
	void EndBurn();
	
	UFUNCTION()
	void UpdateWorldTime(int64 NewPlayTime);
	
	int64 PrevPlayTime;
	
#pragma endregion 

#pragma region Tutorial Notification
public:
	//Function that maps RowNames to GameplayTags in Editor
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	FGameplayTag GetTutorialTagByRowName(FName RowName) const;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void NotifyTutorial(const FGameplayTag& TutorialTag);

#pragma endregion 
};
