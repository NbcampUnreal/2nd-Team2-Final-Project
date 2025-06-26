// Fill out your copyright notice in the Description page of Project Settings.


#include "ModularRobot.h"

#include "CR4S.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerCharacter.h"
#include "CR4S/Character/CharacterController.h"
#include "Camera/CameraComponent.h"
#include "Character/Components/ModularRobotStatusComponent.h"
#include "Character/Components/RobotWeaponComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gimmick/Components/InteractableComponent.h"
#include "Character/Components/EnvironmentalStatusComponent.h"
#include "Character/Components/RobotInputBufferComponent.h"
#include "Character/Data/RobotPartsData.h"
#include "Components/TimelineComponent.h"
#include "Inventory/Components/RobotInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/InGame/SurvivalHUD.h"
#include "Utility/DataLoaderSubsystem.h"


// Sets default values
AModularRobot::AModularRobot()
{
	//Set Tick
	PrimaryActorTick.bCanEverTick = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	LegMesh=CreateDefaultSubobject<USkeletalMeshComponent>("LegMesh");
	ArmMesh=CreateDefaultSubobject<USkeletalMeshComponent>("ArmMesh");
	
	//Set Mesh option
	if (IsValid(GetMesh()))
	{
		GetMesh()->SetRelativeLocation_Direct({0.0f, 0.0f, -92.0f});
		GetMesh()->SetRelativeRotation_Direct({0.0f, -90.0f, 0.0f});

		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
		GetMesh()->bEnableUpdateRateOptimizations = false;

		LegMesh->SetupAttachment(GetMesh());
		ArmMesh->SetupAttachment(GetMesh());

		ArmMesh->SetLeaderPoseComponent(GetMesh());
	}
	
	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1200.0f; // The camera follows at this distance behind the character
	CameraBoom->SetWorldRotation((FRotator(-15, 0, 0)));
	CameraBoom->SocketOffset=FVector({500,300,250});
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 12.0f;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Status=CreateDefaultSubobject<UModularRobotStatusComponent>(TEXT("Status"));
	
	//InteractableComponent
	InteractComp=CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractComp"));

	WeaponManager=CreateDefaultSubobject<URobotWeaponComponent>(TEXT("WeaponManager"));

	InputBuffer=CreateDefaultSubobject<URobotInputBufferComponent>(TEXT("RobotInputBuffer"));
	
	RobotInventoryComponent = CreateDefaultSubobject<URobotInventoryComponent>(TEXT("RobotInventoryComponent"));

	EnvironmentalStatus=CreateDefaultSubobject<UEnvironmentalStatusComponent>(TEXT("EnvironmentalStatus"));

	HoverTimeLine=CreateDefaultSubobject<UTimelineComponent>(TEXT("HoverTimeLine"));
}

void AModularRobot::EquipCoreParts(const FGameplayTag& Tag)
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FCorePartsInfo CoreInfo;
	const bool bSucceed = Loader->LoadCorePartsDataByTag(Tag, CoreInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	if (CoreTag.IsValid())
	{
		UnequipCoreParts();
	}
	
	CoreTag=Tag;
	
	Status->AddAttackPower(CoreInfo.AttackPower);
	Status->AddArmorMultiplier(CoreInfo.ArmorMultiplier);
	Status->AddHeatThreshold(CoreInfo.AdditiveTemperatureThreshold);
	Status->AddColdThreshold(-(CoreInfo.AdditiveTemperatureThreshold));
	Status->AddHumidityThreshold(CoreInfo.AdditiveHumidityThreshold);
	Status->AddMaxResource(CoreInfo.MaxResource);
	Status->AddCurrentResource(CoreInfo.MaxResource);
	Status->SetEnergyConsumptionAmount(CoreInfo.EnergyConsumptionAmount);
	Status->AddMaxStun(CoreInfo.MaxStun);
}

