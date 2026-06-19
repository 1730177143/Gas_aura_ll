// gas_aura_ll


#include "AbilitySystem/Abilities/AuraFireBolt.h"

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
