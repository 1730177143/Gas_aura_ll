// gas_aura_ll


#include "AbilitySystem/Data/AbilityInfo.h"

#include "gas_aura_ll/AuraLogChannels.h"

FAuraAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FAuraAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogAura, Error, TEXT("无法在 AbilityInfo [%s]  找到 AbilityTag  [%s] 对应的 Info"), *GetNameSafe(this), *AbilityTag.ToString());
	}

	return FAuraAbilityInfo();
}
