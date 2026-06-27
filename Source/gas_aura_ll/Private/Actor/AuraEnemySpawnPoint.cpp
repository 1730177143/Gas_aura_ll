// gas_aura_ll

#include "Actor/AuraEnemySpawnPoint.h"
#include "Character/AuraEnemy.h"

/**
 * 在生成点生成一个敌人
 * 使用 Deferred 生成方式，在生成前设置敌人的等级和职业，
 * 然后完成生成并为其创建一个默认 AI 控制器
 */
void AAuraEnemySpawnPoint::SpawnEnemy()
{
	FActorSpawnParameters SpawnParams;
	// 设置碰撞处理：尝试调整位置以防止穿透，但无论如何都会生成
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// 延迟生成敌人，以便在 BeginPlay 前设置属性
	AAuraEnemy* Enemy = GetWorld()->SpawnActorDeferred<AAuraEnemy>(EnemyClass, GetActorTransform());
	Enemy->SetLevel(EnemyLevel); // 设置敌人等级
	Enemy->SetCharacterClass(CharacterClass); // 设置敌人职业/类型
	Enemy->FinishSpawning(GetActorTransform()); // 完成生成，触发 BeginPlay
	Enemy->SpawnDefaultController(); // 创建默认 AI 控制器，使敌人具备行为
}
