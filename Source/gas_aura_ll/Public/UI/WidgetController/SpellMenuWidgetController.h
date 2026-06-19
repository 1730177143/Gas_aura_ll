// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class GAS_AURA_LL_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallBacksToDependencies() override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature SpellPointsChanged;


private:
	
	int32 CurrentSpellPoints = 0;
};
