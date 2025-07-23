#pragma once

#include "Blueprint/UserWidget.h"
#include "Game/System/WorldTimeManager.h"
#include "DefaultInGameWidget.generated.h"

class UMonsterStateComponent;
class UMonsterStunWidget;
class UWeaponInfoBlockWidget;
class UAdditiveRobotStatusWidget;
class UEnvironmentalStatusComponent;
class ABaseWeapon;
class UAmmoWidget;
class URoundProgressBar;
class UImage;
class AHomingWeapon;
class ULockOnWidget;
class UBaseStatusComponent;
class UProgressBarWidget;
class UCharacterStatusWidget;
class UPlayerCharacterStatusComponent;
class UCharacterEnvironmentStatusWidget;
class UTimeDisplayWidget;
class UGuideWidget;

UCLASS()
class CR4S_API UDefaultInGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

#pragma region Get
	FORCEINLINE UCharacterEnvironmentStatusWidget* GetEnvironmentStatusWidget() { return EnvironmentStatusWidget; }
#pragma endregion
	
#pragma region Bind & Unbind
public:
	void ToggleWidgetMode(const bool bIsRobot);
	
	UFUNCTION(BlueprintCallable)
	void BindLockOnWidgetToHomingWeapon(AHomingWeapon* HomingWeapon, const int32 SlotIdx);
	void UnbindAllHomingWeaponFromUI();
	
	UFUNCTION(BlueprintCallable)
	void BindAmmoWidgetToWeapon(ABaseWeapon* InWeapon, const int32 SlotIdx);
	void UnbindWeaponFromUI();

	void BindWidgetsToStatus(UBaseStatusComponent* InStatus);
	void UnbindStatusFromUI();
	
	void BindEnvStatusWidgetToEnvStatus(UEnvironmentalStatusComponent* InStatus);
	void UnbindEnvStatusFromUI();

	void BindMonsterStunWidgetToMonster(UMonsterStateComponent* InMonsterComp);

#pragma endregion
	
#pragma region UpdateWidget
public:
	void UpdateHungerWidget(const float InPercentage);
	void UpdateTimeWidget(FWorldTimeData CurrentTimeData);

	void ShowGuideWidget(int32 DisplayTime);
	void HideGuideWidget();
#pragma endregion

#pragma region CrosshairImage
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Crosshair)
	TObjectPtr<UTexture2D> DefaultCrosshair;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Crosshair)
	TObjectPtr<UTexture2D> RobotCrosshair;
#pragma endregion 
	
#pragma region Widgets
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UCharacterStatusWidget> StatusWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UAdditiveRobotStatusWidget> AdditiveRobotStatusWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBarWidget> HungerWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTimeDisplayWidget> TimeDisplayWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCharacterEnvironmentStatusWidget> EnvironmentStatusWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UImage> CrosshairWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UImage> AimCircle;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UAmmoWidget> CurrentAmmoWidgets;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<UWeaponInfoBlockWidget> WeaponInfoBlockWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UGuideWidget> GuideWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UMonsterStunWidget> MonsterStunWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ULockOnWidget>> LockOnWidgets;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<ULockOnWidget> LockOnWidget0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<ULockOnWidget> LockOnWidget1;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<ULockOnWidget> LockOnWidget2;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(BindWidgetOptional))
	TObjectPtr<ULockOnWidget> LockOnWidget3;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> ToggleGuideAnim;

#pragma endregion

#pragma region Widget Timer

	FTimerHandle GuideWidgetTimerHandle;

#pragma endregion
};
