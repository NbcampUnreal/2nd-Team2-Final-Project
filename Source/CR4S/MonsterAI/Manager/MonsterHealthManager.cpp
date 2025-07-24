#include "MonsterHealthManager.h"
#include "CR4S.h"
#include "EngineUtils.h"
#include "MonsterAI/BaseMonster.h"
#include "MonsterAI/Components/MonsterAttributeComponent.h"
#include "MonsterAI/UI/MonsterHPBarWidget.h"

void UMonsterHealthManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!HPBarWidgetClass)
	{
		if (!WidgetBlueprintPath.IsNull())
		{
			UClass* LoadedClass = WidgetBlueprintPath.TryLoadClass<UMonsterHPBarWidget>();
			if (LoadedClass)
			{
				HPBarWidgetClass = LoadedClass;
				UE_LOG(LogTemp, Warning, TEXT("[HealthManager] Loaded Blueprint widget: %s"), *WidgetBlueprintPath.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[HealthManager] Failed to load Blueprint widget: %s"), *WidgetBlueprintPath.ToString());
			}
		}
		
		// 블루프린트 로드 실패 시 C++ 클래스 사용
		if (!HPBarWidgetClass)
		{
			HPBarWidgetClass = UMonsterHPBarWidget::StaticClass();
			UE_LOG(LogTemp, Warning, TEXT("[HealthManager] Using default C++ widget class"));
		}
	}
	
	GetWorld()->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &UMonsterHealthManager::OnMonsterSpawned)
	);
}

void UMonsterHealthManager::Deinitialize()
{
	for (UMonsterHPBarWidget* Widget : HPBarWidgets)
	{
		if (IsValid(Widget))
		{
			Widget->RemoveFromParent();
		}
	}

	HPBarWidgets.Empty();
	RegisteredMonsters.Empty();
	
	Super::Deinitialize();
}

void UMonsterHealthManager::SetHPBarsVisible(bool bVisible)
{
	bHealthBarsVisible = bVisible;

	if (bVisible)
	{
		for (TActorIterator<ABaseMonster> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (ABaseMonster* Monster = *ActorItr; IsValid(Monster) && !Monster->IsDead())
			{
				RegisterMonster(Monster);
			}
		}
	}

	UpdateHealthBarVisibility();
}

void UMonsterHealthManager::RegisterMonster(ABaseMonster* Monster)
{
	if (!IsValid(Monster) || RegisteredMonsters.Contains(Monster))
		return;

	RegisteredMonsters.Add(Monster);
	
	if (UMonsterHPBarWidget* HealthBarWidget = CreateHealthBarWidget())
	{
		HealthBarWidget->BindToMonster(Monster);
		HPBarWidgets.Add(HealthBarWidget);
		
		if (UMonsterAttributeComponent* AttributeComp = Monster->GetMonsterAttribute())
		{
			AttributeComp->OnDeath.AddDynamic(this, &UMonsterHealthManager::OnMonsterDestroyed);
		}
	}

	UpdateHealthBarVisibility();
}

void UMonsterHealthManager::UnregisterMonster(ABaseMonster* Monster)
{
	if (!IsValid(Monster))
		return;

	const int32 MonsterIndex = RegisteredMonsters.IndexOfByKey(Monster);
	if (MonsterIndex != INDEX_NONE)
	{
		RegisteredMonsters.RemoveAt(MonsterIndex);

		if (HPBarWidgets.IsValidIndex(MonsterIndex))
		{
			if (UMonsterHPBarWidget* Widget = HPBarWidgets[MonsterIndex])
			{
				Widget->UnbindFromMonster();
				Widget->RemoveFromParent();
			}
			HPBarWidgets.RemoveAt(MonsterIndex);
		}
	}
}

void UMonsterHealthManager::RefreshAllHealthBars()
{
	for (int32 i = RegisteredMonsters.Num() - 1; i >= 0; --i)
	{
		if (!RegisteredMonsters[i].IsValid() || RegisteredMonsters[i]->IsDead())
		{
			if (HPBarWidgets.IsValidIndex(i) && IsValid(HPBarWidgets[i]))
			{
				HPBarWidgets[i]->RemoveFromParent();
			}
			RegisteredMonsters.RemoveAt(i);
			HPBarWidgets.RemoveAt(i);
		}
	}

	for (int32 i = 0; i < RegisteredMonsters.Num(); ++i)
	{
		if (RegisteredMonsters[i].IsValid() && HPBarWidgets.IsValidIndex(i))
		{
			ABaseMonster* Monster = RegisteredMonsters[i].Get();
			if (UMonsterAttributeComponent* AttributeComp = Monster->GetMonsterAttribute())
			{
				const FMonsterAttributeRow& Attribute = AttributeComp->GetMonsterAttribute();
				HPBarWidgets[i]->OnMonsterChangedHPBar(AttributeComp->GetCurrentHP(), Attribute.MaxHP);
			}
		}
	}
}

UMonsterHPBarWidget* UMonsterHealthManager::CreateHealthBarWidget()
{
	if (!HPBarWidgetClass)
		return nullptr;

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return CreateWidget<UMonsterHPBarWidget>(PC, HPBarWidgetClass);
		}
	}

	return nullptr;
}

void UMonsterHealthManager::UpdateHealthBarVisibility()
{
	const ESlateVisibility Visibility = bHealthBarsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	for (UMonsterHPBarWidget* Widget : HPBarWidgets)
	{
		if (IsValid(Widget))
		{
			Widget->SetVisibility(Visibility);
			if (bHealthBarsVisible && !Widget->IsInViewport())
			{
				Widget->AddToViewport();
			}
			else if (!bHealthBarsVisible && Widget->IsInViewport())
			{
				Widget->RemoveFromParent();
			}
		}
	}
}

void UMonsterHealthManager::OnMonsterSpawned(AActor* SpawnedMonster)
{
	if (ABaseMonster* Monster = Cast<ABaseMonster>(SpawnedMonster))
	{
		if (bHealthBarsVisible)
		{
			RegisterMonster(Monster);
		}
	}
}

void UMonsterHealthManager::OnMonsterDestroyed()
{
	RefreshAllHealthBars();
}
