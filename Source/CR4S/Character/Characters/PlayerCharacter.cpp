#include "PlayerCharacter.h"
#include "AlsCameraComponent.h"
#include "AlsCharacterMovementComponent.h"
#include "CR4S.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "JsonObjectConverter.h"
#include "Character/CharacterController.h"
#include "Character/Components/PlayerCharacterStatusComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Gimmick/Components/InteractionComponent.h"
#include "Character/Components/GridDetectionComponent.h"
#include "Character/Components/EnvironmentalStatusComponent.h"
#include "UI/InGame/DefaultInGameWidget.h"
#include "UI/InGame/SurvivalHUD.h"
#include "Utility/AlsVector.h"
#include "NavigationInvokerComponent.h"
#include "Character/Components/PlayerInputBufferComponent.h"
#include "Character/Components/WeaponTraceComponent.h"
#include "Character/Weapon/BaseTool.h"
#include "Character/Weapon/PlayerTool.h"
#include "Game/SaveGame/SaveGameManager.h"
#include "Inventory/Components/PlayerInventoryComponent.h"
#include "Game/C4WidgetInteractionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


APlayerCharacter::APlayerCharacter()
{
	Camera = CreateDefaultSubobject<UAlsCameraComponent>(FName{TEXTVIEW("Camera")});
	Camera->SetupAttachment(GetMesh());
	Camera->SetRelativeRotation_Direct({0.0f, 90.0f, 0.0f});

	VisibleMesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VisibleMesh"));
	VisibleMesh->SetupAttachment(GetMesh());
	
	WeaponTrace=CreateDefaultSubobject<UWeaponTraceComponent>(TEXT("WeaponTrace"));

	PlayerInputBuffer=CreateDefaultSubobject<UPlayerInputBufferComponent>(TEXT("PlayerInputBuffer"));
	
	Status=CreateDefaultSubobject<UPlayerCharacterStatusComponent>(TEXT("Status"));

	Interaction=CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
	GridDetection = CreateDefaultSubobject<UGridDetectionComponent>(TEXT("GridDetection"));
	EnvironmentalStatus = CreateDefaultSubobject<UEnvironmentalStatusComponent>(TEXT("EnvironmentalStatus"));

	NavGenerationRadius = 2000.0f;
	NavRemovalRadius = 2500.0f;

	NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
	NavInvoker->SetGenerationRadii(NavGenerationRadius, NavRemovalRadius);

	PlayerInventory = CreateDefaultSubobject<UPlayerInventoryComponent>(TEXT("PlayerInventory"));
	WidgetInteraction = CreateDefaultSubobject<UC4WidgetInteractionComponent>(TEXT("WidgetInteraction"));

	// Disable physics interaction
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bEnablePhysicsInteraction = false;
		MoveComp->bPushForceScaledToMass = false;
		MoveComp->PushForceFactor = 0.0f;
	}
}

FName APlayerCharacter::GetUniqueSaveID()
{
	return UniqueSaveID;
}

void APlayerCharacter::SetUniqueSaveID(FName NewID)
{
	UniqueSaveID=NewID;
}

void APlayerCharacter::GatherSaveData(FSavedActorData& OutSaveData)
{
	OutSaveData.ActorClassName=SavableClassName;
	OutSaveData.ActorTransform=GetActorTransform();
	
	FPlayerCharacterSaveGame PlayerData;
	
	PlayerData.CurrentHP=Status->GetCurrentHP();
	PlayerData.CurrentResource=Status->GetCurrentResource();
	PlayerData.CurrentHunger=Status->GetCurrentHunger();
	PlayerData.CurrentTemperature=EnvironmentalStatus->GetCurrentTemperature();
	PlayerData.CurrentHumidity=EnvironmentalStatus->GetCurrentHumidity();
	PlayerData.EquippedToolTag= CurrentTool ? CurrentTool->GetToolGameplayTag() : FGameplayTag::EmptyTag;
	PlayerData.Stance=GetStance();
	PlayerData.Gait=GetGait();
	PlayerData.bIsRightShoulder=Camera->IsRightShoulder();

	PlayerInventory->GetPlayerInventorySaveGame(PlayerData.InventorySaveGame,PlayerData.QuickSlotSaveGame);

	if (!FJsonObjectConverter::UStructToJsonObjectString(PlayerData,OutSaveData.SerializedData))
	{
		UE_LOG(LogHong1,Error,TEXT("Failed to save player data"));
		OutSaveData.SerializedData.Empty();
	}
	else
	{
		UE_LOG(LogHong1,Log,TEXT("Saved player data"));
	}
}

