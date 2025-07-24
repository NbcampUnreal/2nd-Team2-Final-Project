#include "MonsterAI/UI/MonsterHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MonsterAI/BaseMonster.h"
#include "MonsterAI/Components/MonsterAttributeComponent.h"

void UMonsterHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMonsterHPBarWidget::NativeDestruct()
{
	UnbindFromMonster();
	Super::NativeDestruct();
}

void UMonsterHPBarWidget::BindToMonster(ABaseMonster* Monster)
{
	UnbindFromMonster();

	if (!Monster)
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	BoundMonster = Monster;
	BoundAttributeComponent = Monster->GetMonsterAttribute();

	if (BoundAttributeComponent.IsValid())
	{
		BoundAttributeComponent->OnDeath.AddDynamic(this, &UMonsterHPBarWidget::OnMonsterDeath);
		BoundAttributeComponent->OnHPChanged.AddDynamic(this, &UMonsterHPBarWidget::OnMonsterChangedHPBar);
	}
	
	UpdateHPBar(BoundAttributeComponent->GetCurrentHP(), BoundAttributeComponent->GetMaxHP());
	SetVisibility(ESlateVisibility::Visible);
}

void UMonsterHPBarWidget::UnbindFromMonster()
{
	if (BoundAttributeComponent.IsValid())
	{
		BoundAttributeComponent->OnDeath.RemoveDynamic(this, &UMonsterHPBarWidget::OnMonsterDeath);
		BoundAttributeComponent->OnHPChanged.RemoveDynamic(this, &UMonsterHPBarWidget::OnMonsterChangedHPBar);
	}

	BoundMonster.Reset();
	BoundAttributeComponent.Reset();
}

void UMonsterHPBarWidget::UpdateHPBar(float CurrentHP, float MaxHP)
{
	if (!HPBar || !HPText) return;

	const float HPPercent = MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f;
	HPBar->SetPercent(HPPercent);

	const FString HPString = FString::Printf(TEXT("%.0f / %.0f"), CurrentHP, MaxHP);
	HPText->SetText(FText::FromString(HPString));
}

void UMonsterHPBarWidget::OnMonsterDeath()
{
	SetVisibility(ESlateVisibility::Hidden);
	UnbindFromMonster();
}

void UMonsterHPBarWidget::OnMonsterChangedHPBar(float CurrentHP, float MaxHP)
{
	UpdateHPBar(CurrentHP, MaxHP);
}
