// gas_aura_ll


#include "AbilitySystem/Abilities/AuraFireBlast.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		// Title
		"<Title>火焰爆破</>\n\n"

		// Level
		"<Small>等级: </><Level>%d</>\n"
		// ManaCost
		"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

		// Number of Fire Balls
		"<Default>向四面八方发射 %d </>"
		"<Default>颗火球，每颗火球会在返回时发生爆炸，并造成</>"

		// Damage
		"<Damage>%d</><Default> 点径向火焰伤害，"
		" 并有一定几率触发燃烧</>"),

	                       // Values
	                       Level,
	                       ManaCost,
	                       Cooldown,
	                       NumFireBalls,
	                       ScaledDamage);
}

FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
		// Title
		"<Title>下一级:</>\n\n"

		// Level
		"<Small>等级: </><Level>%d</>\n"
		// ManaCost
		"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
		// Cooldown
		"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

		// Number of Fire Balls
		"<Default>向四面八方发射 %d </>"
		"<Default>颗火球，每颗火球会在返回时发生爆炸，并造成</>"

		// Damage
		"<Damage>%d</><Default> 点径向火焰伤害，"
		" 并有一定几率触发燃烧</>"),

	                       // Values
	                       Level,
	                       ManaCost,
	                       Cooldown,
	                       NumFireBalls,
	                       ScaledDamage);
}

/**
 * 生成环绕角色周围的火球阵列
 * 
 * 根据技能等级对应的火球数量，在 360 度圆周上均匀分布生成火球，
 * 每个火球通过 Deferred 方式生成，以便在生成前配置伤害参数、返回目标和所有者。
 * 
 * @return 生成的所有 AAuraFireBall 指针数组
 */
TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> FireBalls;

	// 获取施法者的前方向量和当前位置，用于确定火球生成基准
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();

	// 在 360 度圆周上均匀分布方向，NumFireBalls 为火球数量
	// 返回的 Rotators 数量等于 NumFireBalls，每个绕 UpVector 均匀间隔
	TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
		Forward, FVector::UpVector, 360.f, NumFireBalls);

	// 为每个旋转方向生成一个火球
	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location); // 生成在施法者位置
		SpawnTransform.SetRotation(Rotator.Quaternion()); // 朝向计算出的方向

		// 延迟生成火球，以便在 BeginPlay 之前设置属性
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// 填充直接撞击伤害的参数
		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo(); // 火球可返回的目标
		FireBall->SetOwner(GetAvatarActorFromActorInfo()); // 设置所有者（施法者）

		// 填充爆炸伤害的参数（与直接伤害使用相同的默认值）
		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();

		FireBalls.Add(FireBall);

		// 完成生成，触发 BeginPlay
		FireBall->FinishSpawning(SpawnTransform);
	}

	return FireBalls;
}
