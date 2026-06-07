// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class GAS_AURA_LL_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	//初始能力
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;
};
