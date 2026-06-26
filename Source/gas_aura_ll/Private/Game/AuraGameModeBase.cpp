// gas_aura_ll


#include "Game/AuraGameModeBase.h"

#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
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
 * 从当前游戏实例记录的槽位中读取游戏内的存档数据
 * 
 * @return 加载的 ULoadScreenSaveGame 对象，若不存在则返回新创建的空存档
 */
ULoadScreenSaveGame* AAuraGameModeBase::RetrieveInGameSaveData()
{
	// 获取自定义游戏实例，读取当前加载的存档槽名称和索引
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	// 调用已有的通用加载函数，从该槽位取出存档数据
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

/**
 * 将游戏内的进度数据保存到当前槽位
 * 
 * @param SaveObject 包含最新游戏进度（如玩家位置标签）的存档对象
 */
void AAuraGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	// 获取自定义游戏实例，读取当前使用的存档槽
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	const FString InGameLoadSlotName = AuraGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = AuraGameInstance->LoadSlotIndex;

	// 将进度中的 PlayerStartTag 同步回 GameInstance，以便下次加载时使用正确的出生点
	AuraGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	// 写入磁盘，覆盖原有的存档
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
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

/**
 * 重写游戏模式选择玩家出生点的逻辑
 * 
 * 根据游戏实例中存储的 PlayerStartTag 查找匹配的 PlayerStart 作为出生点，
 * 若未找到匹配项则回退到场景中第一个 PlayerStart。
 * 
 * @param Player 需要选择出生点的控制器
 * @return 选中的出生点 Actor，若无任何 PlayerStart 则返回 nullptr
 */
AActor* AAuraGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// 获取 Aura 自定义游戏实例，读取要使用的出生点标签
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(GetGameInstance());

	TArray<AActor*> Actors;
	// 获取场景中所有 APlayerStart 类型的 Actor
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		// 默认使用第一个 PlayerStart 作为回退
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				// 标签匹配则直接选用该出生点
				if (PlayerStart->PlayerStartTag == AuraGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	// 未找到任何 PlayerStart
	return nullptr;
}

void AAuraGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	Maps.Add(DefaultMapName, DefaultMap);
}
