// gas_aura_ll


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"
FAuraGameplayTags FAuraGameplayTags::GameplayTags;

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	// GameplayTagsManager 是项目中全局的对象
	//初始化并注册 GameplayTag
	UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attributes.Primary.Strength"),
	                                                 FString("增加物理伤害"));
}
