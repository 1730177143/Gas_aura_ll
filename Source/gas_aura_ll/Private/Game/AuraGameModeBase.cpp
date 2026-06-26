// gas_aura_ll


#include "Game/AuraGameModeBase.h"

#include "EngineUtils.h"
#include "Game/AuraGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/PlayerStart.h"
#include "gas_aura_ll/AuraLogChannels.h"
#include "Interaction/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
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
 * 保存当前世界（关卡）中所有实现了 SaveInterface 的 Actor 的状态
 * 
 * 执行流程：
 * 1. 获取当前世界的名称，并移除流式关卡前缀以得到干净的关卡名。
 * 2. 从 GameInstance 获取当前激活的存档槽。
 * 3. 加载或创建对应的存档对象。
 * 4. 如果传入了目标地图名（DestinationMapAssetName），则将地图信息写入存档。
 * 5. 如果该地图尚未在存档中，则新建一个 FSavedMap 条目。
 * 6. 清空该地图已有的 Actor 数据，然后遍历世界中的所有 Actor：
 *    a) 忽略无效或未实现 SaveInterface 的 Actor。
 *    b) 将每个需要保存的 Actor 的名称、Transform 序列化，并通过 FMemoryWriter
 *       和 FObjectAndNameAsStringProxyArchive 将其标记为 SaveGame 的变量写入字节数组。
 * 7. 用更新后的 FSavedMap 替换存档中对应的地图数据。
 * 8. 将存档写回磁盘。
 *
 * @param World                   需要保存的世界指针
 * @param DestinationMapAssetName 目标地图资源名（若为下一关卡，则提前写入该名称）
 */
void AAuraGameModeBase::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName) const
{
	// 获取世界名称，并去掉流式关卡前缀（如 UEDPIE_0_）
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGI);

	// 读取当前槽位的存档数据（若不存在则创建新的）
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		// 如果传入了目标地图名，则更新存档中的地图信息（用于切换关卡后加载）
		if (DestinationMapAssetName != FString(""))
		{
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}

		// 如果存档中还没有该地图的记录，则新建一个 FSavedMap 条目
		if (!SaveGame->HasMap(WorldName))
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}

		// 获取当前地图的存档数据（按值拷贝，修改后将替换回去）
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // 清空旧的 Actor 数据，重新填充

		// 遍历世界中的所有 Actor
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			// 忽略无效的或未实现 SaveInterface 的 Actor（无需保存）
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName(); // 记录 Actor 名称（用于加载时匹配）
			SavedActor.Transform = Actor->GetTransform(); // 保存 Transform

			// 使用 FMemoryWriter 创建一个写入内存的归档
			FMemoryWriter MemoryWriter(SavedActor.Bytes);
			// 创建一个代理归档 FObjectAndNameAsStringProxyArchive 会将对象引用和名称转为字符串，
			// 并仅序列化标记为 SaveGame 的属性
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true; // 标记为存档序列化，引擎会依此过滤属性
			Actor->Serialize(Archive); // 序列化 Actor 的 SaveGame 变量到字节数组

			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		// 替换存档中对应的地图数据
		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}

		// 写入磁盘
		UGameplayStatics::SaveGameToSlot(SaveGame, AuraGI->LoadSlotName, AuraGI->LoadSlotIndex);
	}
}

/**
 * 从存档中恢复当前世界（关卡）中所有实现 SaveInterface 的 Actor 的状态
 * 
 * 执行流程：
 * 1. 获取当前世界的干净名称。
 * 2. 检查当前槽位的存档是否存在，若不存在则直接返回。
 * 3. 加载存档对象。
 * 4. 遍历世界中的所有 Actor：
 *    a) 忽略未实现 SaveInterface 的 Actor。
 *    b) 在存档中查找与当前 Actor 同名（FName）的 FSavedActor。
 *    c) 若该 Actor 允许加载 Transform，则恢复其位置/旋转/缩放。
 *    d) 通过 FMemoryReader 和 Archive 将字节数组反序列化回 Actor 的变量。
 *    e) 调用 ISaveInterface::Execute_LoadActor 通知 Actor 存档数据已加载。
 *
 * @param World 需要加载状态的世界指针
 */
void AAuraGameModeBase::LoadWorldState(UWorld* World) const
{
	// 获取世界名称，去掉流式关卡前缀
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	UAuraGameInstance* AuraGI = Cast<UAuraGameInstance>(GetGameInstance());
	check(AuraGI);

	// 仅当存档存在时才执行加载
	if (UGameplayStatics::DoesSaveGameExist(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex))
	{
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(
			UGameplayStatics::LoadGameFromSlot(AuraGI->LoadSlotName, AuraGI->LoadSlotIndex));
		if (SaveGame == nullptr)
		{
			UE_LOG(LogAura, Error, TEXT("存档加载失败"));
			return;
		}

		// 遍历世界中的所有 Actor
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			// 只处理实现了 SaveInterface 的 Actor
			if (!Actor->Implements<USaveInterface>()) continue;

			// 在存档的当前地图数据中寻找与 Actor 名称匹配的条目
			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					// 若 Actor 允许加载 Transform，则恢复它的位置/旋转/缩放
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.Transform);
					}

					// 使用 FMemoryReader 从 Bytes 创建读取用归档
					FMemoryReader MemoryReader(SavedActor.Bytes);
					//同样使用代理归档，标记为存档模式
					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					Archive.ArIsSaveGame = true;
					//Actor 从归档中反序列化，恢复属性
					Actor->Serialize(Archive); // 将二进制数据反序列化回 Actor 的变量

					// 通知 Actor 已加载完成，可执行自定义逻辑（如重新激活组件等）
					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
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

/**
 * 根据地图资源名（Asset Name）反向查找对应的地图显示名称（如 "Abyss"、"Cave" 等）
 * 
 * @param MapAssetName 地图资源名称（例如 "Map_Abyss"），不带路径和包名
 * @return 在 Maps 映射中找到的键（显示名称），若未匹配则返回空字符串
 */
FString AAuraGameModeBase::GetMapNameFromMapAssetName(const FString& MapAssetName) const
{
	// 遍历 Maps（FString -> TSoftObjectPtr<UWorld>），查找资源名与参数匹配的条目
	for (auto& Map : Maps)
	{
		// ToSoftObjectPath().GetAssetName() 获取不带路径的资源文件名（如 "Map_Abyss"）
		if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			return Map.Key; // 返回对应的显示名称键
		}
	}
	return FString(); // 未找到匹配项
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
