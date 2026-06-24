// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraPointCollection.generated.h"

/**
 * 用于生成一系列地面参照点的 Actor
 * 包含 11 个预定义的场景组件（Pt_0 ~ Pt_10），以 Pt_0 为根
 * 通过 GetGroundPoints 可获取经过地面贴合和旋转后的点位置
 * 常用于技能召唤、特效布置等需要多个定位点的场景
 */
UCLASS()
class GAS_AURA_LL_API AAuraPointCollection : public AActor
{
	GENERATED_BODY()

public:
	AAuraPointCollection();

	/**
	 * 获取指定数量的地面点（经过地形贴合和旋转调整）
	 * @param GroundLocation  地面参考位置（通常为光标或技能目标点）
	 * @param NumPoints       需要获取的点数量（1~11）
	 * @param YawOverride     可选的水平旋转覆盖角度（度），用于整体旋转点阵
	 * @return 经过调整的场景组件指针数组，最多包含 NumPoints 个元素
	 */
	UFUNCTION(BlueprintPure)
	TArray<USceneComponent*> GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride = 0.f);

protected:
	// 不可变点数组：存储所有预定义的点组件，在构造时填充，运行时不会改变元素
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<USceneComponent*> ImmutablePts;

	// 预定义的 11 个场景组件，在构造时创建并挂载
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_2;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_3;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_4;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_5;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_6;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_7;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_8;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_9;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_10;
};
