// gas_aura_ll


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(
		TEXT("<Default>%s, </><Level>%d</>"),
		L"能力描述",
		Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>等级描述 </><Level>%d</> \n<Default>造成更大的破坏。 </>"), Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default> %d 级解锁</>"), Level);
}
