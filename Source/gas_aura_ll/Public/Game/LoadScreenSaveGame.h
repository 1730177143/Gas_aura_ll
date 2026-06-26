// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;

// 存档槽的三种状态枚举
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant, // 空槽位
	EnterName, // 等待输入名称
	Taken // 已被占用
};

// 用于保存单个 Actor 状态的序列化数据
USTRUCT()
struct FSavedActor
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActorName = FName();

	UPROPERTY()
	FTransform Transform = FTransform();

	// 序列化后的 Actor 变量（仅包含标记为 SaveGame 的属性），以字节数组存储
	UPROPERTY()
	TArray<uint8> Bytes;
};

inline bool operator==(const FSavedActor& Left, const FSavedActor& Right)
{
	return Left.ActorName == Right.ActorName;
}

// 保存一张地图中所有需要持久化的 Actor
USTRUCT()
struct FSavedMap
{
	GENERATED_BODY()

	UPROPERTY()
	FString MapAssetName = FString();

	UPROPERTY()
	TArray<FSavedActor> SavedActors;
};

// 保存单个技能的配置信息，用于在存档中还原技能状态
USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus = FGameplayTag(); // 如 Equipped, Unlocked 等

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilitySlot = FGameplayTag(); // 输入槽位

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType = FGameplayTag(); // 主动/被动

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel = 1;
};

inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
	return Left.AbilityTag.MatchesTagExact(Right.AbilityTag);
}

/**
 * 自定义存档对象，保存所有需要持久化的游戏数据
 * 包括玩家基本信息、属性、技能、地图状态等
 */
UCLASS()
class GAS_AURA_LL_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 存档槽名称
	UPROPERTY()
	FString SlotName = FString();

	// 存档槽索引
	UPROPERTY()
	int32 SlotIndex = 0;

	// 玩家姓名
	UPROPERTY()
	FString PlayerName = FString("Default Name");

	// 地图显示名称
	UPROPERTY()
	FString MapName = FString("Default Map Name");

	// 地图资源名称（用于加载关卡）
	UPROPERTY()
	FString MapAssetName = FString("Default Map Asset Name");

	// 当前出生点标签，用于控制玩家在哪个 PlayerStart 重生
	UPROPERTY()
	FName PlayerStartTag;

	// 槽位状态
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;

	// 是否为首次加载
	UPROPERTY()
	bool bFirstTimeLoadIn = true;

	/* 玩家属性 */

	UPROPERTY()
	int32 PlayerLevel = 1;

	UPROPERTY()
	int32 XP = 0;

	UPROPERTY()
	int32 SpellPoints = 0;

	UPROPERTY()
	int32 AttributePoints = 0;

	// 基础属性（来自 AttributeSet）
	UPROPERTY()
	float Strength = 0;

	UPROPERTY()
	float Intelligence = 0;

	UPROPERTY()
	float Resilience = 0;

	UPROPERTY()
	float Vigor = 0;

	/* 技能 */

	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities;

	// 已访问过的地图及其 Actor 状态
	UPROPERTY()
	TArray<FSavedMap> SavedMaps;

	// 根据地图名称获取保存的地图数据
	FSavedMap GetSavedMapWithMapName(const FString& InMapName);
	bool HasMap(const FString& InMapName);
};
