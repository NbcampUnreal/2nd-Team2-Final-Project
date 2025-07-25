#include "MonsterAttributeComponent.h"
#include "CR4S/MonsterAI/Data/MonsterAttributeRow.h"
#include "CR4S/MonsterAI/Data/MonsterDataSubsystem.h"
#include "MonsterAI/Components/MonsterStateComponent.h"

UMonsterAttributeComponent::UMonsterAttributeComponent()
	: MyHeader(TEXT("MonsterAttrComp"))
{
}

void UMonsterAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UMonsterAttributeComponent::InitializeMonsterAttribute(const FName MonsterID)
{
	UGameInstance* GI = GetOwner()->GetWorld()->GetGameInstance();
	if (auto* Subsys = GI ? GI->GetSubsystem<UMonsterDataSubsystem>() : nullptr)
	{
		if (const FMonsterAttributeRow* Row = Subsys->GetMonsterAttributeData(MonsterID))
		{
			CurrentAttribute = *Row;
			CurrentHP = CurrentAttribute.MaxHP;

			OnHPChanged.Broadcast(CurrentHP, CurrentAttribute.MaxHP);

			// NOTICE :: Test Log
			UE_LOG(LogTemp, Log, TEXT("[%s] %s Init: MaxHP=%.2f, AttackPower=%.2f, AttackRange=%.2f, AtkSpeed=%.2f, MoveSpeed=%.2f"),
				*MyHeader,
				*MonsterID.ToString(),
				CurrentAttribute.MaxHP,
				CurrentAttribute.AttackPower,
				CurrentAttribute.AttackRange,
				CurrentAttribute.AttackSpeed,
				CurrentAttribute.MoveSpeed
			);

			return;
		}
	}

	CurrentAttribute = FMonsterAttributeRow();
	CurrentHP = 0.0f;
}

void UMonsterAttributeComponent::ApplyDamage(float DamageAmount)
{
	ChangeHP(-FMath::Abs(DamageAmount));
}

void UMonsterAttributeComponent::ApplyHeal(float HealAmount)
{
	ChangeHP(FMath::Abs(HealAmount));
}

void UMonsterAttributeComponent::ChangeHP(float Delta)
{
	CurrentHP = FMath::Clamp(CurrentHP + Delta, 0.0f, CurrentAttribute.MaxHP);

	OnHPChanged.Broadcast(CurrentHP, CurrentAttribute.MaxHP);
	
	if (UMonsterStateComponent* StateComp = GetOwner()->FindComponentByClass<UMonsterStateComponent>())
	{
		StateComp->CheckPhaseTransition(CurrentHP, CurrentAttribute.MaxHP);
	}

	if (IsDead())
	{
		if (OnDeath.IsBound())
		{
			OnDeath.Broadcast();
		}
	}
}

bool UMonsterAttributeComponent::IsDead() const
{
	return CurrentHP <= 0.0f;
}

void UMonsterAttributeComponent::SetCurrentHP(const float NewHealth)
{
	const float Delta = NewHealth - CurrentHP;
	ChangeHP(Delta);
}


