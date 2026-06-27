// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "gas_aura_ll/gas_aura_ll.h"
#include "Interaction/HighlightInterface.h"
#include "Interaction/SaveInterface.h"
#include "Checkpoint.generated.h"

class USphereComponent;

/**
 * 检查点 Actor，同时作为玩家出生点和存档触发点
 * 继承 APlayerStart   → 可作为玩家出生位置
 * 实现 ISaveInterface  → 存档系统会保存/加载 bReached 状态
 * 实现 IHighlightInterface → 鼠标悬停时高亮显示
 */
UCLASS()
class GAS_AURA_LL_API ACheckpoint : public APlayerStart, public ISaveInterface, public IHighlightInterface
{
	GENERATED_BODY()

public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

	/* Save Interface */
	// 检查点本身不需要加载 Transform，位置由场景静态决定
	virtual bool ShouldLoadTransform_Implementation() override { return false; };
	// 从存档加载后，若 bReached 为 true，则恢复激活时的视觉效果
	virtual void LoadActor_Implementation() override;
	/* end Save Interface */

	// 是否已激活（被玩家触碰过），标记为 SaveGame 会在存档中保存
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bReached = false;

	// 是否在 BeginPlay 时自动绑定碰撞事件，默认开启；可在蓝图中关闭以自定义行为
	UPROPERTY(EditAnywhere)
	bool bBindOverlapCallback = true;

protected:
	// 当玩家进入球形触发器时调用，激活检查点并保存游戏进度
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                             const FHitResult& SweepResult);

	virtual void BeginPlay() override;

	/* Highlight Interface */
	// 返回高亮时的“移动到此”目标位置
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	// 开启高亮渲染（仅当检查点未激活时）
	virtual void HighlightActor_Implementation() override;
	// 关闭高亮渲染
	virtual void UnHighlightActor_Implementation() override;
	/* Highlight Interface */

	// 当选中高亮时，角色会移动到的目标位置组件（可以放在检查点附近，避免角色与模型重叠）
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MoveToComponent;

	// 自定义深度模板值，默认使用项目中定义的 TAN 色（用于高亮轮廓）
	UPROPERTY(EditDefaultsOnly)
	int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;

	/**
	 * 检查点激活后调用，由蓝图实现视觉/音效反馈
	 * @param DynamicMaterialInstance 动态材质实例，用于动画或颜色变化
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	// 禁用碰撞并应用激活视觉效果的统一入口
	void HandleGlowEffects();

	// 检查点的静态网格体（主要视觉模型）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	// 球形触发器，检测玩家进入
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
