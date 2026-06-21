#include "AuraAbilityTypes.h"

/**
 * NetSerialize，UE 网络序列化接口，用于将对象数据打包/解包以在网络间传输和存读档
 * @param Ar 存储序列化后的数据，能执行序列化和反序列化，重载了 << (双向工作)
 * @param Map 把对象映射为索引
 * @param bOutSuccess 序列化成功返回true
 * @return bool，通常表示序列化是否成功，
 */
bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	//!!!!!!!!!!!!!!!! Warning !!!!!!!!!!!!!!!
	// Any changes to this function also need to be done to FGameplayEffectContextNetSerializer to support Iris replication
	//!!!!!!!!!!!!!!!! Warning !!!!!!!!!!!!!!!

	// RepBits：位掩码，用于标记哪些成员变量需要被序列化（节省带宽，只发送有效数据）
	uint16 RepBits = 0;
	//通过 IsSaving() / IsLoading() 判断是写入还是读取
	/*
	* Ar.IsLoading() 返回 true
	* 表示当前正在执行反序列化（读取/加载）操作。数据从二进制流（网络包或存档文件）中读出，并填充到内存对象中。通常发生在接收端（客户端） 或 加载存档时。
	*
	* Ar.IsSaving() 返回 true
	* 表示当前正在执行序列化（写入/保存）操作。数据从内存对象写入二进制流，用于发送或存储。通常发生在发送端（服务器） 或 保存存档时。
	 */
	if (Ar.IsSaving())
	{
		// 如果允许复制 Instigator 且 Instigator 有效，则设置第 0 位
		if (bReplicateInstigator && Instigator.IsValid())
		{
			//0000 0000 和 0000 0001 或运算
			RepBits |= 1 << 0;
		}
		// 如果允许复制 EffectCauser 且有效，设置第 1 位
		if (bReplicateEffectCauser && EffectCauser.IsValid())
		{
			// 和 0000 0010 或运算
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		//自定义变量 bIsCriticalHit 和 bIsBlockedHit
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 8;
		}
		if (bIsSuccessfulDebuff)
		{
			RepBits |= 1 << 9;
		}
		if (DebuffDamage > 0.f)
		{
			RepBits |= 1 << 10;
		}
		if (DebuffDuration > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if (DebuffFrequency > 0.f)
		{
			RepBits |= 1 << 12;
		}
		// RepBits 位掩码的第 13 位用于控制 DamageType 是否需要序列化
		if (DamageType.IsValid())
		{
			RepBits |= 1 << 13; // 有效时才置位，通知接收端需要读取该字段
		}
	}
	// 序列化 RepBits 本身，使用 7 位（因为只用到 0~6）
	// Ar.SerializeBits(&RepBits, 7);
	Ar.SerializeBits(&RepBits, 14);

	// 以下根据 RepBits 的各个位依次读取或写入对应的成员

	// 如果第 0 位为 1，序列化 Instigator
	// 和 和 0000 0001 和运算 ，如果非零为真
	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	// 第 4 位：序列化 Actors 数组（TArray<TWeakObjectPtr<AActor>>） 该类型没有重载 <<
	// SafeNetSerializeTArray_Default<31> 是一个辅助模板，限制数组最大元素数为 31，安全处理网络序列化
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	// 第 5 位：序列化 HitResult（命中信息结构体）
	if (RepBits & (1 << 5))
	{
		// 如果是加载（接收）端，且 HitResult 尚未分配，则创建新的 FHitResult 对象
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		// 调用 FHitResult 自身的 NetSerialize 函数进行实际序列化
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	// 第 6 位：序列化 WorldOrigin（FVector_NetQuantize10 或类似类型，世界坐标偏移
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true; // 存在原点信息，置为 true
	}
	else
	{
		bHasWorldOrigin = false; // 未传输，置为 false
	}
	//自定义变量的序列化
	if (RepBits & (1 << 7))
	{
		Ar << bIsCriticalHit;
	}

	if (RepBits & (1 << 8))
	{
		Ar << bIsBlockedHit;
	}
	if (RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDebuff;
	}
	if (RepBits & (1 << 10))
	{
		Ar << DebuffDamage;
	}
	if (RepBits & (1 << 11))
	{
		Ar << DebuffDuration;
	}
	if (RepBits & (1 << 12))
	{
		Ar << DebuffFrequency;
	}
	// 反序列化阶段，检查第 13 位是否被置位
	if (RepBits & (1 << 13))
	{
		// 如果是加载（接收端），且共享指针尚未持有对象，则动态创建一个 FGameplayTag
		if (Ar.IsLoading())
		{
			if (!DamageType.IsValid())
			{
				DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		// 调用 FGameplayTag 的自定义网络序列化函数，完成具体数据的读/写
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}

	// 无论是否传输了 Instigator 和 EffectCauser，在加载完成后都调用 AddInstigator
	// 主要目的是初始化 InstigatorAbilitySystemComponent（能力系统组件引用），确保上下文内部状态正确
	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}

	// 设置输出参数为 true，表示序列化成功
	bOutSuccess = true;
	// 函数返回 true，表示序列化过程正常完成
	return true;
}
