// gas_aura_ll


#include "AbilitySystem/Abilities/AuraBeamSpell.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UAuraBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

/**
 * 使用球形射线追踪从武器尖端到目标位置，检测并记录第一个被击中的目标
 * 
 * - 如果目标实现了 CombatInterface，则绑定其死亡委托，以便在目标死亡时做出响应
 * 
 * @param BeamTargetLocation  光束法术的目标位置（通常为鼠标指向的世界坐标）
 */
void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	// 确保 OwnerCharacter 有效
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		// 获取角色当前装备的武器
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter); // 忽略施法者自身，避免击中自己
			FHitResult HitResult;
			// 获取武器尖端插槽的世界位置作为射线起点
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));
			// 使用球形射线（半径10）从武器尖端追踪到目标位置
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				10.f,
				TraceTypeQuery1, // 自定义的追踪通道
				false, // 不追踪复杂碰撞
				ActorsToIgnore,
				EDrawDebugTrace::None, // 不绘制调试线
				HitResult,
				true); // 忽略自身

			if (HitResult.bBlockingHit)
			{
				// 记录碰撞点和碰撞的 Actor，用于后续处理（如施加伤害、显示特效）
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}
