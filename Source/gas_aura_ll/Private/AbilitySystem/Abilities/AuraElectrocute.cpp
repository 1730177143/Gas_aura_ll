// gas_aura_ll


#include "AbilitySystem/Abilities/AuraElectrocute.h"

FString UAuraElectrocute::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>闪电链</>\n\n"

			// Level
			"<Small>等级: </><Level>%d</>\n"
			// ManaCost
			"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

			"<Default>发射一道闪电, "
			"击中目标，反复造成 </>"

			// Damage
			"<Damage>%d</><Default> 雷击伤害并"
			" 有概率造成眩晕</>"),

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
			"<Title>闪电链</>\n\n"

			// Level
			"<Small>等级: </><Level>%d</>\n"
			// ManaCost
			"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>发射一道闪电, "
			"链接 %d 个附近的额外目标, 造成 </>"

			// Damage
			"<Damage>%d</><Default> 雷击伤害并"
			" 有概率造成眩晕</>"),

		                       // Values
		                       Level,
		                       ManaCost,
		                       Cooldown,
		                       FMath::Min(Level, MaxNumShockTargets - 1),
		                       ScaledDamage);
	}
}

FString UAuraElectrocute::GetNextLevelDescription(int32 Level)
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

		// Addition Number of Shock Targets
		"<Default>发射一道闪电, "
		"链接 %d 个附近的额外目标, 造成 </>"

		// Damage
		"<Damage>%d</><Default> 雷击伤害并"
		" 有概率造成眩晕</>"),

	                       // Values
	                       Level,
	                       ManaCost,
	                       Cooldown,
	                       FMath::Min(Level, MaxNumShockTargets - 1),
	                       ScaledDamage);
}
