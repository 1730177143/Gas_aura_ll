// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadSlot.generated.h"

// 存档槽的三种状态枚举
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant, // 空槽位
	EnterName, // 等待输入名称
	Taken // 已被占用
};

// 委托：通知 View 切换 WidgetSwitcher 的索引（用于显示不同状态的子控件）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);

// 委托：通知 View 启用/禁用选择按钮
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 单个存档槽的 ViewModel（MVVM 中的 VM）
 * 
 * == UE5 MVVM 简介 ==
 * Model-View-ViewModel 模式将 UI 逻辑与数据分离：
 * - Model（模型）：原始数据，如存档文件。
 * - ViewModel（视图模型）：持有 View 需要的数据和简单逻辑，通过 FieldNotify 自动通知 View 更新。
 * - View（视图）：UMG Widget，通过绑定表达式关联 ViewModel 的属性，无需手动刷新。
 * 
 * 本类继承自 UMVVMViewModelBase（引擎提供的 ViewModel 基类），
 * 用 FieldNotify 标注的属性一旦变化，绑定的 Widget 就会自动更新。
 * 例如：当 PlayerLevel 改变时，显示等级的 TextBlock 会自动刷新。
 */
UCLASS()
class GAS_AURA_LL_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// 通知 View 切换 WidgetSwitcher 索引（如从“空槽位”切到“已有存档”）
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	// 通知 View 启用/禁用选择按钮
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	/** 初始化槽位状态，向 View 广播当前 WidgetSwitcher 索引 */
	void InitializeSlot();

	// 槽位索引（0,1,2...）
	UPROPERTY()
	int32 SlotIndex;

	// 槽位状态（Vacant / EnterName / Taken）
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	// 玩家起始标签（用于关卡加载）
	UPROPERTY()
	FName PlayerStartTag;

	// 地图资源名
	UPROPERTY()
	FString MapAssetName;

	// ----------------------------------------
	// 通过 UE_MVVM_SET_PROPERTY_VALUE 设置的属性会自动通知 View 更新
	// ----------------------------------------
	void SetPlayerName(FString InPlayerName);
	void SetMapName(FString InMapName);
	void SetPlayerLevel(int32 InLevel);
	void SetLoadSlotName(FString InLoadSlotName);

	// 供 View 绑定的 Getter 函数
	FString GetPlayerName() const { return PlayerName; }
	FString GetMapName() const { return MapName; }
	int32 GetPlayerLevel() const { return PlayerLevel; }
	FString GetLoadSlotName() const { return LoadSlotName; }

private:
	/**
	 * FieldNotify 属性：当值变化时，MVVM 框架自动通知绑定此属性的 Widget 刷新。
	 * 例如：一个 TextBlock 绑定 PlayerName，调用 SetPlayerName 后，文本立即更新。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	int32 PlayerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	FString LoadSlotName;
};