void AModularRobot::EquipBodyParts(const FGameplayTag& Tag)
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FBodyPartsInfo BodyInfo;
	const bool bSucceed = Loader->LoadBodyPartsDataByTag(Tag, BodyInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	if (BodyTag.IsValid())
	{
		UnequipBodyParts();
	}
	
	BodyTag=Tag;

	Status->AddMaxHP(BodyInfo.MaxHealth);
	Status->AddCurrentHP(BodyInfo.MaxHealth);
	Status->AddArmor(BodyInfo.Armor);
	Status->AddMaxStun(BodyInfo.MaxStun);
	Status->AddCurrentWeight(BodyInfo.Weight);
	Status->ApplyEnergyEfficiency(BodyInfo.EnergyEfficiency);
	Status->ApplyResourceRegenModifier(BodyInfo.ResourceRegenModifier);
	Status->SetResourceRegenDelay(BodyInfo.ResourceRegenDelay);
	Status->ApplyResourceConsumptionModifier(BodyInfo.ResourceConsumptionModifier);
	Status->AddHeatThreshold(BodyInfo.HeatThreshold);
	Status->AddColdThreshold(BodyInfo.ColdThreshold);
	Status->AddHumidityThreshold(BodyInfo.HumidityThreshold);
	
	if (!CR4S_ENSURE(LogHong1,BodyInfo.SkeletalMesh)) return;
	GetMesh()->SetSkeletalMesh(BodyInfo.SkeletalMesh);
}

void AModularRobot::EquipArmParts(const FGameplayTag& Tag)
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FArmPartsInfo ArmInfo;
	const bool bSucceed = Loader->LoadArmPartsDataByTag(Tag, ArmInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	if (ArmTag.IsValid())
	{
		UnequipArmParts();
	}
	
	ArmTag=Tag;

	Status->AddMaxHP(ArmInfo.MaxHealth);
	Status->AddCurrentHP(ArmInfo.MaxHealth);
	Status->AddArmor(ArmInfo.Armor);
	Status->AddCurrentWeight(ArmInfo.Weight);
	Status->AddMaxArmMountWeight(ArmInfo.MaxArmLoad);
	Status->ApplyRecoilModifier(ArmInfo.RecoilModifier);
	Status->ApplyMeleeDamageModifier(ArmInfo.MeleeDamageModifier);
	
	if (!CR4S_ENSURE(LogHong1,ArmInfo.SkeletalMesh)) return;
	ArmMesh->SetSkeletalMesh(ArmInfo.SkeletalMesh);
}

void AModularRobot::EquipLegParts(const FGameplayTag& Tag)
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FLegPartsInfo LegInfo;
	const bool bSucceed = Loader->LoadLegPartsDataByTag(Tag, LegInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	if (LegTag.IsValid())
	{
		UnequipLegParts();
	}
	
	Status->AddMaxWeight(LegInfo.MaxTotalWeight);
	
	LegTag=Tag;
	
	Status->AddMaxHP(LegInfo.MaxHealth);
	Status->AddCurrentHP(LegInfo.MaxHealth);
	Status->AddArmor(LegInfo.Armor);
	Status->AddMaxStun(LegInfo.MaxStun);
	Status->AddCurrentWeight(LegInfo.Weight);
	RobotSettings.MaxWalkSpeed=LegInfo.MaxWalkSpeed;
	RobotSettings.LegStrength=LegInfo.LegStrength;
	GetCharacterMovement()->SetWalkableFloorAngle(LegInfo.MaxSlopeAngle);
	Status->ApplyEnergyEfficiency(LegInfo.EnergyEfficiency);
	
	if (!CR4S_ENSURE(LogHong1,LegInfo.SkeletalMesh)) return;
	LegMesh->SetSkeletalMesh(LegInfo.SkeletalMesh);

	if (!CR4S_ENSURE(LogHong1,LegInfo.AnimInstance)) return;
	LegMesh->SetAnimInstanceClass(LegInfo.AnimInstance);
	
	const bool bIsQuadrupedal=Tag.MatchesTag(LegTags::Quadrupedal);
	SetLegManagerEnabled(bIsQuadrupedal);
}

void AModularRobot::EquipBoosterParts(const FGameplayTag& Tag)
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;
	
	FBoosterPartsInfo BoosterInfo;
	const bool bSuccessed = Loader->LoadBoosterPartsDataByTag(Tag, BoosterInfo);
	if (!CR4S_ENSURE(LogHong1,bSuccessed)) return;

	if (BoosterTag.IsValid())
	{
		UnequipBoosterParts();
	}
	
	BoosterTag=Tag;

	RobotSettings.BoosterStrength=BoosterInfo.BoosterStrength;
	RobotSettings.DashCooldown=BoosterInfo.DashCooldown;
	Status->SetResourceConsumptionAmount(BoosterInfo.ResourceConsumption);
}

