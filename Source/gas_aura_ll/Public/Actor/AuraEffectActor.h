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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	bool bDestroyOnEffectRemoval = false;

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
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	float ActorLevel = 1.f;

private:
};
