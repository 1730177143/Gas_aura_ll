// gas_aura_ll


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::BeginPlay()
{
	// 调用父类的 BeginPlay，确保基类逻辑正常执行
	Super::BeginPlay();

	// 断言检查 AuraContext（输入映射上下文）是否已正确设置，若为空则在开发阶段直接崩溃，方便排查问题
	check(AuraContext);

	// 从当前拥有的本地玩家身上获取增强输入子系统
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());
	// 确保子系统获取成功
	check(Subsystem);

	// 将 Aura 的输入映射上下文添加到子系统中，优先级为 0（数字越小优先级越高）
	Subsystem->AddMappingContext(AuraContext, 0);

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