void APlayerCharacter::ApplySaveData(FSavedActorData& InSaveData)
{
	FPlayerCharacterSaveGame PlayerData;

	if (FJsonObjectConverter::JsonObjectStringToUStruct(InSaveData.SerializedData,&PlayerData,0,0))
	{
		Status->SetCurrentHP(PlayerData.CurrentHP);
		Status->SetCurrentResource(PlayerData.CurrentResource);
		Status->OnResourceConsumed();
		Status->SetCurrentHunger(PlayerData.CurrentHunger);
		EnvironmentalStatus->SetCurrentTemperature(PlayerData.CurrentTemperature);
		EnvironmentalStatus->SetCurrentHumidity(PlayerData.CurrentHumidity);
		SetCurrentToolByTag(PlayerData.EquippedToolTag);
		SetDesiredStance(PlayerData.Stance);
		SetDesiredGait(PlayerData.Gait);
		Camera->SetRightShoulder(PlayerData.bIsRightShoulder);
		
		
		PlayerInventory->LoadPlayerInventorySaveGame(PlayerData.InventorySaveGame,PlayerData.QuickSlotSaveGame);
		PlayerInventory->SetHeldToolTag(PlayerData.EquippedToolTag);
	}
	else
	{
		UE_LOG(LogHong1,Error,TEXT("Failed to load player data"));
	}
	
}

void APlayerCharacter::OnDeath()
{
	if (IsValid(PlayerInventory) && PlayerInventory->IsOpen())
	{
		PlayerInventory->CloseInventoryWidget();
	}
	
	APlayerController* PC=Cast<APlayerController>(GetController());
	if (!CR4S_ENSURE(LogHong1,PC)) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!CR4S_ENSURE(LogHong1,InputSubsystem)) return;

	InputSubsystem->RemoveMappingContext(InputMappingContext);
	//SetOverlayMode(AlsOverlayModeTags::Default);
	StartRagdolling();
	Status->StopConsumeHunger();
}

void APlayerCharacter::SetCurrentToolByTag(const FGameplayTag& ToolTag)
{
	if (!CurrentTool||CurrentTool->GetToolGameplayTag().MatchesTagExact(ToolTag)) return;
	
	CurrentTool->SetGameplayTag(ToolTag);
	
	SetOverlayMode(ToolTag);
}

void APlayerCharacter::InitializeCurrentTool()
{
	if (CR4S_ENSURE(LogHong1,CurrentTool))
	{
		CurrentTool->Destroy();
		CurrentTool=nullptr;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator=this;
	SpawnParams.Owner=this;
	
	APlayerTool* NewTool=GetWorld()->SpawnActor<APlayerTool>(SpawnParams);
	const FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepRelative,
		true
	);
	if (!CR4S_ENSURE(LogHong1,NewTool)) return;
	
	CurrentTool=NewTool;
	CurrentTool->AttachToComponent(VisibleMesh,AttachRules,"Tool");
	
	if (!CR4S_ENSURE(LogHong1,PlayerInputBuffer)||!CR4S_ENSURE(LogHong1,WeaponTrace)) return;
	PlayerInputBuffer->SetCurrentTool(CurrentTool);
	WeaponTrace->SetCurrentTool(CurrentTool);
	CurrentTool->Initialize(this);
}

void APlayerCharacter::InitializeWidgets() const
{
	if (ACharacterController* CurrentController=Cast<ACharacterController>(GetController()))
	{
		if (ASurvivalHUD* CurrentHUD=Cast<ASurvivalHUD>(CurrentController->GetHUD()))
		{
			if (UDefaultInGameWidget* InGameWidget=CurrentHUD->GetInGameWidget())
			{
				if (!CR4S_ENSURE(LogHong1,Status)) return;
				
				InGameWidget->BindWidgetsToStatus(Status);
				InGameWidget->ToggleWidgetMode(false);

				if (!CR4S_ENSURE(LogHong1,EnvironmentalStatus)) return;
				InGameWidget->BindEnvStatusWidgetToEnvStatus(EnvironmentalStatus);
				
			}

			CurrentHUD->BindGameOverWidget(Status);
		}
	}
}

void APlayerCharacter::DisconnectWidgets() const
{
	if (ACharacterController* CurrentController=Cast<ACharacterController>(GetController()))
	{
		if (ASurvivalHUD* CurrentHUD=Cast<ASurvivalHUD>(CurrentController->GetHUD()))
		{
			if (UDefaultInGameWidget* InGameWidget=CurrentHUD->GetInGameWidget())
			{
				InGameWidget->UnbindStatusFromUI();
				InGameWidget->UnbindEnvStatusFromUI();
			}
		}
	}
}

