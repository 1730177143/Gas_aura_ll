// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GAS_AURA_LL_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();

	/** Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	/** end Combat Interface */

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Player Interface */
	virtual void AddToXP_Implementation(int32 InXP) override;
	/** end Player Interface */

private:
	virtual void InitAbilityActorInfo() override;
};
