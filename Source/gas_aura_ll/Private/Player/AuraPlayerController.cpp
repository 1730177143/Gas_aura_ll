// gas_aura_ll


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	// 从当前拥有的本地玩家身上获取增强输入子系统
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());

	if (Subsystem)//多人时本地其他玩家存在合法的为空情况
	{
		// 将 Aura 的输入映射上下文添加到子系统中，优先级为 0（数字越小优先级越高）
		Subsystem->AddMappingContext(AuraContext, 0);
	}



	// 显示鼠标光标（通常用于 UI 交互或人物界面）
	bShowMouseCursor = true;
	// 设置默认鼠标光标样式为系统默认箭头
	DefaultMouseCursor = EMouseCursor::Default;

	// 构造一个“游戏与 UI 混合”的输入模式，使玩家同时能操控角色和与 UI 交互
	FInputModeGameAndUI InputModeData;
	// 设置鼠标锁定行为：不将鼠标锁定在视口内，允许鼠标自由移出窗口
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	// 设置在鼠标被捕获（例如按住右键旋转视角）时不隐藏光标
	InputModeData.SetHideCursorDuringCapture(false);
	// 应用该输入模式
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 将通用的 InputComponent 转换为增强输入组件，以便使用增强输入系统绑定动作
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	// 绑定移动动作：当 MoveAction 被触发（按下/持续按住）时，调用 Move 函数
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// 从输入动作值中提取 2D 轴向量 (X: 左右, Y: 前后)
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	// 获取当前控制器的旋转角度（视角方向）
	const FRotator Rotation = GetControlRotation();
	// 仅保留 Yaw（水平旋转）分量，忽略 Pitch 和 Roll，确保移动在同一平面上
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// 根据 Yaw 旋转计算出世界空间中的前方向向量 (X轴)
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// 根据 Yaw 旋转计算出世界空间中的右方向向量 (Y轴)
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControllerPawn = GetPawn<APawn>())
	{
		// 向前/后移动：InputAxisVector.Y 正值表示向前，负值表示向后
		ControllerPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		// 向左/右移动：InputAxisVector.X 正值表示向右，负值表示向左
		ControllerPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			ThisActor->HighlightActor();
		}
	}
	else
	{
		if (ThisActor == nullptr)
		{
			LastActor->UnHighlightActor();
		}
		else
		{
			if (LastActor != ThisActor)
			{
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
		}
	}
}