void APlayerCharacter::NotifyControllerChanged()
{
	const auto* PreviousPlayer{Cast<APlayerController>(PreviousController)};
	if (IsValid(PreviousPlayer))
	{
		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayer->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}

	auto* NewPlayer{Cast<APlayerController>(GetController())};
	if (IsValid(NewPlayer))
	{
		NewPlayer->InputYawScale_DEPRECATED = 1.0f;
		NewPlayer->InputPitchScale_DEPRECATED = 1.0f;
		NewPlayer->InputRollScale_DEPRECATED = 1.0f;

		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(NewPlayer->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			FModifyContextOptions Options;
			Options.bNotifyUserSettings = true;

			InputSubsystem->AddMappingContext(InputMappingContext, PlayerCharacterSettings.MappingContextPriority, Options);
		}
	}
	
	Super::NotifyControllerChanged();
}

float APlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	Status->TakeDamage(DamageAmount);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void APlayerCharacter::Destroyed()
{
	if (CurrentTool)
	{
		CurrentTool->Destroy();
		CurrentTool=nullptr;
	}
	Super::Destroyed();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI=GetWorld()->GetGameInstance())
	{
		if (USaveGameManager* SaveManager=GI->GetSubsystem<USaveGameManager>())
		{
			SaveManager->RegisterSavableActor(this);
		}
	}
	
	//Binding Delegate Functions and Set up Widget
	InitializeCurrentTool(); 

	if (!CR4S_ENSURE(LogHong1,PlayerCharacterSettingsDataAsset)) return;
	PlayerCharacterSettings=PlayerCharacterSettingsDataAsset->PlayerCharacterSettings;

	if (!CR4S_ENSURE(LogHong1,Status)) return;
	Status->OnDeathState.AddUObject(this,&APlayerCharacter::OnDeath);
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI=GetWorld()->GetGameInstance())
	{
		if (USaveGameManager* SaveManager=GI->GetSubsystem<USaveGameManager>())
		{
			SaveManager->UnregisterSavableActor(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	if (Camera->IsActive())
	{
		Camera->GetViewInfo(ViewInfo);
		return;
	}

	Super::CalcCamera(DeltaTime, ViewInfo);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

	auto* EnhancedInput{Cast<UEnhancedInputComponent>(Input)};
	if (IsValid(EnhancedInput))
	{
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLookMouse);
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnLookMouse);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLook);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnLook);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnMove);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnSprint);
		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnWalk);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnCrouch);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnJump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnJump);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnAim);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnAim);
		EnhancedInput->BindAction(RagdollAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRagdoll);
		EnhancedInput->BindAction(RollAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRoll);
		EnhancedInput->BindAction(RotationModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnRotationMode);
		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnViewMode);
		EnhancedInput->BindAction(SwitchShoulderAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSwitchShoulder);
		EnhancedInput->BindAction(AttackAction,ETriggerEvent::Triggered,this,&ThisClass::Input_OnAttack);
		EnhancedInput->BindAction(InteractionAction,ETriggerEvent::Started,this,&ThisClass::Input_OnInteraction);
		EnhancedInput->BindAction(QuickSlotAction,ETriggerEvent::Triggered,this,&ThisClass::Input_OnQuickSlotNumberPressed);
		EnhancedInput->BindAction(OpenInventoryAction,ETriggerEvent::Started,this,&ThisClass::Input_OnInventoryOpen);
	}
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	Interaction->InitComponent();
	Interaction->StartDetectProcess();
	
	InitializeWidgets();
	Status->SetIsUnPossessed(false);
	PlayerInventory->InitInventory();
	PlayerInventory->ToggleQuickSlotBarWidget();
}

void APlayerCharacter::UnPossessed()
{
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}
	PlayerInventory->ToggleQuickSlotBarWidget();
	DisconnectWidgets();
	Status->SetIsUnPossessed(true);

	Super::UnPossessed();
}

void APlayerCharacter::Input_OnLookMouse(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ActionValue.Get<FVector2D>()};

	AddControllerPitchInput(Value.Y * LookUpMouseSensitivity);
	AddControllerYawInput(Value.X * LookRightMouseSensitivity);
}

void APlayerCharacter::Input_OnLook(const FInputActionValue& ActionValue)
{
	const FVector2f Value{ActionValue.Get<FVector2D>()};

	AddControllerPitchInput(Value.Y * LookUpRate);
	AddControllerYawInput(Value.X * LookRightRate);
}

void APlayerCharacter::Input_OnMove(const FInputActionValue& ActionValue)
{
	const auto Value{UAlsVector::ClampMagnitude012D(ActionValue.Get<FVector2D>())};

	const auto ForwardDirection{UAlsVector::AngleToDirectionXY(UE_REAL_TO_FLOAT(GetViewState().Rotation.Yaw))};
	const auto RightDirection{UAlsVector::PerpendicularCounterClockwiseXY(ForwardDirection)};

	AddMovementInput(ForwardDirection * Value.Y + RightDirection * Value.X);
}

