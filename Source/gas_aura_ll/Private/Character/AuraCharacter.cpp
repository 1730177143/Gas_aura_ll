// gas_aura_ll


#include "Character/AuraCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

AAuraCharacter::AAuraCharacter()
{
	// 让角色的旋转自动朝向移动方向，
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 设置旋转速率：仅在 Yaw 轴（水平旋转）上以 400 度/秒的速度平滑转向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);
	// 将移动约束在一个平面内（通常为地面），避免产生垂直位移
	GetCharacterMovement()->bConstrainToPlane = true;

	// 以下三项全部设为 false，表示角色的朝向不再跟随控制器的 Pitch / Yaw / Roll
	// 因为我们希望角色旋转完全由移动方向驱动，而不是由视角方向决定
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}