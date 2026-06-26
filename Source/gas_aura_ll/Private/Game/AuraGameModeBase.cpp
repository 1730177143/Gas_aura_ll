// gas_aura_ll


#include "Game/AuraGameModeBase.h"

#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
/**
 * 保存或更新一个存档槽的数据
 * 
 * @param LoadSlot   待保存的 LoadSlot ViewModel，包含玩家名称、地图等信息
 * @param SlotIndex  存档槽索引，用于定位特定存档文件
 */
void AAuraGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// 如果该槽位已有旧存档，则先删除，避免数据残留
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}

	// 创建新的存档对象，并转换为自定义存档类型
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);

	// 将 ViewModel 中的数据填充到存档对象中
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->SaveSlotStatus = Taken; // 标记为已占用
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->MapAssetName = LoadSlot->MapAssetName;
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;

	// 写入磁盘
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

/**
 * 读取指定存档槽的数据，若不存在则返回一个默认的空存档对象
 * 
 * @param SlotName  存档槽名称（通常与槽位 ViewModel 的 LoadSlotName 一致）
 * @param SlotIndex 存档槽索引
 * @return 加载或新创建的 ULoadScreenSaveGame 对象指针
 */
ULoadScreenSaveGame* AAuraGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;

	// 若存档已存在，直接加载
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		// 否则创建一个全新的默认存档对象（字段均为默认值）
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}

	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;
}

/**
 * 删除指定存档槽的数据
 * 
 * @param SlotName  存档槽名称
 * @param SlotIndex 存档槽索引
 */
void AAuraGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

/**
 * 根据选中的存档槽加载对应地图
 * 
 * @param Slot 选中的 LoadSlot ViewModel，其中 MapName 用于查找地图引用
 */
void AAuraGameModeBase::TravelToMap(UMVVM_LoadSlot* Slot)
{
	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;

	// 使用软对象指针异步加载并打开地图
	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	Maps.Add(DefaultMapName, DefaultMap);
}