void APlayerCharacter::Input_OnSprint(const FInputActionValue& ActionValue)
{
	const bool bIsSprinting=ActionValue.Get<bool>() && !GetLastMovementInputVector().IsNearlyZero();
	
	SetDesiredGait(bIsSprinting ? AlsGaitTags::Sprinting : AlsGaitTags::Running);
	if (bIsSprinting)
	{
		Status->StartSprint();
	}
	else
	{
		Status->StopSprint();
	}
}

void APlayerCharacter::Input_OnWalk()
{
	if (GetDesiredGait() == AlsGaitTags::Walking)
	{
		SetDesiredGait(AlsGaitTags::Running);
	}
	else if (GetDesiredGait() == AlsGaitTags::Running)
	{
		SetDesiredGait(AlsGaitTags::Walking);
	}
}

void APlayerCharacter::Input_OnCrouch()
{
	if (GetDesiredStance() == AlsStanceTags::Standing)
	{
		SetDesiredStance(AlsStanceTags::Crouching);
	}
	else if (GetDesiredStance() == AlsStanceTags::Crouching)
	{
		SetDesiredStance(AlsStanceTags::Standing);
	}
}

void APlayerCharacter::Input_OnJump(const FInputActionValue& ActionValue)
{
	if (ActionValue.Get<bool>())
	{
		if (StopRagdolling())
		{
			return;
		}

		if (StartMantlingGrounded())
		{
			return;
		}

		if (GetStance() == AlsStanceTags::Crouching)
		{
			SetDesiredStance(AlsStanceTags::Standing);
			return;
		}

		Jump();
	}
	else
	{
		StopJumping();
	}
}

void APlayerCharacter::Input_OnAim(const FInputActionValue& ActionValue)
{
	SetDesiredAiming(ActionValue.Get<bool>());
}

void APlayerCharacter::Input_OnRagdoll()
{
	if (!StopRagdolling())
	{
		StartRagdolling();
	}
}

void APlayerCharacter::Input_OnRoll()
{
	static constexpr auto PlayRate{1.3f};
	if (!CR4S_ENSURE(LogHong1,Status->HasEnoughResourceForRoll()
		&& LocomotionAction!=AlsLocomotionActionTags::Rolling)) return;
		
	StartRolling(PlayRate);
	Status->ConsumeResourceForRoll();
}

void APlayerCharacter::Input_OnRotationMode()
{
	SetDesiredRotationMode(GetDesiredRotationMode() == AlsRotationModeTags::VelocityDirection
		                       ? AlsRotationModeTags::ViewDirection
		                       : AlsRotationModeTags::VelocityDirection);
}

void APlayerCharacter::Input_OnViewMode()
{
	SetViewMode(GetViewMode() == AlsViewModeTags::ThirdPerson ? AlsViewModeTags::FirstPerson : AlsViewModeTags::ThirdPerson);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void APlayerCharacter::Input_OnSwitchShoulder()
{
	Camera->SetRightShoulder(!Camera->IsRightShoulder());
}

void APlayerCharacter::Input_OnAttack() 
{
	if (!CR4S_ENSURE(LogHong1,CurrentTool)
		||!PlayerInputBuffer->CheckInputQueue(EInputType::Attack))
	{
		return;
	}
	CurrentTool->OnAttack();
}

void APlayerCharacter::Input_OnInteraction()
{
	if (!CR4S_ENSURE(LogHong1, GetLocomotionMode()==AlsLocomotionModeTags::Grounded
		&& AlsCharacterMovement->GetGaitAmount()<=KINDA_SMALL_NUMBER))
	{
		return;
	}
	
	CR4S_ENSURE(LogHong1,Interaction->TryStartInteraction());
}

void APlayerCharacter::Input_OnQuickSlotNumberPressed(const FInputActionValue& ActionValue)
{
	float RawValue=ActionValue.Get<float>();
	int32 Index=FMath::RoundToInt(RawValue)-1;

	if (Index>=0 && Index<=9)
	{
		PlayerInventory->UseItem(Index);
		UE_LOG(LogHong1,Warning,TEXT("Quick Slot : %d"),Index);
	}
}

void APlayerCharacter::Input_OnInventoryOpen()
{
	if (PlayerInventory)
	{
		PlayerInventory->OpenPlayerInventoryWidget(0);
	}
}

void APlayerCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation)
{
	if (Camera->IsActive())
	{
		Camera->DisplayDebug(Canvas, DisplayInfo, VerticalLocation);
	}

	Super::DisplayDebug(Canvas, DisplayInfo, Unused, VerticalLocation);
}
