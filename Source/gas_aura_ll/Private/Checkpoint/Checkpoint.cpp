// gas_aura_ll

#include "Checkpoint/Checkpoint.h"

#include "Components/SphereComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"

class AAuraGameModeBase;

// 继承 APlayerStart 必须使用此构造函数签名
ACheckpoint::ACheckpoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建检查点静态模型，并设置为阻挡所有通道的碰撞
	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	CheckpointMesh->SetupAttachment(GetRootComponent());
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 设置自定义深度模板值，用于高亮显示
	CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride);
	// 通知引擎重新创建该组件的渲染状态
	CheckpointMesh->MarkRenderStateDirty();

	// 创建球形触发器，仅与 Pawn 通道重叠
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(CheckpointMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 创建“移动到此”位置组件，默认挂载在根组件下
	MoveToComponent = CreateDefaultSubobject<USceneComponent>("MoveToComponent");
	MoveToComponent->SetupAttachment(GetRootComponent());
}

// 从存档加载后调用，若之前已激活则恢复激活视觉效果
void ACheckpoint::LoadActor_Implementation()
{
	if (bReached)
	{
		HandleGlowEffects();
	}
}

// 玩家进入触发器时的处理：激活检查点、保存世界状态、保存玩家进度、播放视觉效果
void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UPlayerInterface>())
	{
		bReached = true;
		// 通过 GameMode 保存当前世界所有 Actor 的存档数据
		if (AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			const UWorld* World = GetWorld();
			FString MapName = World->GetMapName();
			MapName.RemoveFromStart(World->StreamingLevelsPrefix); // 移除 PIE 等前缀，得到干净的地图名

			AuraGM->SaveWorldState(GetWorld(), MapName);
		}
		// 调用 PlayerInterface 保存玩家专属进度（等级、经验、技能等）
		IPlayerInterface::Execute_SaveProgress(OtherActor, PlayerStartTag);
		// 触发激活视觉效果
		HandleGlowEffects();
	}
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();
	// 如果允许绑定重叠回调，则动态绑定 OnSphereOverlap
	if (bBindOverlapCallback)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnSphereOverlap);
	}
}

// 实现高亮接口：返回“移动到此”目标位置（由 MoveToComponent 决定）
void ACheckpoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	OutDestination = MoveToComponent->GetComponentLocation();
}

// 高亮显示接口：未激活时启用自定义深度渲染，显示轮廓
void ACheckpoint::HighlightActor_Implementation()
{
	if (!bReached)
	{
		CheckpointMesh->SetRenderCustomDepth(true);
	}
}

// 取消高亮显示接口：关闭自定义深度渲染
void ACheckpoint::UnHighlightActor_Implementation()
{
	CheckpointMesh->SetRenderCustomDepth(false);
}

// 激活后视觉效果：禁用碰撞，创建动态材质实例并调用蓝图事件
void ACheckpoint::HandleGlowEffects()
{
	// 激活后不再需要碰撞，避免重复触发
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 创建动态材质实例，允许蓝图修改参数（如发光、颜色变化）
	UMaterialInstanceDynamic* DynamicMaterialInstace = UMaterialInstanceDynamic::Create(
		CheckpointMesh->GetMaterial(0), this);
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstace);
	// 调用蓝图实现事件，传递动态材质实例，用于自定义激活动画
	CheckpointReached(DynamicMaterialInstace);
}
