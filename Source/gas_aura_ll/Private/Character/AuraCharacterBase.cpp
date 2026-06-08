// gas_aura_ll


#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
// Sets default values
AAuraCharacterBase::AAuraCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


// Called when the game starts or when spawned
void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

FVector AAuraCharacterBase::GetCombatSocketLocation()
{
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
}

void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> AttributesClass,
                                           float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(AttributesClass);
	/*
	 *应用GE的流程
	 * 1. 从ASC->MakeEffectContext()得到 FGameplayEffectContextHandle
	 * 可选：FGameplayEffectContextHandle记录信息
	 * 2. 向ASC->MakeOutgoingSpec()填入GameplayEffectClass、Level、FGameplayEffectContextHandle 得到 FGameplayEffectSpecHandle
	 * 3. ASC->ApplyGameplayEffectSpecTo(Target/Self)(),填入解析，*FGameplayEffectSpecHandle.Data.Get()，(目标TargetASC)
	 */


	FGameplayEffectContextHandle DefaultEffectContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	DefaultEffectContextHandle.AddSourceObject(this); //设置源对象
	const FGameplayEffectSpecHandle DefaultEffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(
		AttributesClass, Level, DefaultEffectContextHandle);

	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*DefaultEffectSpecHandle.Data.Get(),
	                                                             GetAbilitySystemComponent());
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributesClass, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributesClass, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	//只在服务器端添加能力
	if (!HasAuthority())return;

	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->AddCharacterAbilities(StartupAbilities);
}
