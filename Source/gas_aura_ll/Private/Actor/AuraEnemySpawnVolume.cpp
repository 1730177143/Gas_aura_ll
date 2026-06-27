// gas_aura_ll

#include "Actor/AuraEnemySpawnVolume.h"
#include "Actor/AuraEnemySpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Interaction/PlayerInterface.h"

/**
 * 敌人出生盒的构造函数
 * 创建一个 Box 组件作为根组件，并配置为仅与 Pawn 通道重叠，
 * 用于检测玩家进入从而触发敌人生成
 */
AAuraEnemySpawnVolume::AAuraEnemySpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 仅用于查询，不产生物理阻挡
	Box->SetCollisionObjectType(ECC_WorldStatic); // 对象类型设为静态世界
	Box->SetCollisionResponseToAllChannels(ECR_Ignore); // 忽略所有碰撞通道
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 仅对 Pawn 通道产生重叠事件
}

/**
 * 从存档加载后调用
 * 如果该出生盒已触发（bReached 为 true），则直接销毁自身，
 * 避免玩家重新进入区域时再次生成敌人
 */
void AAuraEnemySpawnVolume::LoadActor_Implementation()
{
	if (bReached)
	{
		Destroy();
	}
}

/**
 * 当玩家进入出生盒时触发
 * 标记 bReached 为 true，遍历所有生成点生成敌人，最后禁用碰撞
 */
void AAuraEnemySpawnVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	// 只处理实现了玩家接口的 Actor（即玩家角色）
	if (!OtherActor->Implements<UPlayerInterface>()) return;

	bReached = true; // 标记已触发，防止重复生成

	// 遍历所有关联的生成点，逐个生成敌人
	for (AAuraEnemySpawnPoint* Point : SpawnPoints)
	{
		if (IsValid(Point))
		{
			Point->SpawnEnemy();
		}
	}
	// 禁用碰撞盒，避免重复触发
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/**
 * 游戏开始或生成时调用
 * 动态绑定 Box 组件的重叠事件到 OnBoxOverlap
 */
void AAuraEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAuraEnemySpawnVolume::OnBoxOverlap);
}
