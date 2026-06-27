// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "Checkpoint/Checkpoint.h"
#include "MapEntrance.generated.h"

/**
 * 地图入口 Actor，继承自 ACheckpoint
 * 用于玩家从一个关卡传送到另一个关卡
 * 与普通检查点的区别：进入后直接加载目标地图，而非仅保存进度
 */
UCLASS()
class GAS_AURA_LL_API AMapEntrance : public ACheckpoint
{
	GENERATED_BODY()

public:
	AMapEntrance(const FObjectInitializer& ObjectInitializer);

	/* Highlight Interface */
	virtual void HighlightActor_Implementation() override;
	/* Highlight Interface */

	/* Save Interface */
	// 地图入口不需要从存档恢复状态，留空
	virtual void LoadActor_Implementation() override;
	/* end Save Interface */

	// 目标关卡的软引用（编辑器中选择要切换到的地图）
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	// 玩家进入目标关卡后的出生点标签
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;

protected:
	// 玩家进入触发器时：保存进度，然后加载目标关卡
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
								 const FHitResult& SweepResult) override;
};