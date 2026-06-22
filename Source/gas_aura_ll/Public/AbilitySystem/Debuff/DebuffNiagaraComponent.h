// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"

/**
 * 根据 GameplayTag 自动控制 Niagara 特效的开关
 * 常用于在角色身上存在特定 Debuff 标签时显示对应的特效（如着火、中毒等）
 */
UCLASS()
class GAS_AURA_LL_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UDebuffNiagaraComponent();

	// 触发该特效所需的 GameplayTag（例如 "Debuff.Burning"）
	UPROPERTY(VisibleAnywhere)
	FGameplayTag DebuffTag;

protected:
	virtual void BeginPlay() override;

	// 当 DebuffTag 被添加或移除时调用，通过 NewCount 判断特效的启用/禁用
	void DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor); //在角色死亡时的回调
};