void AModularRobot::UnequipCoreParts()
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FCorePartsInfo CoreInfo;
	const bool bSucceed = Loader->LoadCorePartsDataByTag(CoreTag, CoreInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	CoreTag=FGameplayTag::EmptyTag;
	
	Status->AddAttackPower(-(CoreInfo.AttackPower));
	Status->AddArmorMultiplier(-(CoreInfo.ArmorMultiplier));
	Status->AddHeatThreshold(-(CoreInfo.AdditiveTemperatureThreshold));
	Status->AddColdThreshold(CoreInfo.AdditiveTemperatureThreshold);
	Status->AddHumidityThreshold(-(CoreInfo.AdditiveHumidityThreshold));
	Status->AddCurrentResource(-(CoreInfo.MaxResource));
	Status->AddMaxResource(-(CoreInfo.MaxResource));
	Status->ResetEnergyConsumptionAmount();
	Status->AddMaxStun(-(CoreInfo.MaxStun));
}

void AModularRobot::UnequipBodyParts()
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FBodyPartsInfo BodyInfo;
	const bool bSucceed = Loader->LoadBodyPartsDataByTag(BodyTag, BodyInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	BodyTag=FGameplayTag::EmptyTag;
	
	GetMesh()->SetSkeletalMesh(nullptr);

	Status->AddMaxHP(-(BodyInfo.MaxHealth));
	Status->AddCurrentHP(-(BodyInfo.MaxHealth));
	Status->AddArmor(-(BodyInfo.Armor));
	Status->AddMaxStun(-(BodyInfo.MaxStun));
	Status->AddCurrentWeight(-(BodyInfo.Weight));
	Status->RevertEnergyEfficiency(BodyInfo.EnergyEfficiency);
	Status->RevertResourceRegenModifier(BodyInfo.ResourceRegenModifier);
	Status->ResetResourceRegenDelay();
	Status->RevertResourceConsumptionModifier(BodyInfo.ResourceConsumptionModifier);
	Status->AddHeatThreshold(-(BodyInfo.HeatThreshold));
	Status->AddColdThreshold(-(BodyInfo.ColdThreshold));
	Status->AddHumidityThreshold(-(BodyInfo.HumidityThreshold));
}

void AModularRobot::UnequipArmParts()
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FArmPartsInfo ArmInfo;
	const bool bSucceed = Loader->LoadArmPartsDataByTag(ArmTag, ArmInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	ArmTag=FGameplayTag::EmptyTag;
	
	ArmMesh->SetSkeletalMesh(nullptr);

	Status->AddMaxHP(-(ArmInfo.MaxHealth));
	Status->AddCurrentHP(-(ArmInfo.MaxHealth));
	Status->AddArmor(-(ArmInfo.Armor));
	Status->AddCurrentWeight(-(ArmInfo.Weight));
	Status->AddMaxArmMountWeight(-(ArmInfo.MaxArmLoad));
	Status->RevertRecoilModifier(ArmInfo.RecoilModifier);
	Status->RevertMeleeDamageModifier(ArmInfo.MeleeDamageModifier);
}

void AModularRobot::UnequipLegParts()
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FLegPartsInfo LegInfo;
	const bool bSucceed = Loader->LoadLegPartsDataByTag(LegTag, LegInfo);
	if (!CR4S_ENSURE(LogHong1,bSucceed)) return;

	LegTag=FGameplayTag::EmptyTag;
	
	LegMesh->SetSkeletalMesh(nullptr);
	
	LegMesh->SetAnimInstanceClass(nullptr);
	
	SetLegManagerEnabled(false);

	Status->AddMaxHP(-(LegInfo.MaxHealth));
	Status->AddCurrentHP(-(LegInfo.MaxHealth));
	Status->AddArmor(-(LegInfo.Armor));
	Status->AddMaxStun(-(LegInfo.MaxStun));
	Status->AddCurrentWeight(-(LegInfo.Weight));
	Status->AddMaxWeight(-(LegInfo.MaxTotalWeight));
	RobotSettings.MaxWalkSpeed=DefaultSettings.MaxWalkSpeed;
	RobotSettings.LegStrength=DefaultSettings.LegStrength;
	GetCharacterMovement()->SetWalkableFloorAngle(DefaultSettings.MaxSlopeAngle);
	Status->RevertEnergyEfficiency(LegInfo.EnergyEfficiency);
}

