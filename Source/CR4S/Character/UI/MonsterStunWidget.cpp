// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterStunWidget.h"

#include "CR4S.h"
#include "RoundProgressBar.h"
#include "MonsterAI/Components/MonsterStateComponent.h"

void UMonsterStunWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Stun->SetVisibility(ESlateVisibility::Hidden);
}

void UMonsterStunWidget::InitializeWidgetForTargetComp(UMonsterStateComponent* TargetComp)
{
	if (!CR4S_ENSURE(LogHong1,TargetComp)) return;
	if (CachedTargetComponent.IsValid() && CachedTargetComponent==TargetComp)
	{
		return;
	}

	ClearBinding();
	
	CachedTargetComponent=TargetComp;
	Stun->SetVisibility(ESlateVisibility::HitTestInvisible);
	
	CachedTargetComponent->OnStunChangedDelegate.AddUniqueDynamic(this,&UMonsterStunWidget::UpdateStunWidget);
	
	GetWorld()->GetTimerManager().ClearTimer(WidgetVisibleTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		WidgetVisibleTimerHandle,
		this,
		&UMonsterStunWidget::ClearBinding,
		VisibleDuration,
		false
	);
}

void UMonsterStunWidget::ClearBinding()
{
	if (!CR4S_ENSURE(LogHong1,Stun&&CachedTargetComponent.IsValid())) return;

	Stun->SetVisibility(ESlateVisibility::Hidden);
	CachedTargetComponent->OnStunChangedDelegate.RemoveAll(this);
	CachedTargetComponent=nullptr;
}

void UMonsterStunWidget::UpdateStunWidget()
{
	if (!CR4S_ENSURE(LogHong1,Stun&&CachedTargetComponent.IsValid())) return;

	const float Percent=CachedTargetComponent->GetCurrentStunAmount()*MaxPercent;
	Stun->SetPercent(Percent);

	GetWorld()->GetTimerManager().ClearTimer(WidgetVisibleTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		WidgetVisibleTimerHandle,
		this,
		&UMonsterStunWidget::ClearBinding,
		VisibleDuration,
		false
	);
}

