// gas_aura_ll


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>火焰箭</>\n\n"

			// Level
			"<Small>等级: </><Level>%d</>\n"
			// ManaCost
			"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

			"<Default>发射一道火焰, "
			"撞击爆炸: </>"

			// Damage
			"<Damage>%d</><Default> 火焰伤害"
			" 概率附着燃烧</>"),

		                       // Values
		                       Level,
		                       ManaCost,
		                       Cooldown,
		                       ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>火焰箭</>\n\n"

			// Level
			"<Small>等级: </><Level>%d</>\n"
			// ManaCost
			"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

			// Number of FireBolts
			"<Default>发射 %d 道火焰, "
			"撞击爆炸: </>"

			// Damage
			"<Damage>%d</><Default> 火焰伤害"
			" 概率附着燃烧</>"),

		                       // Values
		                       Level,
		                       ManaCost,
		                       Cooldown,
		                       FMath::Min(Level, NumProjectiles),
		                       ScaledDamage);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		// Title
		"<Title>下一级: </>\n\n"

		// Level
		"<Small>等级: </><Level>%d</>\n"
		// ManaCost
		"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

		// Number of FireBolts
		"<Default>发射 %d 道火焰, "
		"撞击爆炸: </>"

		// Damage
		"<Damage>%d</><Default> 火焰伤害"
		" 概率附着燃烧</>"),

	                       // Values
	                       Level,
	                       ManaCost,
	                       Cooldown,
	                       FMath::Min(Level, NumProjectiles),
	                       ScaledDamage);
}

/**
 * 生成多个火球投射物，支持扇形散布和追踪目标
 * 
 * @param ProjectileTargetLocation  投射物的目标位置，用于确定发射方向和可能的追踪目标
 * @param SocketTag                 发射投射物的骨骼/插槽标签，用于获取起始位置
 * @param bOverridePitch            是否覆盖默认仰角
 * @param PitchOverride             覆盖的仰角值（当 bOverridePitch 为 true 时生效）
 * @param HomingTarget              追踪目标 Actor（若为 nullptr 则追踪地面目标位置）
 */
void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
                                     bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	// 仅在服务器端生成，保证投射物的网络权威性
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 从战斗接口获取插槽位置
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(), SocketTag);

	// 计算从插槽指向目标的方向
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	// 设置仰角（如需要）
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	const FVector Forward = Rotation.Vector();
	// 计算实际发射的投射物数量，取技能等级和配置数量的较小值
	const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());
	// 在扇面内均匀分布方向角度
	TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
		Forward, FVector::UpVector, ProjectileSpread, EffectiveNumProjectiles);

	// 遍历每个计算出的方向，生成对应的投射物
	for (const FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

		// 使用 Deferred 方式生成投射物，以便在生成前配置属性
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetAvatarActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// 为投射物填充伤害参数
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		
		// 设置追踪目标
		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			// 如果提供了有效的活体目标，追踪其根组件
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			// 没有活体目标时，创建一个临时场景组件并放置在目标位置，作为追踪点
			//ProjectileMovement->HomingTargetComponent是弱引用，ProjectileMovement销毁时，不会去销毁HomingTargetComponent
			//所以为了避免GC无法回收，在 Projectile 定义 HomingTargetSceneComponent
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
			// 随机化追踪加速度，增强多样性
    		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(
    			HomingAccelerationMin, HomingAccelerationMax);
    		// 根据技能配置决定是否启用追踪
    		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
    		
    		// 完成生成，触发 Construction Script 和 BeginPlay
    		Projectile->FinishSpawning(SpawnTransform);
	}
}