void AModularRobot::UnequipBoosterParts()
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	FBoosterPartsInfo BoosterInfo;
	const bool bSuccessed = Loader->LoadBoosterPartsDataByTag(BoosterTag, BoosterInfo);
	if (!CR4S_ENSURE(LogHong1,bSuccessed)) return;

	BoosterTag=FGameplayTag::EmptyTag;

	RobotSettings.BoosterStrength=DefaultSettings.BoosterStrength;
	RobotSettings.DashCooldown=DefaultSettings.DashCooldown;
	Status->ResetResourceConsumptionAmount();
}

void AModularRobot::TakeStun_Implementation(const float StunAmount)
{
	 Status->AddStun(StunAmount);
}

void AModularRobot::SetInputEnable(const bool bEnableInput) const
{
	APlayerController* PC=Cast<APlayerController>(GetController());
	if (!CR4S_ENSURE(LogHong1,PC)) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!CR4S_ENSURE(LogHong1,InputSubsystem)) return;
	
	if (bEnableInput)
	{
		InputSubsystem->AddMappingContext(MovementMappingContext,RobotSettings.MovementMappingContextPriority);
		InputSubsystem->AddMappingContext(UtilityMappingContext,RobotSettings.UtilityMappingContextPriority);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(MovementMappingContext);
		InputSubsystem->RemoveMappingContext(UtilityMappingContext);
	}
}

void AModularRobot::SetMovementInputEnable(const bool bEnableMovementInput) const
{
	APlayerController* PC=Cast<APlayerController>(GetController());
	if (!CR4S_ENSURE(LogHong1,PC)) return;

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!CR4S_ENSURE(LogHong1,InputSubsystem)) return;
	
	if (bEnableMovementInput)
	{
		InputSubsystem->AddMappingContext(MovementMappingContext,RobotSettings.MovementMappingContextPriority);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(MovementMappingContext);
	}
}

void AModularRobot::OnDeath()
{
	APlayerCharacter* PC=MountedCharacter;
	if (!CR4S_ENSURE(LogHong1,PC)) return;

	AController* CachedController=GetController();
	if (!CR4S_ENSURE(LogHong1,CachedController)) return;
	
	UnMountRobot();
	UGameplayStatics::ApplyDamage(
		PC,
		FLT_MAX,
		CachedController,
		this,
		UDamageType::StaticClass()
	);
}

UDataLoaderSubsystem* AModularRobot::GetDataLoaderSubsystem() const
{
	UGameInstance* GI=GetGameInstance();
	if (!CR4S_ENSURE(LogHong1,GI)) return nullptr;
	
	UDataLoaderSubsystem* Loader=GI->GetSubsystem<UDataLoaderSubsystem>();
	if (!CR4S_ENSURE(LogHong1,Loader)) return nullptr;

	return Loader;
}

void AModularRobot::LoadDataFromDataLoader()
{
	UDataLoaderSubsystem* Loader=GetDataLoaderSubsystem();
	if (!CR4S_ENSURE(LogHong1,Loader)) return;

	Loader->LoadRobotSettingsData(RobotSettings);
	DefaultSettings=RobotSettings;
}

void AModularRobot::MountRobot(AActor* InActor)
{
	if (!IsValid(InActor)) return;

	ACharacter* PreviousCharacter=Cast<ACharacter>(InActor);
	if (IsValid(PreviousCharacter))
	{
		MountedCharacter=Cast<APlayerCharacter>(PreviousCharacter);
		RobotInventoryComponent->UpdatePlayerInventoryComponent(MountedCharacter);
		PreviousCharacter->SetActorEnableCollision(false);
		PreviousCharacter->SetActorTickEnabled(false);

		FAttachmentTransformRules AttachRule(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			false
		);
		PreviousCharacter->AttachToComponent(
			GetMesh(),
			AttachRule,
			RobotSettings.MountSocketName
		);
	}
	AController* InController=Cast<AController>(PreviousCharacter->GetController());
	
	if (!IsValid(InController)) return;
	
	InController->UnPossess();
	InController->Possess(this);
	Status->StartConsumeEnergy();
}

