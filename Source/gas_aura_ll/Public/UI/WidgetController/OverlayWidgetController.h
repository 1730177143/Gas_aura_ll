// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"


struct FOnAttributeChangeData;
// 声明两个动态多播委托，分别用于广播当前生命值和最大生命值的变化
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, NewMaxHealth);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class GAS_AURA_LL_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	// 初始化时广播所有属性的当前值给 View
	virtual void BroadcastInitialValues() override;
	// 绑定 Model 侧属性变化的回调，建立 M -> C 的连接
	virtual void BindCallBackToDependencies() override;
	
	// 以下两个委托用于 C -> V 的通信，在蓝图中绑定以更新 UI
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;
	
protected:
	// 属性变化回调：从 Model 收到新值后，转发给 View
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
};
