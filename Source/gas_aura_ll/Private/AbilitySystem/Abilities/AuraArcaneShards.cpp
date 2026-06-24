// gas_aura_ll


#include "AbilitySystem/Abilities/AuraArcaneShards.h"

FString UAuraArcaneShards::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>奥术碎片</>\n\n"

			// Level
			"<Small>等级: </><Level>%d</>\n"
			// ManaCost
			"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

			"<Default>召唤一块奥术能量碎片, "
			"在奥术能量原点造成</>"

			// Damage
			"<Damage>%d</><Default> 范围奥术伤害</>"),

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
			"<Title>奥术碎片</>\n\n"

			// Level
			"<Small>等级: </><Level>%d</>\n"
			// ManaCost
			"<Small>魔力消耗: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>冷却时间: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>召唤 %d 块奥术能量碎片, 在奥术能量原点造成 </>"

			// Damage
			"<Damage>%d</><Default> 范围奥术伤害</>"),

		                       // Values
		                       Level,
		                       ManaCost,
		                       Cooldown,
		                       FMath::Min(Level, MaxNumShards),
		                       ScaledDamage);
	}
}

FString UAuraArcaneShards::GetNextLevelDescription(int32 Level)
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

		// Addition Number of Shock Targets
		"<Default>召唤 %d 块奥术能量碎片, 在奥术能量原点造成 </>"

		// Damage
		"<Damage>%d</><Default> 范围奥术伤害</>"),

	                       // Values
	                       Level,
	                       ManaCost,
	                       Cooldown,
	                       FMath::Min(Level, MaxNumShards),
	                       ScaledDamage);
}