void AModularRobot::UnMountRobot()
{
	UCharacterMovementComponent* MovementComp=GetCharacterMovement();
	if (!CR4S_ENSURE(LogHong1,MovementComp)) return;

	const bool bInAir=MovementComp->IsFalling();
	const bool bIsStopped=MovementComp->Velocity.IsNearlyZero();
	
	if (bInAir || !bIsStopped) return;
	
	ACharacter* NextCharacter=MountedCharacter.Get();
	if (IsValid(NextCharacter))
	{
		FVector PossibleLocation;
		FRotator CharRot=NextCharacter->GetActorRotation();
		const bool bCanExit=FindPossibleUnmountLocation(NextCharacter,PossibleLocation);
		if (!bCanExit)
		{
			UE_LOG(LogHong1,Warning,TEXT("Can't find unmounted location"));
			return;
		}
		
		NextCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);	
		NextCharacter->TeleportTo(PossibleLocation,CharRot,false,true);
		
		NextCharacter->SetActorEnableCollision(true);
		NextCharacter->SetActorTickEnabled(true);
	}

	if (AController* CurrentController=GetController())
	{
		CurrentController->UnPossess();
		if (IsValid(NextCharacter))
		{
			CurrentController->Possess(NextCharacter);
		}
	}
	MountedCharacter=nullptr;
	RobotInventoryComponent->UpdatePlayerInventoryComponent(MountedCharacter);
	Status->StopConsumeEnergy();
}

bool AModularRobot::FindPossibleUnmountLocation(ACharacter* CharacterToDrop, FVector& OutLocation) const
{
	UCapsuleComponent* Capsule=CharacterToDrop->GetCapsuleComponent();
	
	if (!Capsule)
	{
		return false;
	}

	const float Radius=Capsule->GetScaledCapsuleRadius();
	const float HalfHeight=Capsule->GetScaledCapsuleHalfHeight();
	const float ZOffset=HalfHeight;

	const int32 CheckCount=RobotSettings.CollisionCheckCount;
	for (int32 i=0;i<CheckCount;i++)
	{
		const float AngleRad=2*PI*i/CheckCount;
		const FVector Dir=FVector(FMath::Cos(AngleRad),FMath::Sin(AngleRad),0);

		const FVector CapsuleCenter=GetActorLocation()+Dir*RobotSettings.UnMountOffset+FVector(0,0,ZOffset);

		FCollisionQueryParams CollisionParams(NAME_None,false);
		const bool bBlocked=GetWorld()->OverlapBlockingTestByChannel(
			CapsuleCenter,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeCapsule(Radius,HalfHeight),
			CollisionParams
		);
		
		if (bIsDebugMode)
		{
			FColor CapsuleColor=bBlocked ? FColor::Red : FColor::Green;
			DrawDebugCapsule(GetWorld(),CapsuleCenter,HalfHeight,Radius,FQuat::Identity,CapsuleColor,false,3.0f);
		}

		if (!bBlocked)
		{
			OutLocation=CapsuleCenter-FVector(0,0,ZOffset);
			return true;
		}
	}

	return false;
}

void AModularRobot::InitializeWidgets() const
{
	if (ACharacterController* CurrentController=Cast<ACharacterController>(GetController()))
	{
		if (ASurvivalHUD* CurrentHUD=Cast<ASurvivalHUD>(CurrentController->GetHUD()))
		{
			if (UDefaultInGameWidget* InGameWidget=CurrentHUD->GetInGameWidget())
			{
				if (!CR4S_ENSURE(LogHong1,Status)) return;
				InGameWidget->BindWidgetsToStatus(Status);
				InGameWidget->ToggleWidgetMode(true);

				if (!CR4S_ENSURE(LogHong1,EnvironmentalStatus)) return;
				InGameWidget->BindEnvStatusWidgetToEnvStatus(EnvironmentalStatus);

				if (!CR4S_ENSURE(LogHong1,WeaponManager)) return;
				WeaponManager->BindWidgetWeapon();
			}
		}
	}
}

