// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraBeamSpell.h"
#include "AuraElectrocute.generated.h"

/**
 * 
 */
UCLASS()
class GAS_AURA_LL_API UAuraElectrocute : public UAuraBeamSpell
{
	GENERATED_BODY()
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
