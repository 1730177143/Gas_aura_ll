// gas_aura_ll


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "gas_aura_ll/AuraLogChannels.h"
#include "Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	//OnGameplayEffectAppliedDelegateToSelf 只在服务器端调用。
	//为确保客户端也能触发回调，EffectApplied 应该是 Client RPC 函数（Client RPC : 服务器端发起调用，客户端执行逻辑）
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			/*
			 * 变量 'DynamicAbilityTags' 已被弃用，原因: 
			 * '使用GetDynamicSpecSourceTags（），它更好地表示该变量的作用
			 * - 请在升级到下一个版本前将代码更新到新的 API，否则你的项目将无法再编译。'
			 */
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartupInputTag);
			// AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			// AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);

			GiveAbility(AbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
		// AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		AbilitySpecInputPressed(AbilitySpec);
		if (AbilitySpec.IsActive())
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			const UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();
			FPredictionKey OriginalPredictionKey = Instance
				                                       ? Instance->GetCurrentActivationInfo().
				                                                   GetActivationPredictionKey()
				                                       : AbilitySpec.ActivationInfo.GetActivationPredictionKey();

			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputPressed,
				AbilitySpec.Handle,
				OriginalPredictionKey
			);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		// if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			//AbilitySpecInputPressed 通知能力，能力对应的按键被按下
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	// 输入标签无效则直接返回
	if (!InputTag.IsValid()) return;

	// 遍历所有当前可激活的技能
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 检查技能的动态源标签中是否包含该输入标签，且技能处于激活状态
		// 注意：新版 GAS 推荐使用 GetDynamicSpecSourceTags() 替代直接访问 DynamicAbilityTags
		// 以确保正确获取与技能实例相关的动态标签（特别是实例化技能或预测中技能）
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// 通知技能输入已释放
			AbilitySpecInputReleased(AbilitySpec);
			// ================= 新版 API (UE 5.3+ 获取 PredictionKey) =================
			// 抑制弃用警告，因为我们需要兼容可能没有 PrimaryInstance 的边缘情况 (如 InstancedPerExecution)
			PRAGMA_DISABLE_DEPRECATION_WARNINGS

			// 1. 获取技能的主实例 (对于默认的 InstancedPerActor 策略，这会返回当前激活的实例)
			const UGameplayAbility* Instance = AbilitySpec.GetPrimaryInstance();

			// 2. 从实例的 CurrentActivationInfo 中获取 PredictionKey。
			// 如果实例为空，则回退到旧的 ActivationInfo (用于兼容极少数未实例化的边缘情况)
			FPredictionKey OriginalPredictionKey = Instance
				                                       ? Instance->GetCurrentActivationInfo().
				                                                   GetActivationPredictionKey()
				                                       : AbilitySpec.ActivationInfo.GetActivationPredictionKey();

			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			// ===============================================================

			// 将释放事件通过网络复制到其他客户端/服务器
			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputReleased,
				AbilitySpec.Handle,
				OriginalPredictionKey
			);
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	//锁定列表， 作用域结束后再执行 添加/删除 能力
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		//如果绑定了，就每个能力执行一次
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("delegate 执行失败， 位置 %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

//返回能力的 Status 标签
FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

//返回能力的 Input 标签
FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.GetDynamicSpecSourceTags().HasTagExact(Slot);
}

//检查 Ability 是否已经被 分配 Slot
bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

/**
 * 查找占用指定槽位（Slot）的 AbilitySpec
 * 
 * @param Slot 槽位标签，如 "InputTag.1"
 * @return 匹配槽位的 FGameplayAbilitySpec 指针，未找到则返回 nullptr
 */
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	// 锁定当前可激活技能列表，确保遍历期间列表不被修改
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// 使用 GetDynamicSpecSourceTags 检查是否包含指定槽位标签
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

/**
 * 判断给定的 AbilitySpec 是否为被动技能
 * 
 * @param Spec 技能规格引用
 * @return 如果技能类型标签为被动类型，则返回 true
 */
bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	// 获取技能信息数据资产
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	// 从 Spec 中提取技能标签
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	// 查找对应技能的信息
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	// 检查技能类型是否与全局被动标签精确匹配
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.GetDynamicSpecSourceTags().AddTag(Slot);
}

void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag,
	bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

//检查是否拥有指定 AbilityTag 的能力 ，没有返回空指针 nullptr
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	//锁定列表， 避免循环时 列表被修改
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}


// 客户端调用：尝试升级指定标签对应的属性
void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	// 检查当前控制的角色是否实现了玩家接口（确保是玩家操作）
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			// 通过可靠的服务器 RPC 进行属性升级，保证只有服务器能真正修改属性和点数
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

// 服务器 RPC 的实现：实际执行属性升级逻辑
void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	// 构建 GameplayEvent Payload
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	// 向自身发送 Gameplay 事件，由 GAS 寻找匹配的技能或效果来实际修改属性值
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	// 升级成功后，扣除一点属性点
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			//满足条件，可学习 的技能
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			//给予能力， 但不激活
			GiveAbility(AbilitySpec);
			//MarkAbilitySpecDirty 强制立即网络同步
			MarkAbilitySpecDirty(AbilitySpec);
			//发送委托到 widgetController 更新客户端ui
			ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void UAuraAbilitySystemComponent::ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	// 根据技能标签找到对应的 AbilitySpec
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		// 只有状态为“已装备”或“已解锁”的技能才允许进行装备操作
		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.
			Abilities_Status_Unlocked;
		if (bStatusValid)
		{
			if (!SlotIsEmpty(Slot)) // 目标槽位已有其他技能占用
			{
				// 查找占用该槽位的 AbilitySpec
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// 如果占位技能与要装备的技能完全相同，无需做任何更改，直接通知客户端即可
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
						return;
					}

					// 如果占位技能是被动技能，需要先在所有客户端上关闭被动效果
					if (IsPassiveAbility(*SpecWithSlot))
					{
						// 多播停用被动效果（参数 false 表示关闭）
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						// 广播被动技能被替换/停用的事件
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					// 清除该技能的槽位绑定，使其不再占用此槽
					ClearSlot(SpecWithSlot);
				}
			}
			// 如果要装备的技能还没有绑定任何槽位（即尚未激活）
			if (!AbilityHasAnySlot(*AbilitySpec))
			{
				// 对于被动技能，首次装备时需要激活其持续效果
				if (IsPassiveAbility(*AbilitySpec))
				{
					// 尝试激活技能（会触发其被动效果）
					TryActivateAbility(AbilitySpec->Handle);
					// 多播通知所有客户端激活该被动效果
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				// 移除技能身上的旧状态标签（如“已解锁”） 添加“已装备”状态标签
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			// 将技能绑定到新槽位
			AssignSlotToAbility(*AbilitySpec, Slot);
			// 强制触发网络同步到客户端
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		// 通知所有客户端该技能已装备，传递当前状态、新槽位和旧槽位
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag,
                                                                    const FGameplayTag& Status,
                                                                    const FGameplayTag& Slot,
                                                                    const FGameplayTag& PreviousSlot)
{
	// 广播技能装备事件，更新 UI
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}

		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			//确保 Status tag 唯一
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(
			GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
                                                              FString& OutNextLevelDescription)
{
	//已解锁技能
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		OutDescription = FString();
	}
	else
	{
		//未解锁， 但有效 的技能
		OutDescription = UAuraGameplayAbility::GetLockedDescription(
			AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;
}

//清除 AbilitySpec 的 Slot tag
void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
}

//清除所有 Ability 的指定 Slot tag
void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

//查询 AbilitySpec 是否拥有指定的 Slot tag
bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	for (FGameplayTag Tag : Spec->GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,
                                                                           const FGameplayTag& StatusTag,
                                                                           int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                     const FGameplayEffectSpec& EffectSpec,
                                                                     FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	//广播AllAssetTags到widgetController
	EffectAssetTags.Broadcast(TagContainer);
}
