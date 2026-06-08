// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FVector&,
                                            DataHandle);

/**
 * 用于在技能激活时获取鼠标指针下的目标数据
 */
UCLASS()
class GAS_AURA_LL_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:
	// meta 使 OwningAbility 默认是 self
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks",
		meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility",
			BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

private:
	virtual void Activate() override;
	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
