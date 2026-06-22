// gas_aura_ll

#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	// 默认不自动激活，而是由标签事件驱动
	bAutoActivate = false;
}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	// 尝试立即获取 Owner 身上的 ASC
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		// ASC 已存在，直接注册标签事件
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved)
		   .AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	else
	{
		// ASC 可能尚未初始化（例如敌人延迟生成），通过 CombatInterface 的委托等待 ASC 注册完成
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
		{
			// 若 ASC 当前不可用（如敌人延迟生成），则通过 CombatInterface 的委托等待 ASC 注册完成
			// AddWeakLambda 通过弱引用跟踪 this，在对象销毁后自动移除回调，避免悬空崩溃。
			CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
			{
				// ASC 就绪后，再注册 DebuffTag 的标签事件，监听标签添加/移除以驱动特效开关
				InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved)
				     .AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
			});
		}
	}
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
	{
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UDebuffNiagaraComponent::OnOwnerDeath);
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 检查 Owner 是否有效且存活（死亡时不显示特效）
	const bool bOwnerValid = IsValid(GetOwner());
	const bool bOwnerAlive = GetOwner()->Implements<UCombatInterface>() &&
		!ICombatInterface::Execute_IsDead(GetOwner());

	// 当标签计数 > 0 且 Owner 存活时激活特效，否则关闭
	if (NewCount > 0 && bOwnerValid && bOwnerAlive)
	{
		Activate();
	}
	else
	{
		Deactivate();
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	Deactivate();
}
