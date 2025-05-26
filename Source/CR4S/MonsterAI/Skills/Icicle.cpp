#include "MonsterAI/Skills/Icicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "MonsterAI/Components/MonsterAttributeComponent.h"
#include "MonsterAI/Components/MonsterSkillComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AIcicle::AIcicle()
{
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = RootComp;

	// TODO :: I Will Change NiagaraComponent
/*	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComp);*/	
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetupAttachment(RootComp);
	NiagaraComp->SetAutoActivate(false);
	NiagaraComp->Activate(true);
}

void AIcicle::InitIcicle(float Radius)
{
	if (APawn* OwnerPawn = GetInstigator())
	{
		if (UMonsterSkillComponent* SkillComp = OwnerPawn->FindComponentByClass<UMonsterSkillComponent>())
		{
			Damage = SkillComp->GetCurrentSkillData().Damage;
		}
	}

	NiagaraComp->SetAsset(NiagaraTemplate);
	//NiagaraComp->SetRelativeScale3D(FVector(Radius / 100.f));  // �����ϸ�

	//NiagaraComp->Deactivate();
	//NiagaraComp->ResetSystem();
	//NiagaraComp->Activate(true);

	if (NiagaraTemplate)
	{
		// RootComp �� �Ź� ���ο� �ý��� �ٿ��� ����
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraTemplate,
			RootComp,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true  // AutoDestroy = true
		)->SetRelativeScale3D(FVector(Radius / 100.f));
	}

	for (UCapsuleComponent* C : EdgeColliders)
		if (C) C->DestroyComponent();
	EdgeColliders.Empty();

	const float Circumference = 2.f * PI * Radius;
	const float CoveragePerCapsule = (CapsuleRadius * 2.f) * CoverageFactor;
	int32 NumSeg = FMath::CeilToInt(Circumference / CoveragePerCapsule);
	NumSeg = FMath::Max(3, NumSeg);

	const FVector BaseLoc = GetActorLocation();
	const float BottomZ = BaseLoc.Z;
	const float CenterZ = BottomZ + CapsuleHalfHeight;

	const float TwoPi = 2.f * PI;
	for (int32 i = 0; i < NumSeg; i++)
	{
		float Angle = TwoPi * float(i) / float(NumSeg);
		FVector Dir = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f);
		//FVector Loc = GetActorLocation() + Dir * Radius;

		FVector LocXY(BaseLoc.X + Dir.X * Radius, BaseLoc.Y + Dir.Y * Radius, 0.f);
		FVector CapsuleCenter(LocXY.X, LocXY.Y, CenterZ);

		UCapsuleComponent* CapsuleComp = NewObject<UCapsuleComponent>(this);
		CapsuleComp->AttachToComponent(RootComp, FAttachmentTransformRules::KeepWorldTransform);
		CapsuleComp->RegisterComponent();

		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		CapsuleComp->SetWorldLocation(CapsuleCenter);
		CapsuleComp->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
		CapsuleComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

		CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AIcicle::OnOverlap);
		EdgeColliders.Add(CapsuleComp);

		DrawDebugCapsule(
			GetWorld(),
			CapsuleCenter,
			CapsuleHalfHeight,
			CapsuleRadius,
			FQuat::Identity,
			FColor::Cyan,
			false,    // persistent
			1.f,    // lifetime
			0,       // depthPriority
			2.0f     // thickness
		);
	}

	SetLifeSpan(1.f);
}

void AIcicle::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner() || OtherActor == GetInstigator()) return;

	if (OtherActor->FindComponentByClass<UMonsterAttributeComponent>())
		return;

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);
}