void AModularRobot::DisconnectWidgets() const
{
	if (ACharacterController* CurrentController=Cast<ACharacterController>(GetController()))
	{
		if (ASurvivalHUD* CurrentHUD=Cast<ASurvivalHUD>(CurrentController->GetHUD()))
		{
			if (UDefaultInGameWidget* InGameWidget=CurrentHUD->GetInGameWidget())
			{
				InGameWidget->UnbindStatusFromUI();
				InGameWidget->UnbindEnvStatusFromUI();
				InGameWidget->UnbindWeaponFromUI();
				InGameWidget->UnbindAllHomingWeaponFromUI();
			}
		}
	}
}

// Called when the game starts or when spawned
void AModularRobot::BeginPlay()
{
	Super::BeginPlay();

	LoadDataFromDataLoader();

	GetCharacterMovement()->JumpZVelocity=RobotSettings.JumpZVelocity;
	GetCharacterMovement()->MaxWalkSpeed=RobotSettings.MaxWalkSpeed;

	if (RobotSettings.HoverCurve)
	{
		FOnTimelineFloat UpdateCallback;
		UpdateCallback.BindDynamic(this,&ThisClass::OnHoverTimeLineUpdate);
		HoverTimeLine->AddInterpFloat(RobotSettings.HoverCurve,UpdateCallback);
		HoverTimeLine->SetLooping(true);
	}
	
	if (InteractComp)
	{
		InteractComp->OnTryInteract.AddUniqueDynamic(this,&AModularRobot::MountRobot);
	}
	if (Status)
	{
		Status->OnDeathState.AddUObject(this,&AModularRobot::OnDeath);
	}

	if (WeaponManager && InputBuffer)
	{
		InputBuffer->SetWeaponComponent(WeaponManager);
	}
	
	InitializeWidgets();
}

void AModularRobot::NotifyControllerChanged()
{
	const auto* PreviousPlayer{Cast<APlayerController>(PreviousController)};
	if (IsValid(PreviousPlayer))
	{
		auto* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PreviousPlayer->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(MovementMappingContext);
			InputSubsystem->RemoveMappingContext(UtilityMappingContext);
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

			InputSubsystem->AddMappingContext(MovementMappingContext, RobotSettings.MovementMappingContextPriority, Options);
			InputSubsystem->AddMappingContext(UtilityMappingContext, RobotSettings.UtilityMappingContextPriority, Options);
			
			if (!Status) return;
			const bool bEnableInput= !(Status->IsOverWeighted() || Status->IsArmOverWeighted());
			if (!bEnableInput)
			{
				SetMovementInputEnable(bEnableInput);
			}
		}
	}

	Super::NotifyControllerChanged();
}

void AModularRobot::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitializeWidgets();
}

void AModularRobot::UnPossessed()
{
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())};
		if (IsValid(InputSubsystem))
		{
			InputSubsystem->RemoveMappingContext(MovementMappingContext);
			InputSubsystem->RemoveMappingContext(UtilityMappingContext);
		}
	}
	DisconnectWidgets();
	Super::UnPossessed();
}

// Called every frame
void AModularRobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AModularRobot::Input_Move(const FInputActionValue& Value)
{
	if (!Controller||!Status->IsRobotActive()) return;
	//Move Logic
	// input is a Vector2D
	FVector2D MoveInput = Value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(),MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(),MoveInput.Y);
	}
}

