// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * 
 */
UCLASS()
class GAS_AURA_LL_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	/* 自定义计算类
	 *	1. 重写 CalculateBaseMagnitude_Implementation 
	 *	2. 创建 FGameplayEffectAttributeCaptureDefinition 结构体用于捕获属性
	 *	3. 在构造函数 UMMC_MaxHealth 里使用 AttributeToCapture 捕获属性
	 *		 使用 AttributeSource 设置属性来源
	 *		 使用 bSnapshot 设置快照
	 *	4. FGameplayEffectAttributeCaptureDefinition 结构体加入 TArray 待捕获属性数组
	 *	5. 在 CalculateBaseMagnitude_Implementation 中
	 *				使用 Spec.Captured(Source/Target)Tags.GetAggregatedTags 从源/目标获取tags
	 *			创建 FAggregatorEvaluateParameters 并设置 SourceTags 和 TargetTags
	 *			使用 GetCapturedAttributeMagnitude 将传入的 float 引用修改为捕获值
	 */

public:
	UMMC_MaxHealth();


	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition VigorDef;
};
