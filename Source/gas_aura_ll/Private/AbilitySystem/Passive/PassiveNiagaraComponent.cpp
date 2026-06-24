// gas_aura_ll

#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	// 默认不自动激活，由被动技能状态驱动
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	// 尝试立即获取 Owner 身上的 AuraASC
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		// 绑定被动激活事件，当被动技能状态变化时调用 OnPassiveActivate
		AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
		// 检查技能是否已装备，若是则立即激活特效
		ActivateIfEquipped(AuraASC);
	}
	else
	{
		// 如果 ASC 尚未初始化，通过 CombatInterface 的委托延迟绑定
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
		{
			CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC)
			{
				// 再次获取 AuraASC 并绑定事件
				if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(
					UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
				{
					AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
					ActivateIfEquipped(AuraASC);
				}
			});
		}
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	// 仅当变化的技能标签与本组件的被动标签匹配时才响应
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if (bActivate && !IsActive())
		{
			// 被动技能激活且特效未激活 -> 显示特效
			Activate();
		}
		else
		{
			// 否则停用特效（可能是被替换或移除）
			Deactivate();
		}
	}
}

void UPassiveNiagaraComponent::ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC)
{
	// 确保初始技能已经授予完毕
	const bool bStartupAbilitiesGiven = AuraASC->bStartupAbilitiesGiven;
	if (bStartupAbilitiesGiven)
	{
		// 获取该被动技能当前的状态，若为“已装备”则直接激活特效
		if (AuraASC->GetStatusFromAbilityTag(PassiveSpellTag) == FAuraGameplayTags::Get().Abilities_Status_Equipped)
		{
			Activate();
		}
	}
}