// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

// 效果应用策略：定义在重叠事件的哪个阶段施加效果
UENUM()
enum class EEffectApplicationPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DontApply,
};

// 效果移除策略：定义何时移除已施加的无限持续时间效果
UENUM()
enum class EEffectRemovePolicy
{
	RemoveOnEndOverlap,
	DontRemove,
};

// 用于关卡中的效果触发器Actor，通过碰撞重叠对目标施加 GameplayEffect
UCLASS()
class GAS_AURA_LL_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAuraEffectActor();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 计算后的Actor所在的位置
	UPROPERTY(BlueprintReadWrite)
	FVector CalculatedLocation;

	// 计算后的Actor的旋转
	UPROPERTY(BlueprintReadWrite)
	FRotator CalculatedRotation;

	// Actor是否帧更新旋转
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	bool bRotates = false;

	// Actor每秒旋转的角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	float RotationRate = 45.f;

	// Actor是否更新位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	bool bSinusoidalMovement = false;

	// 正弦值-1到1，此值为调整更新移动范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	float SineAmplitude = 1.f;

	// 此值参与正弦运算，默认值为1秒一个循环（2PI走完一个正弦的循环，乘以时间，就是一秒一个循环，可用于调整位置移动速度）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup Movement")
	float SinePeriodConstant = 1.f; //2 * PI

	// Actor生成的默认初始位置，在Actor动态浮动时，需要默认位置作为基础位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	FVector InitialLocation;

	//调用此函数，Actor开始自动更新上下位置
	UFUNCTION(BlueprintCallable)
	void StartSinusoidalMovement();

	//调用此函数，Actor开始自动旋转
	UFUNCTION(BlueprintCallable)
	void StartRotation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	bool bDestroyOnEffectApplication = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bApplyEffectsToEnemies = false;

	// 向目标Actor施加一个 GE 的核心函数
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	// 当其他Actor开始与这个触发器重叠时调用（由蓝图或C++触发）
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);

	// 当其他Actor结束与这个触发器重叠时调用
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

	// 配置变量：在蓝图中按需设置
	// 即时效果类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;

	// 即时效果的应用策略，默认不自动应用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DontApply;

	// 持续时间效果类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DontApply;

	// 无限持续时间效果类，需要手动移除
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DontApply;

	// 无限效果的移除策略，默认离开重叠时移除，以实现"离开区域即失效"
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	EEffectRemovePolicy InfiniteEffectRemovePolicy = EEffectRemovePolicy::RemoveOnEndOverlap;

	// 存储已施加的无限效果句柄及其对应的目标ASC
	// 用于后续精确移除（离开区域时移除效果）
	UPROPERTY()
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Applied Effects")
	float ActorLevel = 1.f;

private:
	//当前掉落物的存在时间，可以通过此时间实现动态效果
	float RunningTime = 0.f;

	// 每一帧更新Actor的位置和转向
	void ItemMovement(float DeltaTime);
};
