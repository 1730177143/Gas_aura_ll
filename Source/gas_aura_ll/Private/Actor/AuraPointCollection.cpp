// gas_aura_ll

#include "Actor/AuraPointCollection.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

AAuraPointCollection::AAuraPointCollection()
{
	// 本 Actor 仅作为静态点集合，不需要每帧 Tick
	PrimaryActorTick.bCanEverTick = false;

	// 创建根点 Pt_0，并将其添加到不可变数组
	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	ImmutablePts.Add(Pt_0);
	SetRootComponent(Pt_0);

	// 依次创建 Pt_1 到 Pt_10，并挂载到根组件上
	Pt_1 = CreateDefaultSubobject<USceneComponent>("Pt_1");
	ImmutablePts.Add(Pt_1);
	Pt_1->SetupAttachment(GetRootComponent());

	Pt_2 = CreateDefaultSubobject<USceneComponent>("Pt_2");
	ImmutablePts.Add(Pt_2);
	Pt_2->SetupAttachment(GetRootComponent());

	Pt_3 = CreateDefaultSubobject<USceneComponent>("Pt_3");
	ImmutablePts.Add(Pt_3);
	Pt_3->SetupAttachment(GetRootComponent());

	Pt_4 = CreateDefaultSubobject<USceneComponent>("Pt_4");
	ImmutablePts.Add(Pt_4);
	Pt_4->SetupAttachment(GetRootComponent());

	Pt_5 = CreateDefaultSubobject<USceneComponent>("Pt_5");
	ImmutablePts.Add(Pt_5);
	Pt_5->SetupAttachment(GetRootComponent());

	Pt_6 = CreateDefaultSubobject<USceneComponent>("Pt_6");
	ImmutablePts.Add(Pt_6);
	Pt_6->SetupAttachment(GetRootComponent());

	Pt_7 = CreateDefaultSubobject<USceneComponent>("Pt_7");
	ImmutablePts.Add(Pt_7);
	Pt_7->SetupAttachment(GetRootComponent());

	Pt_8 = CreateDefaultSubobject<USceneComponent>("Pt_8");
	ImmutablePts.Add(Pt_8);
	Pt_8->SetupAttachment(GetRootComponent());

	Pt_9 = CreateDefaultSubobject<USceneComponent>("Pt_9");
	ImmutablePts.Add(Pt_9);
	Pt_9->SetupAttachment(GetRootComponent());

	Pt_10 = CreateDefaultSubobject<USceneComponent>("Pt_10");
	ImmutablePts.Add(Pt_10);
	Pt_10->SetupAttachment(GetRootComponent());
}
/**
 * 获取一系列经过地形贴合和旋转调整后的地面参照点
 * 
 * 主要流程：
 * 1. 根据 YawOverride 将所有非根点相对于根点（Pt_0）进行水平旋转
 * 2. 在每个点的位置垂直向下做射线检测，找到实际地面高度
 * 3. 将点移动到地面，并调整其朝向与地面法线对齐
 * 4. 收集指定数量的点返回（最多 NumPoints 个）
 * 
 * @param GroundLocation  地面参考位置（通常为光标或技能目标点）
 * @param NumPoints       需要获取的点数量（1 到 ImmutablePts 的大小）
 * @param YawOverride     额外水平旋转角度（度），用于整体旋转点阵方向
 * @return 经过调整的场景组件数组，数组长度 = min(NumPoints, ImmutablePts.Num())
 */
TArray<USceneComponent*> AAuraPointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints,
                                                               float YawOverride)
{
	// 确保请求的点数量不超过预定义的总点数
	checkf(ImmutablePts.Num() >= NumPoints, TEXT("尝试访问超出上界的 ImmutablePts."));

	TArray<USceneComponent*> ArrayCopy;

	// 遍历所有预定义的点组件（Pt_0 ~ Pt_10）
	for (USceneComponent* Pt : ImmutablePts)
	{
		// 已经收集到所需数量则提前退出
		if (ArrayCopy.Num() >= NumPoints) return ArrayCopy;

		if (Pt != Pt_0)  // 根点保持原位不动
		{
			// 计算相对于根点的方向向量
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();
			// 将相对方向绕世界 Z 轴（UpVector）旋转 YawOverride 度
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			// 更新该点的世界位置 = 根点位置 + 旋转后的相对方向
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}

		// 准备垂直射线检测：从当前点上方 500 单位到下方 500 单位
		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y,
		                                       Pt->GetComponentLocation().Z + 500.f);
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y,
		                                        Pt->GetComponentLocation().Z - 500.f);

		FHitResult HitResult;
		TArray<AActor*> IgnoreActors;
		// 获取周围 1500 单位内的存活玩家，添加到忽略列表
		// 避免射线击穿其他玩家导致点位错误
		UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f,
		                                                      GetActorLocation());

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);

		// 执行射线检测，使用 "BlockAll" 碰撞预设（与任何阻挡性物体碰撞）
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"),
		                                     QueryParams);

		// 将点的 Z 坐标调整到地面碰撞点，X、Y 保持不变
		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y,
		                                         HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);

		// 将点的朝向调整为垂直于地面（法线方向作为 Z 轴）
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		// 将调整后的点加入结果数组
		ArrayCopy.Add(Pt);
	}
	return ArrayCopy;
}