// gas_aura_ll


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AuraAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	// 获取 Model 层（AttributeSet）数据，并将初始值广播给 View
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
}

void UOverlayWidgetController::BindCallBackToDependencies()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	// M -> C：绑定 ASC 的属性变化委托，当 Health 改变时调用 HealthChanged
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute()).AddUObject(
		this, &UOverlayWidgetController::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute()).
	                        AddUObject(
		                        this, &UOverlayWidgetController::MaxHealthChanged);
}

void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
	// C -> V：将来自 Model 的新值通过动态多播委托广播出去，View 会响应更新
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}
