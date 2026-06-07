// gas_aura_ll



#include " Input/AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
                                                                   bool bLogNotFound) const
{
	for (const FAuraInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("找不到 InputTag [%s] 的 AbilityInputAction , on InputConfig [%s]"),
		       *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
