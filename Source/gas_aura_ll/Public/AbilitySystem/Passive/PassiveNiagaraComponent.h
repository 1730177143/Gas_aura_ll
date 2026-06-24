// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "PassiveNiagaraComponent.generated.h"

class UAuraAbilitySystemComponent;

/**
 * 被动技能对应的 Niagara 特效组件
 * 根据被动技能的激活/停用状态自动显示或隐藏特效
 * 同时处理 ASC 延迟注册的情况
 */
UCLASS()
class GAS_AURA_LL_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UPassiveNiagaraComponent();

	// 该组件所代表的被动技能的 GameplayTag
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;

protected:
	virtual void BeginPlay() override;

	/**
	 * 响应被动技能激活/停用的回调
	 * @param AbilityTag  发生状态变化的技能标签
	 * @param bActivate   是否激活（true 激活，false 停用）
	 */
	void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);

	/**
	 * 检查对应被动技能是否已装备，若已装备则立即激活特效
	 * @param AuraASC  角色身上的 Aura 技能系统组件
	 */
	void ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC);
};