void AModularRobot::Input_Look(const FInputActionValue& Value)
{
	if (!CR4S_ENSURE(LogHong1,Status->IsRobotActive())) return;
	FVector2D LookInput=Value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AModularRobot::Input_StartJump(const FInputActionValue& Value)
{
	if (!CR4S_ENSURE(LogHong1,Status->IsRobotActive())) return;
	
	if (GetCharacterMovement()->IsFalling() && !bIsHovering)
	{
		Status->StartHover();
		bIsHovering=true;
		HoverTimeLine->PlayFromStart();
	}
	else
	{
		Jump();
	}
}

void AModularRobot::Input_StopJump(const FInputActionValue& Value)
{
	if (!CR4S_ENSURE(LogHong1,Status->IsRobotActive())) return;
	
	StopJumping();

	if (bIsHovering)
	{
		Status->StopHover();
		bIsHovering=false;
		HoverTimeLine->Stop();
	}
}

void AModularRobot::Input_Dash(const FInputActionValue& Value)
{
	if (bIsDashing||!Status->HasEnoughResourceForRoll()||!Status->IsRobotActive()|| !BoosterTag.IsValid()) return;
	
	bIsDashing = true;
	
	FVector LastInput=GetLastMovementInputVector();
	FVector ForwardVector=GetActorForwardVector();
	FVector DashDirection=LastInput.IsNearlyZero()?ForwardVector:LastInput.GetSafeNormal();
	float DashPower=RobotSettings.BoosterStrength;
	FVector LaunchVelocity=FVector::ZeroVector;
	
	const bool bInAir=GetCharacterMovement()->IsFalling()||GetCharacterMovement()->IsFlying();
	if (!bInAir)
	{
		DashPower+=RobotSettings.LegStrength;
	}
	

	const float WeightBasedDivisor=Status->GetCurrentWeight()*RobotSettings.WeightFactor;
	const float FinalVelocityAmount=DashPower/WeightBasedDivisor;
	LaunchVelocity=DashDirection*FinalVelocityAmount;
	LaunchVelocity.Z += bInAir ? 0 : RobotSettings.DashZMultiplier*FinalVelocityAmount;;

	
	Status->ConsumeResourceForRoll();
	LaunchCharacter(LaunchVelocity,true,true);

	if (RobotSettings.DashCooldown<KINDA_SMALL_NUMBER) return;
	
	GetWorldTimerManager().SetTimer(
		DashCooldownTimerHandle,
		this,
		&AModularRobot::ResetDashCooldown,
		RobotSettings.DashCooldown,
		false
	);
}

void AModularRobot::ResetDashCooldown()
{
	bIsDashing = false;
}

void AModularRobot::OnHoverTimeLineUpdate(float Value)
{
	const float UpwardSpeed = RobotSettings.HoverUpwardSpeed*Value;
	if (UCharacterMovementComponent* MovementComp=GetCharacterMovement())
	{
		MovementComp->Velocity.Z=UpwardSpeed;
	}
}


// Called to bind functionality to input
void AModularRobot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		if (ACharacterController* MyController=Cast<ACharacterController>(GetController()))
		{
			// Moving
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AModularRobot::Input_Move);
			//Looking
			EnhancedInputComponent->BindAction(LookAction,ETriggerEvent::Triggered, this, &AModularRobot::Input_Look);
			//Dash
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AModularRobot::Input_Dash);
			// Jump
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AModularRobot::Input_StartJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AModularRobot::Input_StopJump);
			//Attack
			EnhancedInputComponent->BindAction(Attack1Action,ETriggerEvent::Started,WeaponManager.Get(),&URobotWeaponComponent::Input_OnAttackLeftArm);
			EnhancedInputComponent->BindAction(Attack1Action,ETriggerEvent::Completed,WeaponManager.Get(),&URobotWeaponComponent::Input_StopAttackLeftArm);
			EnhancedInputComponent->BindAction(Attack2Action,ETriggerEvent::Started,WeaponManager.Get(),&URobotWeaponComponent::Input_OnAttackRightArm);
			EnhancedInputComponent->BindAction(Attack2Action,ETriggerEvent::Completed,WeaponManager.Get(),&URobotWeaponComponent::Input_StopAttackRightArm);
			EnhancedInputComponent->BindAction(Attack3Action,ETriggerEvent::Started,WeaponManager.Get(),&URobotWeaponComponent::Input_OnAttackLeftShoulder);
			EnhancedInputComponent->BindAction(Attack3Action,ETriggerEvent::Completed,WeaponManager.Get(),&URobotWeaponComponent::Input_StopAttackLeftShoulder);
			EnhancedInputComponent->BindAction(Attack4Action,ETriggerEvent::Started,WeaponManager.Get(),&URobotWeaponComponent::Input_OnAttackRightShoulder);
			EnhancedInputComponent->BindAction(Attack4Action,ETriggerEvent::Completed,WeaponManager.Get(),&URobotWeaponComponent::Input_StopAttackRightShoulder);
			
			EnhancedInputComponent->BindAction(InteractionAction,ETriggerEvent::Started, this, &AModularRobot::UnMountRobot);
		}
	}
}

float AModularRobot::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	Status->TakeDamage(DamageAmount);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
