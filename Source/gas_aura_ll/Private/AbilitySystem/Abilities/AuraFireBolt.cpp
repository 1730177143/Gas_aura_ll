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


void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
                                     bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	// 仅在服务器端生成，保证投射物的网络权威性
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	if (GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
	{
		//限制产生火球的最大数量
		NumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());

		//根据可生成数量进行逻辑判断
		if (NumProjectiles > 1)
		{
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

			const float DeltaSpread = ProjectileSpread / NumProjectiles; //技能分的段数
			const FVector LeftOfSpread = Rotation.Vector().RotateAngleAxis(-ProjectileSpread / 2.f, FVector::UpVector);
			//获取到最左侧的角度

			for (int32 i = 0; i < NumProjectiles; i++)
			{
				const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * (i + 0.5f), FVector::UpVector);
				//获取当前分段的角度
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(SocketLocation);
				SpawnTransform.SetRotation(Direction.Rotation().Quaternion());

				//SpawnActorDeferred将异步创建实例，在实例创建完成时，相应的数据已经应用到了实例身上
				AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
					ProjectileClass,
					SpawnTransform,
					GetOwningActorFromActorInfo(),
					Cast<APawn>(GetAvatarActorFromActorInfo()),
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

				Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

				//确保变换设置被正确应用
				Projectile->FinishSpawning(SpawnTransform);

				UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation,
				                                     SocketLocation + Direction * 100.f, 5, FLinearColor::Green, 120,
				                                     5);
			}
		}
		else
		{
			SpawnProjectile(ProjectileTargetLocation, SocketTag, bOverridePitch, PitchOverride);
		}
	}
}
