// gas_aura_ll


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

/**
 * 每帧更新，用于驱动物品的浮动和旋转动画
 * @param DeltaTime 上一帧到当前帧的时间间隔
 */
void AAuraEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 累加运行时间，用于正弦波计算
	RunningTime += DeltaTime;
	// 根据周期常量计算正弦波的周期时长，当运行时间超过一个周期时重置
	const float SinePeriod = 2 * PI / SinePeriodConstant;
	if (RunningTime > SinePeriod)
	{
		RunningTime = 0.f;
	}
	// 执行实际的运动计算（位置、旋转）
	ItemMovement(DeltaTime);
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();

	// 记录初始位置和旋转，用于后续动画计算
	InitialLocation = GetActorLocation();
	CalculatedLocation = InitialLocation;
	CalculatedRotation = GetActorRotation();
}

/**
 * 启用正弦波上下浮动
 * 调用后 Actor 会沿 Z 轴做正弦波运动
 */
void AAuraEffectActor::StartSinusoidalMovement()
{
	bSinusoidalMovement = true;
	InitialLocation = GetActorLocation();
	CalculatedLocation = InitialLocation;
}

/**
 * 启用持续旋转
 * 调用后 Actor 会绕 Z 轴自动旋转
 */
void AAuraEffectActor::StartRotation()
{
	bRotates = true;
	CalculatedRotation = GetActorRotation();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	// 尝试从目标Actor身上获取其技能系统组件（ASC）
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (TargetASC == nullptr) return;

	check(GameplayEffectClass);

	// 创建一个效果上下文句柄，用于携带与本次效果应用相关的额外信息（如来源、原因等）
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	// 将当前Actor设置为效果的来源对象，方便后续溯源
	EffectContextHandle.AddSourceObject(this);

	// 根据效果类、等级和上下文，生成一个待应用的效果规格句柄
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(
		GameplayEffectClass, ActorLevel, EffectContextHandle);

	// 将规格应用到目标自身的ASC上，返回一个活跃效果句柄用于后续操作（如移除）
	const FActiveGameplayEffectHandle ActiveGameplayEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(
		*EffectSpecHandle.Data.Get());
	//应用了无限时间的GE的Actor可以用Map存储actor和GE进行保存
	//应用了无限时间的GE的Actor可以用运行时检查给定效果持续时间策略来获取
	// 判断该效果是否为无限持续时间类型（永久效果）
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy ==
		EGameplayEffectDurationType::Infinite;

	// 如果施加的是无限效果，且移除策略为“离开重叠时移除”，则保存句柄以便后续清理
	// 这样设计是因为无限效果不会自动过期，必须手动移除，存储句柄可实现区域离开时自动消失
	if (bIsInfinite && InfiniteEffectRemovePolicy == EEffectRemovePolicy::RemoveOnEndOverlap)
	{
		ActiveEffectHandles.Add(ActiveGameplayEffectHandle, TargetASC);
	}
	//非无限 且设置了应用后销毁 ，进行销毁
	if (bDestroyOnEffectApplication && !bIsInfinite)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	// 根据配置的策略，在进入重叠时施加对应类型的效果
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	// 如果配置为“离开重叠时应用”，则此时施加效果
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
	}
	// 处理需要移除的无限效果：只有当策略为“离开重叠时移除”时才执行
	if (InfiniteEffectRemovePolicy == EEffectRemovePolicy::RemoveOnEndOverlap)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!IsValid(TargetASC)) return;

		// 收集所有需要移除的效果句柄（属于该目标ASC的）
		TArray<FActiveGameplayEffectHandle> HandlesToRemove;
		for (auto HandlePair : ActiveEffectHandles)
		{
			if (TargetASC == HandlePair.Value)
			{
				// 移除效果（参数1表示移除1层堆叠，如果有多层则只减一层）
				TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
				HandlesToRemove.Add(HandlePair.Key);
			}
		}
		// 从记录中删除已移除的句柄，防止重复移除或引用无效句柄
		for (auto& Handle : HandlesToRemove)
		{
			ActiveEffectHandles.FindAndRemoveChecked(Handle);
		}
	}
}

/**
 * 根据当前设置更新 Actor 的位置和旋转
 * 由 Tick 调用，实现正弦浮动和持续旋转效果
 * @param DeltaTime 帧间隔时间，用于计算增量旋转角度
 */
void AAuraEffectActor::ItemMovement(float DeltaTime)
{
	// 若启用旋转，则绕 Z 轴（偏航）增加 RotationRate * DeltaTime 度
	if (bRotates)
	{
		const FRotator DeltaRotation(0.f, DeltaTime * RotationRate, 0.f);
		CalculatedRotation = UKismetMathLibrary::ComposeRotators(CalculatedRotation, DeltaRotation);
	}

	// 若启用正弦浮动，则在初始位置基础上沿 Z 轴做正弦波偏移
	if (bSinusoidalMovement)
	{
		const float Sine = SineAmplitude * FMath::Sin(RunningTime * SinePeriodConstant);
		CalculatedLocation = InitialLocation + FVector(0.f, 0.f, Sine);
	}
}
