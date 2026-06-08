// gas_aura_ll


#include "Character/AuraEnemy.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "gas_aura_ll/gas_aura_ll.h"

AAuraEnemy::AAuraEnemy()
{
	// 让敌人的网格体对ECC_Visibility产生阻挡响应，常用于高亮描边或目标选择
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	// 创建自定义的 ASC 并设置为网络可复制
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	// 使用 Minimal 复制模式，仅复制Gameplay Cues和Gameplay Tags,Gameplay Effects不会被复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	// 创建属性集
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AuraAttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void AAuraEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

int32 AAuraEnemy::GetPlayerLevel()
{
	return Level;
}

void AAuraEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();

	if (UAuraUserWidget* AuraUserWidget = Cast<UAuraUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		AuraUserWidget->SetWidgetController(this);
	}

	if (const UAuraAttributeSet* AuraAS = Cast<UAuraAttributeSet>(AttributeSet))
	{
		//绑定
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		//广播初始值
		OnHealthChanged.Broadcast(AuraAS->GetHealth());
		OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	}
}

void AAuraEnemy::InitAbilityActorInfo()
{
	// 将自身注册为 ASC 的owner和Avatar，这是 ASC 正常工作的前提
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	// 通知自定义 ASC：ActorInfo 已设置完毕，可以绑定内部委托
	// 这样做使 Character 只需要单向依赖 ASC，ASC 自身负责处理效果事件
	Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	InitializeDefaultAttributes();
}
