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

//
/* 创建 TBaseStaticDelegateInstance 委托，指定函数类型，内存/复制策略，默认 FDefaultDelegateUserPolicy，
 *
 * using 类似 typedef，但可以带模板参数 ,为任意函数签名 指定一个别名 
 * 定义 TStaticFuncPtr<T>，用来指代一种函数指针类型
 * TBaseStaticDelegateInstance 包装一个普通的 C 风格静态函数（或非成员函数），使其能够作为 UE 的委托（Delegate）来使用
 * FFuncPtr，它就是那个静态函数的原始指针类型
 * 
 * 定义了一个简短的模板别名 TStaticFuncPtr<T>，它实际代表 UE 静态委托内部所使用的原始函数指针类型。
 * 它被用来声明一个 TMap，实现从 FGameplayTag 到属性获取函数的映射，从而可以动态地通过 Tag 查找 Attribute，
 */
template <class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

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

	//返回给 AttributeMenuWidgetController  ，将 FGameplayTag 和返回 Attribute 的静态函数指针对应
	//TStaticFuncPtr 是一个模板，指定类型FGameplayAttribute()
	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;

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
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	/*
	 * 创建属性的样板过程
	 * 1. 声明 FGameplayAttributeData 变量，添加 UPROPERTY 和 ATTRIBUTE_ACCESSORS 宏
	 * 2. 创建对应的RepNotify函数 OnRep_属性 
	 * 3. 在RepNotify函数里使用 GAMEPLAYATTRIBUTE_REPNOTIFY 宏
	 * 4. 在 GetLifetimeReplicatedProps 函数中使用 DOREPLIFETIME_CONDITION_NOTIFY 宏 设置 复制条件 和 何时网络同步
	 */

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
	 * Secondary Attributes
	 */

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Secondary Attributes")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BlockChance, Category = "Secondary Attributes")
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitChance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitDamage, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitResistance, Category = "Secondary Attributes")
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ManaRegeneration, Category = "Secondary Attributes")
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxMana, Category="Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana)
	/*
	 * Vital Attributes
	 */
	// 核心属性，网络复制并在客户端变化时调用 OnRep_Health
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Mana, Category="Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana)

	/*
	 * Resistance Attributes
	 */

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_FireResistance, Category = "Resistance Attributes")
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, FireResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_LightningResistance, Category = "Resistance Attributes")
	FGameplayAttributeData LightningResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, LightningResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArcaneResistance, Category = "Resistance Attributes")
	FGameplayAttributeData ArcaneResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArcaneResistance);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_PhysicalResistance, Category = "Resistance Attributes")
	FGameplayAttributeData PhysicalResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, PhysicalResistance);

	/*
	 * Meta Attributes
	 * 只是个临时的占位符，我们只在服务端用它做计算
	 */

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage);

	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
	FGameplayAttributeData IncomingXP;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingXP);

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

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;

	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const;

	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const;

	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const;

	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const;

	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const;

	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const;

	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const;

	UFUNCTION()
	void OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const;

	UFUNCTION()
	void OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const;

	UFUNCTION()
	void OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const;

	UFUNCTION()
	void OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const;

private:
	// 从 Data 中提取源和目标的信息填充到 Props，供 PostGameplayEffectExecute 使用
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;

	void HandleIncomingDamage(const FEffectProperties& Props);

	void HandleIncomingXP(const FEffectProperties& Props);

	void Debuff(const FEffectProperties& Props);
	
	void ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit) const;

	void SendXPEvent(const FEffectProperties& Props);
	
	bool bTopOffHealth = false;
	bool bTopOffMana = false;
};
