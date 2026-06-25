// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectile.h"
#include "AuraFireBall.generated.h"

/**
 * 火球投射物 (AuraFireBall)
 * 继承自 AuraProjectile，实现了火球独特的发射动画、重叠伤害和命中爆发效果
 */
UCLASS()
class GAS_AURA_LL_API AAuraFireBall : public AAuraProjectile
{
	GENERATED_BODY()

public:
	/**
	 * 蓝图可实现事件，用于启动火球的发射动画时间线
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();

	// 返回目标 Actor（用于火球返回或追踪的引用）
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> ReturnToActor;

	// 爆炸伤害参数，用于在命中时可能触发的范围伤害效果
	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParams ExplosionDamageParams;

protected:
	virtual void BeginPlay() override;

	/**
	 * 火球与其他 Actor 发生重叠时的处理
	 * 在服务器端施加伤害效果，并设置死亡冲量方向为火球当前的前向
	 */
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                             const FHitResult& SweepResult) override;

	/**
	 * 火球命中后的表现处理
	 * 触发一个非复制的 GameplayCue（爆发特效），停止飞行音效并标记 bHit
	 */
	virtual void OnHit() override;
};
