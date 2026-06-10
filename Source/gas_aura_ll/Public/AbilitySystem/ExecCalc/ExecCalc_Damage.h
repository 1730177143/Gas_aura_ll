// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Damage.generated.h"

/**
 * 
 */
UCLASS()
class GAS_AURA_LL_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_Damage();


	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	                                    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
