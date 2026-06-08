// gas_aura_ll


#include "AuraAssetManager.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemGlobals.h"


UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	//在早期初始化 Gameplay Tags 
	FAuraGameplayTags::InitializeNativeGameplayTags();
	
	// 这是使用 ASC 的 Target Data的必备条件！
	UAbilitySystemGlobals::Get().InitGlobalData();
}
