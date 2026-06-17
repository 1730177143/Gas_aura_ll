// gas_aura_ll

#pragma once
#include "GameplayTagContainer.h"
#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownChange.generated.h"

class UAbilitySystemComponent;
struct FGameplayEffectSpec;

// 委托：冷却变化时广播剩余时间（秒）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCooldownChangeSignature, float, TimeRemaining);

/**
 * 异步任务节点，用于等待冷却标签的开始或结束
 * 在蓝图中作为异步节点使用，可以通过“CooldownStart”和“CooldownEnd”引脚驱动逻辑
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class GAS_AURA_LL_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// 当冷却开始时触发（即匹配的冷却标签被添加）
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;

	// 当冷却结束时触发（即匹配的冷却标签被移除）
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownEnd;

	/**
	 * 静态工厂函数，创建等待冷却变化的异步任务
	 * @param AbilitySystemComponent 要监听的 ASC
	 * @param InCooldownTag 要监听的冷却标签
	 * @return 异步任务对象指针，若参数无效则立即结束任务并返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent,
	                                                  const FGameplayTag& InCooldownTag);
	// 手动结束任务，解除绑定并销毁对象
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	// 监听的目标 ASC
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	// 要监听的冷却标签
	FGameplayTag CooldownTag;

	// 冷却标签变化回调：当标签被添加或移除时调用
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);

	// 活跃效果添加回调：当自身被施加新的 GameplayEffect 时调用，用于检测冷却开始
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied,
	                         FActiveGameplayEffectHandle ActiveEffectHandle);
};
