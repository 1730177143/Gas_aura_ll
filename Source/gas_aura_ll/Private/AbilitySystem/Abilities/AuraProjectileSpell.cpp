// gas_aura_ll


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

/**
 * 在服务器上生成一个投射物，并设置其伤害参数
 * 
 * @param ProjectileTargetLocation 投射物目标位置，用于确定飞行方向
 * @param SocketTag 生成投射物的骨骼插槽标签，用于获取起始位置
 * @param bOverridePitch 是否覆盖默认俯仰角
 * @param PitchOverride 覆盖的俯仰角度值（当 bOverridePitch 为 true 时生效）
 */
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
                                           bool bOverridePitch, float PitchOverride)
{
	// 仅在服务器端生成，保证投射物的网络权威性
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// 从战斗接口获取插槽位置
	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(), SocketTag);

	// 计算从插槽指向目标的方向
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	//设置仰角
	if (bOverridePitch)
	{
		Rotation.Pitch = PitchOverride;
	}

	// 构造生成变换
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	// 延迟生成投射物，允许在生成前配置属性
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	// 从技能类默认值构建伤害参数，并传递给投射物，后续由投射物继续添加参数
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

	// 完成生成，触发 Construction Script 和 BeginPlay
	Projectile->FinishSpawning(SpawnTransform);
}
