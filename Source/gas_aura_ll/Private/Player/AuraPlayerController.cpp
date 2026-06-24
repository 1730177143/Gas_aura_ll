// gas_aura_ll


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include " Input/AuraInputComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "gas_aura_ll/gas_aura_ll.h"
#include "Interaction/EnemyInterface.h"
#include "UI/Widget/DamageTextComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
	AutoRun();
	// 更新魔法阵位置
	UpdateMagicCircleLocation();
}

/**
 * 显示魔法阵（光标指示器）
 * 如果当前没有魔法阵实例，则生成一个并设置其贴花材质
 * 
 * @param DecalMaterial 用于魔法阵贴花的材质接口（可为 nullptr）
 */
void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			// 设置魔法阵贴花的材质（例如用于不同技能类型显示不同图案）
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

/**
 * 隐藏魔法阵
 * 直接销毁已存在的魔法阵实例
 */
void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter,
                                                            bool bBlockedHit, bool bCriticalHit)
{
	//IsValid 会额外检查是否待销毁
	//IsLocalController 检查是否本地控制器， 避免服务器端和客户端都调用显示伤害数字
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		//动态创建手动注册
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(),
		                              FAttachmentTransformRules::KeepRelativeTransform);
		//创建后分离，执行动画
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	// 从当前拥有的本地玩家身上获取增强输入子系统
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer());

	if (Subsystem) //多人时本地其他玩家存在合法的为空情况
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
	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	// 绑定移动动作：当 MoveAction 被触发（按下/持续按住）时，调用 Move 函数
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed,
	                                       &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}

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

/**
 * 每帧执行的鼠标悬停追踪
 * - 检测鼠标下是否有 Actor
 * - 处理高亮/取消高亮
 * - 当存在阻止光标追踪的标签时（如打开技能菜单），清除所有高亮并跳过
 */
void AAuraPlayerController::CursorTrace()
{
	// 如果 ASC 存在且含有阻止光标追踪的标签，则取消所有高亮并直接返回
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		if (LastActor) LastActor->UnHighlightActor();
		if (ThisActor) ThisActor->UnHighlightActor();

		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	// 根据魔法阵是否存在选择不同的追踪通道：存在时排除玩家，以免魔法阵遮挡玩家
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	// 更新上一帧和当前帧的悬停 Actor
	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	// 若 Actor 发生变化，取消旧的高亮，添加新的高亮
	if (LastActor != ThisActor)
	{
		if (LastActor)
		{
			LastActor->UnHighlightActor();
		}
		if (ThisActor)
		{
			ThisActor->HighlightActor();
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		//拿到曲线里角色最近的节点
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(
			ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		//拿到角色的移动方向
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(
			LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		//检测距离，判断是否到达目的地
		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

/**
 * 更新魔法阵位置
 * 将魔法阵移动到光标命中点（地面位置）
 */
void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}


void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (ThisActor)
		{
			TargetingStatus = ThisActor != nullptr
				                  ? ETargetingStatus::TargetingEnemy
				                  : ETargetingStatus::TargetingNonEnemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}
	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}
	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			//自动寻路
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				//制作平滑路线
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				//选取最后一个可达点作为目的地，避免目的地不可达
				if (NavPath->PathPoints.Num() > 0)
				{
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	//检测鼠标左键
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
		return;
	}
	//检测是否选中目标
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
	}
	else
	{
		//按住移动
		FollowTime += GetWorld()->GetDeltaSeconds();

		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			//得到指向目的地的向量
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}
