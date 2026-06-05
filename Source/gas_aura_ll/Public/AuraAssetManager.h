// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AuraAssetManager.generated.h"

/**
 * AssetManager 是项目中全局的单例对象
 */
UCLASS()
class GAS_AURA_LL_API UAuraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UAuraAssetManager& Get();
protected:
	//引擎早期就会被调用
	virtual void StartInitialLoading() override;
};
