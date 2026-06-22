#pragma once//避免重复包含

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"
class UGameplayEffect;

//传给 AuraDamageGameplayAbility
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams()
	{
	}

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();

	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	//getter 函数
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedHit; }
	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }
	float GetDebuffDamage() const { return DebuffDamage; }
	float GetDebuffDuration() const { return DebuffDuration; }
	float GetDebuffFrequency() const { return DebuffFrequency; }
	TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }
	FVector GetDeathImpulse() const { return DeathImpulse; }

	//setter 函数
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }
	void SetIsSuccessfulDebuff(bool bInIsDebuff) { bIsSuccessfulDebuff = bInIsDebuff; }
	void SetDebuffDamage(float InDamage) { DebuffDamage = InDamage; }
	void SetDebuffDuration(float InDuration) { DebuffDuration = InDuration; }
	void SetDebuffFrequency(float InFrequency) { DebuffFrequency = InFrequency; }
	void SetDamageType(TSharedPtr<FGameplayTag> InDamageType) { DamageType = InDamageType; }
	void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse; }

	/** Returns the actual struct used for serialization, subclasses must override this! */
	//返回用于序列化的实际结构，子类必须重写此！
	virtual UScriptStruct* GetScriptStruct() const
	{
		return StaticStruct();
	}

	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FAuraGameplayEffectContext* Duplicate() const
	{
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/** Custom serialization, subclasses must override this */
	//自定义序列化，子类必须重写此！
	//决定结构体如何序列化（保存、网络传输等）
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

protected:
	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsSuccessfulDebuff = false;

	UPROPERTY()
	float DebuffDamage = 0.f;

	UPROPERTY()
	float DebuffDuration = 0.f;

	UPROPERTY()
	float DebuffFrequency = 0.f;

	TSharedPtr<FGameplayTag> DamageType;
	
		UPROPERTY()
    	FVector DeathImpulse = FVector::ZeroVector;
};

/*
 * 为 FAuraGameplayEffectContext 提供类型特性（Type Traits）特化，告诉 UE 反射系统该结构体支持哪些高级操作。
 * 枚举值定义在 TStructOpsTypeTraitsBase2 的基类中，此处显式启用需要的特性。
 * 注意：如果不添加此特化，即使结构体中有 NetSerialize 函数，UE 网络系统也不会调用它。
 * 此特化必须与 FAuraGameplayEffectContext 定义在同一个头文件中，并且一般在结构体定义之后。
 */
template <>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		//指示该结构体拥有自定义的网络序列化函数 NetSerialize()
		//框架在复制此结构体时会调用 FAuraGameplayEffectContext::NetSerialize，而非使用默认的逐成员复制
		WithNetSerializer = true,
		// 确保结构体支持深拷贝操作，尤其重要因为基类中包含 TSharedPtr<FHitResult> 成员
		// 若不启用，复制 TSharedPtr 时可能导致引用计数错误或悬空指针；此标志让系统生成正确的拷贝构造函数和赋值运算符
		WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
