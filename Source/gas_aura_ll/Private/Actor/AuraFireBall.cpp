// gas_aura_ll

#include "Actor/AuraFireBall.h"
#include "GameplayCueManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"

void AAuraFireBall::BeginPlay()
{
	Super::BeginPlay();
	// 启动火球的发射动画时间线（由蓝图实现具体视觉效果）
	StartOutgoingTimeline();
}

void AAuraFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                    const FHitResult& SweepResult)
{
	// 检查目标合法性（避免自伤、友伤等）
	if (!IsValidOverlap(OtherActor)) return;

	// 伤害处理仅在服务器端执行
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			// 设置死亡冲量方向为火球自身的前向（击退效果）
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;

			// 填入目标 ASC 并应用伤害效果
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}

void AAuraFireBall::OnHit()
{
	if (GetOwner())
	{
		// 构建 GameplayCue 参数并触发非复制的爆发特效（由 GameplayCue 系统处理）
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
			GetOwner(), FAuraGameplayTags::Get().GameplayCue_FireBlast, CueParams);
	}

	// 停止并销毁飞行中的循环音效
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true; // 标记已命中，防止重复触发命中逻辑
}
