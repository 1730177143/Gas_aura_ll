// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
/**
 * AuraGameplayTags
 * 单例，包含原生Gameplay Tags
 */
struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	//_ 替代了路径的.
	FGameplayTag Attributes_Primary_Strength;
protected:

private:
	static FAuraGameplayTags GameplayTags;
};
