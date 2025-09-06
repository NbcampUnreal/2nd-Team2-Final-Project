#pragma once

#include "CoreMinimal.h"
#include "BaseInventoryComponent.h"
#include "Game/Interface/TutorialNotifiable.h"
#include "CR4S.h"
#include "Gimmick/Components/InteractionComponent.h"
#include "Inventory/UI/InventoryContainerWidget.h"
#include "PlayerInventoryComponent.generated.h"


class UPlanterBoxInventoryWidget;
class UInventoryContainerWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CR4S_API UPlayerInventoryComponent : public UBaseInventoryComponent, public ITutorialNotifiable
{
	GENERATED_BODY()

#pragma region UBaseInventoryComponent Override

public:
	UPlayerInventoryComponent();

	virtual void InitInventory() override;

	virtual FAddItemResult AddItem(FName RowName, int32 Count, UBaseInventoryItem* OriginItem = nullptr, bool bUseItemNotification = true) override;

	virtual int32 RemoveItemByRowName(const FName RowName, const int32 Count) override;
	virtual void RemoveAllItemByRowName(const FName RowName) override;
	virtual int32 GetItemCountByRowName(const FName RowName) const override;

private:
	bool bPlayerInitialized;

#pragma endregion

#pragma region ItemNotification

public:
	void AddItemNotification(bool bUseItemNotification, FName RowName, int32 AddedCount) const;
	
#pragma endregion

#pragma region CraftingFreeMode

public:
	FORCEINLINE bool IsCraftingFreeMode() const { return bIsCraftingFreeMode; }
	FORCEINLINE void ChangeCraftingFreeMode(const bool bNewCraftingFreeMode)
	{
		CR4S_Log(LogInventory, Warning, TEXT("FreMode: %s"),
		         bNewCraftingFreeMode ? TEXT("TRUE") : TEXT("FALSE"));
		bIsCraftingFreeMode = bNewCraftingFreeMode;
	}

private:
	bool bIsCraftingFreeMode;

#pragma endregion

#pragma region InventoryContainerWidget

public:
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|InventoryContainerWidget")
	void OpenPlayerInventoryWidget(const int32 CraftingDifficulty = 0) const;
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|InventoryContainerWidget")
	void OpenOtherInventoryWidget(EOpenWidgetType InventoryType, UBaseInventoryComponent* InventoryComponent) const;

	void CloseInventoryWidget() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|InventoryContainerWidget")
	void ToggleQuickSlotBarWidget();

	FORCEINLINE int32 GetInventoryContainerWidgetOrder() const { return InventoryContainerWidgetOrder; }

	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|InventoryContainerWidget")
	UPlanterBoxInventoryWidget* GetPlanterBoxInventoryWidget() const;
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|InventoryContainerWidget")
	URobotInventoryWidget* GetRobotInventoryWidget() const;

	FORCEINLINE bool IsOpen() const
	{
		if (IsValid(InventoryContainerWidgetInstance))
		{
			return InventoryContainerWidgetInstance->IsOpen();
		}

		return false;
	}

private:
	bool PrepareOpenInventory(UInteractionComponent* InteractionComponent = nullptr) const;

	UPROPERTY(EditDefaultsOnly)
	int32 InventoryContainerWidgetOrder;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryContainerWidget> InventoryContainerWidgetClass;
	UPROPERTY()
	TObjectPtr<UInventoryContainerWidget> InventoryContainerWidgetInstance;

	UPROPERTY()
	TObjectPtr<ASurvivalHUD> SurvivalHUD;

#pragma endregion

#pragma region QuickSlotInventoryComponent

public:
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|QuickSlot")
	void UseItem(const int32 Index) const;

	FORCEINLINE UBaseInventoryComponent* GetQuickSlotInventoryComponent() { return QuickSlotInventoryComponent; }
	FORCEINLINE const FGameplayTag& GetHeldToolTag() const { return HeldToolTag; }
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|QuickSlot")
	FORCEINLINE void GetHeldToolTag(FGameplayTag& OutHeldToolTag) const { OutHeldToolTag = HeldToolTag; }

	FORCEINLINE void SetHeldToolTag(const FGameplayTag& NewHeldToolTags) { HeldToolTag = NewHeldToolTags; }

private:
	UPROPERTY()
	TObjectPtr<UBaseInventoryComponent> QuickSlotInventoryComponent;

	FGameplayTag HeldToolTag;

#pragma endregion

#pragma region Save & Load

public:
	UFUNCTION(BlueprintPure, Category = "PlayerInventoryComponent|SaveGame")
	void GetPlayerInventorySaveGame(FInventorySaveGame& OutPlayerInventorySaveGame,
	                                FInventorySaveGame& OutQuickSlotSaveGame);
	UFUNCTION(BlueprintCallable, Category = "PlayerInventoryComponent|LoadGame")
	void LoadPlayerInventorySaveGame(const FInventorySaveGame& PlayerInventorySaveGame,
	                                 const FInventorySaveGame& QuickSlotSaveGame);

#pragma endregion

#pragma region Delegate

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryOpen);

	UPROPERTY(BlueprintAssignable, Category = "InventoryComponent|Delegate")
	FOnInventoryOpen OnInventoryOpen;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClose);

	UPROPERTY(BlueprintAssignable, Category = "InventoryComponent|Delegate")
	FOnInventoryClose OnInventoryClose;

#pragma endregion

#pragma region ITutorialNotifiable

#pragma endregion
};
