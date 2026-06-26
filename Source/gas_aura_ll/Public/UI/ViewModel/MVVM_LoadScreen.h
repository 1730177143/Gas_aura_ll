// gas_aura_ll

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

class UMVVM_LoadSlot;

// 委托：当用户选择一个槽位时通知 View
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlotSelected);

/**
 * 加载界面的 ViewModel，管理所有存档槽
 * 
 * == MVVM 实践 ==
 * 本类作为“父级 ViewModel”，持有多个 UMVVM_LoadSlot（子 ViewModel）。
 * View（LoadScreen Widget）会绑定本类的属性（如 NumLoadSlots）以及通过
 * 函数获取到的子 ViewModel 属性。当任何 ViewModel 的属性变化时，
 * 对应的 UI 控件会自动更新，无需手动调用 SetText 等。
 */
UCLASS()
class GAS_AURA_LL_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** 创建并初始化 3 个存档槽 ViewModel */
	void InitializeLoadSlots();

	/** 槽位选择事件，蓝图可绑定以处理选择逻辑 */
	UPROPERTY(BlueprintAssignable)
	FSlotSelected SlotSelected;

	/** 存档槽 ViewModel 的类，用于动态创建槽位实例 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

	/**
	 * 根据索引获取对应的槽位 ViewModel
	 * 在蓝图中标记为 BlueprintPure，可直接作为绑定表达式的数据源
	 */
	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnteredName);

	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);

	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);

	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();

	UFUNCTION(BlueprintCallable)
	void PlayButtonPressed();
	
	void LoadData();
	
	/** 设置存档槽总数（触发 FieldNotify，更新 UI） */
	void SetNumLoadSlots(int32 InNumLoadSlots);

	/** 获取存档槽总数 */
	int32 GetNumLoadSlots() const { return NumLoadSlots; }

private:
	// 索引 -> 槽位 ViewModel 的映射
	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlots;

	// 通过 UPROPERTY() 保护，防止被垃圾回收
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;

	UPROPERTY()
	UMVVM_LoadSlot* SelectedSlot;

	/**
	 * 存档槽数量，使用 FieldNotify 标注。
	 * 当调用 SetNumLoadSlots 时，绑定了此属性的 UI（如列表数量）会自动更新。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	int32 NumLoadSlots;
};
