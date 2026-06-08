// gas_aura_ll


#include "AbilitySystem/AbilityTask/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	//标准的任务工厂函数，分配一个 UTargetDataUnderMouse 实例并返回。
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	//本地玩家：调用 SendMouseCursorData() 直接获取鼠标数据并上报服务器
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		//其他玩家（模拟端或服务器上非本地控制的角色）：需要等待目标数据从服务器复制过来
		//拿到当前技能实例的 SpecHandle 和激活时的 PredictionKey（预测键）
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		//注册一个委托，当对应 spec/激活键的目标数据被复制到客户端时，调用 OnTargetDataReplicatedCallback。
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey)
		                      .AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);


		//尝试立即调用已存在的复制数据委托 CallReplicatedTargetDataDelegatesIfSet，
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(
			SpecHandle, ActivationPredictionKey);
		//如果数据已经先到了，就直接触发回调；若返回 false 表示数据还没来，则调用 SetWaitingOnRemotePlayerData() 让任务进入等待状态
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	//用于在 GAS 预测系统中开启一段预测窗口，允许客户端先进行一下逻辑，服务器同步后会执行相应逻辑
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);

	//将目标数据通过 RPC 发送到服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	//应在将代表广播回归能力图之前调用 ShouldBroadcastAbilityTaskDelegates 这样可以确保该能力仍然有效。
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
                                                           FGameplayTag ActivationTag)
{
	//当服务器复制来的目标数据到达客户端，并匹配到之前注册的回调时触发。数据已经收到，可以清除缓存
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	//如果任务有效，广播 ValidData 委托，
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
