// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AuraAttributeSet.generated.h"

// 宏：为一组属性快速生成 Getter、Setter 和初始化函数，减少重复代码
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// 用于临时存储 GameplayEffect 执行时的源和目标信息，方便在 PostGameplayEffectExecute 中使用
USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties()
	{
	}

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
};

/**
 * 
 */
UCLASS()
class GAS_AURA_LL_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAuraAttributeSet();
	// 网络复制相关的生命周期设置
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**GAS
	属性分Base Value：持久化数值和Current Value：临时视图值
	Modifiers是装备或Buff等附加在角色身上，可能随时间变化、增减或消失的临时效果
	聚合器(Aggregator)负责属性值的实时计算，得出CurrentValue
	GE读：
	1. Modifier读取Current Value/Base Value
	2. Execution Calculation 中调用GetNumericAttribute()/GetNumericAttributeBase()读取Current Value/Base Value
	GE写入：
	根据GE类型决定是否修改Base Value。
		
		[GE 应用]
			 │
			 ├─ 如果是 Instant / Periodic GE
			 │       │
			 │       ├─ 1. 直接修改 Base Value
			 │       ├─ 2. 调用 PostGameplayEffectExecute
			 │       │      └─ 在此修改 Base Value → 钳制成功
			 │       └─ 3. 聚合器根据新 Base + Modifiers 计算 Current
			 │
			 └─ 如果是 Infinite / Duration GE
					 │
					 └─ 1. 向聚合器添加/移除 Modifier
						└─ 2. 聚合器根据 Base + 所有 Modifiers 重新计算 Current
		
		[属性的 Current Value 即将被修改时（由于 Base 变化或 Modifier 变化）]
			 │
			 ├─ 聚合器计算出新的临时值 = Base + 所有 Modifiers
			 ├─ 调用 PreAttributeChange(Attribute, 临时值)   // 可修改临时值
			 └─ 将修改后的临时值写入 Current Value
		
		[Current Value 写入完成后]//Base Value 的任何修改，都必然触发 Current Value 的重新计算写入
			 └─ 触发 GetGameplayAttributeValueChangeDelegate
	 */

	// Current Value 实际改变前调用，此处不应触发游戏逻辑或事件
	//进行数据钳制时，仅仅改变了查询属性修改器后的Current Value，但其他GE计算会使用未被限制的Base Value，导致数据钳制失败
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;


	// 只有在GameplayEffect修改Base Value后调用，可获取此次效果的几乎所有信息，用于应用后续逻辑
	//监听永久属性Base Value修改 (Instant / Periodic的Infinite 或 HasDuration GE)
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/*
	 * Primary Attributes
	 */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributes")
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Primary Attributes")
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Primary Attributes")
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);


	/*
	 * Vital Attributes
	 */
	// 核心属性，网络复制并在客户端变化时调用 OnRep_Health
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana)

	// 以下为网络复制时的回调，用于客户端接收属性变化后处理 UI 或效果
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;

	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

private:
	// 从 Data 中提取源和目标的信息填充到 Props，供 PostGameplayEffectExecute 使用
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;
};
