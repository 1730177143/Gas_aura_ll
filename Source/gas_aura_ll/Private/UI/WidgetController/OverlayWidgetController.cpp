// gas_aura_ll


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	// 获取 Model 层（AttributeSet）数据，并将初始值广播给 View

	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallBacksToDependencies()
{
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);

	// M -> C：绑定 ASC 的属性变化委托，当 Health 改变时调用 HealthChanged
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			//属性变化回调：从 Model 收到新值后，转发给 View
			// C -> V：将来自 Model 的新值通过动态多播委托广播出去，View 会响应更新
			OnHealthChanged.Broadcast(Data.NewValue);
		});

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxHealthAttribute()).
	                        AddLambda(
		                        [this](const FOnAttributeChangeData& Data)
		                        {
			                        OnMaxHealthChanged.Broadcast(Data.NewValue);
		                        }
	                        );

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetAuraAS()->GetMaxManaAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		}
	);
	if (GetAuraASC())
	{
		GetAuraASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);

		if (GetAuraASC()->bStartupAbilitiesGiven)
		{
			//直接调用
			BroadcastAbilityInfo();
		}
		else
		{
			//绑定委托再赋予初始能力后自动调用
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}

		GetAuraASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& TagContainer)
			{
				for (const FGameplayTag& Tag : TagContainer)
				{
					//"A.1".MatchesTag("A") will return True, "A".MatchesTag("A.1") will return False 
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (Tag.MatchesTag(MessageTag))
					{
						//捕获this来访问成员函数
						const FUIWidgetRow* Row = GetDataTableRowByTags<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("无法找到 LevelUpInfo.请填充 AuraPlayerState 蓝图"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
                                                 const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	// 构造一个代表“空槽位”的技能信息，用于清空 PreviousSlot 的 UI 显示
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked; // 状态标记为未锁定（无技能）
	LastSlotInfo.InputTag = PreviousSlot; // 指向旧槽位
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None; // 无技能

	// 第一次广播 AbilityInfoDelegate：通知 Overlay UI 清空旧槽位上的技能图标
	// （仅当 PreviousSlot 是一个有效槽位且该技能之前已装备时才有实际意义）
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 从数据资产中查找新装备技能对应的显示信息
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status; // 更新为实际状态（已装备）
	Info.InputTag = Slot; // 指向新槽位

	// 第二次广播 AbilityInfoDelegate：通知 Overlay UI 在新槽位上显示该技能的图标和信息
	// 注意：此处的 AbilityInfoDelegate 是 UOverlayWidgetController 自己的委托实例，
	// 与 SpellMenuWidgetController 中的同名委托互相独立，仅会驱动 HUD 技能栏的刷新
	AbilityInfoDelegate.Broadcast(Info);
}
