# 本项目规范

> `gas_aura_ll` 的目录、命名、GAS 与代码风格约定。**写或改代码前必读。**
> 格式与维护约定见 [README.md](./README.md)。
> 内容基于对源码的实际勘察；标 **⚠️ 现状** 的表示当前代码**尚未完全遵守**，属"目标约定 + 现存偏差"。

## 0. 引擎与构建

- 单一 **Game 模块**：`gas_aura_ll`（`Type: Runtime`），模块导出宏 **`GAS_AURA_LL_API`**。
- **引擎版本：UE 5.8**（`gas_aura_ll.uproject` 的 `EngineAssociation` = `5.8`；已于 2026-09 从 5.6 迁移）。
  - ✅ 两个 Target 的 `IncludeOrderVersion` 已统一为 `Unreal5_8`（2026-09-12）。
  - ⚠️ 仅剩：`readme.md` 顶部仍写 5.6（陈旧）。
- `Build.cs` 依赖：
  - Public：`Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayAbilities, CommonUI`
  - Private：`GameplayTags, GameplayTasks, NavigationSystem, Niagara, AIModule`
- 启用的插件：`GameplayAbilities`、`CommonUI`、`ModelViewViewModel`(+`Preview`)、`MotionWarping`、`GASToolsets`、`ToolsetRegistry`、`EditorToolset`、`ModelingToolsEditorMode`(仅 Editor)
- `Config/DefaultGame.ini`：
  - `AbilitySystemGlobalsClassName=/Script/gas_aura_ll.AuraAbilitySystemGlobals`（启用自定义 GE Context）
  - `GameplayCueNotifyPaths=/Game/Blueprints/AbilitySystem/GameplayCueNotifies`
  - `CommonButtonAcceptKeyHandling=TriggerClick`
- `Config/DefaultEngine.ini`：`[ConsoleVariables] net.MaxRPCPerNetUpdate=10`

## 1. 目录结构

```
Source/gas_aura_ll/
├── gas_aura_ll.Build.cs / .h / .cpp     # 模块文件在「模块根」，不在 Public/Private 下
├── AuraLogChannels.h / .cpp             # 自定义日志（模块根）
├── Public/    (头文件)  ┐ 两者对称
└── Private/   (实现)    ┘
```

`Public/` 与 `Private/` 下的功能域子目录：

| 子目录 | 放什么 |
|---|---|
| `AbilitySystem/` | GAS 核心：ASC、AttributeSet、BlueprintLibrary、Globals |
| `AbilitySystem/Abilities/` | `UGameplayAbility` 子类 |
| `AbilitySystem/AbilityTask/` | `UAbilityTask` 子类 |
| `AbilitySystem/AsyncTasks/` | 蓝图异步节点（`UBlueprintAsyncActionBase`） |
| `AbilitySystem/Data/` | DataAsset / 表结构（`UCharacterClassInfo`、`UAbilityInfo`、`UAttributeInfo`、`ULevelUpInfo`、`ULootTiers`） |
| `AbilitySystem/ExecCalc/` | `UGameplayEffectExecutionCalculation` |
| `AbilitySystem/MMC/` | `UGameplayModMagnitudeCalculation` |
| `AbilitySystem/Debuff/`、`AbilitySystem/Passive/` | 表现组件（Niagara） |
| `Actor/` `AI/` `Character/` `Checkpoint/` `Game/` `Input/` `Interaction/` `Player/` `UI/` | 按功能域划分 |
| `UI/` 之下再分 | `HUD/`、`Widget/`、`WidgetController/`、`ViewModel/` |

- ✅ **已修正（2026-09-12）**：输入目录原名为 ` Input`（带前导空格），include 曾被迫写成 `#include " Input/AuraInputComponent.h"`。现已重命名为 `Input/`，include 统一为 `#include "Input/..."`。**注意旧资料/旧代码里可能残留 ` Input/` 写法。**
- `Public/` 根目录直接放 `AuraAbilityTypes.h`、`AuraAssetManager.h`、`AuraGameplayTags.h`（实现同样在 `Private/` 根）。
- `Interaction/*.cpp` 多为只含 include 的空壳（接口类的正常现象）。

## 2. 命名

### 通用前缀（Epic 规范）

| 类型 | 前缀 | 示例 |
|---|---|---|
| UObject | `U` | `UAuraAttributeSet` |
| AActor | `A` | `AAuraCharacter` |
| 结构体 | `F` | `FAuraGameplayEffectContext` |
| 枚举 | `E` | `ECharacterClass` |
| 接口（UInterface / 实现类） | `U` / `I` | `UCombatInterface` / `ICombatInterface` |
| 布尔 | `b` | `bIsStunned` |
| 模板 | `T` | `TArray`、`TMap` |

- 委托命名：`FOn<...>` 或 `F<...>Signature`（如 `FOnAttributeChangedSignature`、`FCooldownChangeSignature`）。
- ⚠️ **`Aura` 前缀不统一**：核心类带（`AAuraCharacter`、`UAuraAttributeSet`、`UAuraHUD`），部分不带（`ACheckpoint`、`AMagicCircle`、`UDamageTextComponent`、`UWaitCooldownChange`、`UExecCalc_Damage`、`UMMC_*`）。**新增"属于 Aura 玩法"的类建议带 `Aura`**。

### 资产前缀（`Content/`）

`BP_`(蓝图) · `WBP_`(控件) · `ABP_`(动画蓝图) · `AM_`(蒙太奇) · `AN_`(动画通知) · `GA_`(技能) · `GE_`(效果) · `GC_`(Cue) · `DA_`(DataAsset) · `DT_`(DataTable) · `CT_`(CurveTable) · `IMC_`(输入映射) · `M_`/`MI_`/`PP_`(材质) · `SM_`(静态网格)

- `Content/` 顶层：`Blueprints/`（含 `AbilitySystem/`、`UI/`、`Character/`…）、`Assets/`、`Maps/`。
- ⚠️ `GE_Data/`（`CT_*.csv` / `*.json` 源表）放在**仓库根目录**而非 `Content/`，与常规不同；改表时注意路径。

## 3. GAS 约定

### 类职责

- **Ability 基类**：`UAuraGameplayAbility` → `UAuraDamageGameplayAbility`、`UAuraPassiveAbility`、`UAuraProjectileSpell`、`UAuraBeamSpell`；具体技能以**技能名**命名（`UAuraFireBolt`、`UAuraSummonAbility`、`UAuraArcaneShards`）。
- **AttributeSet**：单一 `UAuraAttributeSet`（**不拆分**）。
- **ASC**：`UAuraAbilitySystemComponent`（承载大量自定义逻辑）。
- **ExecutionCalculation**：`UExecCalc_<Name>`（如 `UExecCalc_Damage`）。
- **ModMagnitude**：`UMMC_<Name>`（如 `UMMC_MaxHealth`）。
- **自定义 GE Context**：`FAuraGameplayEffectContext`（**用 `struct`**），由 `UAuraAbilitySystemGlobals::AllocGameplayEffectContext()` 返回。
- **GameplayCue**：**无 C++ 类**，全部是蓝图 `GC_*`，靠 `DefaultGame.ini` 的 `GameplayCueNotifyPaths` 扫描。

### 属性（Attribute）书写范式

```cpp
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
FGameplayAttributeData Strength;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);
```

- `ATTRIBUTE_ACCESSORS` 定义在 `AuraAttributeSet.h`（`GAMEPLAYATTRIBUTE_*` 宏的组合）。
- 每个属性配 `void OnRep_X(const FGameplayAttributeData& OldX) const`（统一 `const`）。
- Meta 属性（`IncomingDamage`、`IncomingXP`）**只在服务端**参与计算，不复制。

### GameplayTag（★ 以此为准）

- **主机制：C++ 原生标签单例 `FAuraGameplayTags`**（`AuraGameplayTags.h/.cpp`）
  - 在 `InitializeNativeGameplayTags()` 中用 `UGameplayTagsManager::Get().AddNativeGameplayTag(FName("..."), FString("中文说明"))` 注册（约 70 个）
  - 在 `UAuraAssetManager::StartInitialLoading()` 中调用（引擎早期）
  - 读取统一用 **`FAuraGameplayTags::Get().Xxx`**
- **补充来源**：`Config/DefaultGameplayTags.ini` 声明了 `Message.*`、`GameplayCue.*`、`Event.Montage.*` 等标签，并引用标签表 `DT_PrimaryAttributs`。
- ❌ **不要**用 `FGameplayTag::RequestGameplayTag(FName("..."))` 运行时按名查找（全项目仅 1 处反例，见 `OverlayWidgetController.cpp`）。

## 4. 代码风格与惯用法

- **文件头**：业务文件首行统一 `// gas_aura_ll`（**不是** Epic 版权头；仅 5 个骨架文件带 Epic 头）。
- **注释**：中文为主；教学式长注释 / ASCII 流程图是允许的（本项目特色）。
- **断言**：`check` / `checkf` 使用频繁，提示文案**中文**（如 `checkf(OverlayWidgetClass, TEXT("Overlay Widget Class未初始化，请在BP_AuraHUD中设置"))`）。⚠️ 现状 `check` 远多于 `ensure`；**可恢复的错误优先 `ensure`**。
- **日志**：用自定义 `LogAura`（`AuraLogChannels.h`）。❌ 不要用 `LogTemp`（现存 1 处误用：`Private/Input/AuraInputConfig.cpp`）。
- **指针**：`UPROPERTY` 成员**新代码统一用 `TObjectPtr<T>`**。⚠️ 现状新旧混用（如 `AuraCharacterBase.h` 中 `TObjectPtr<>` 与 `UNiagaraSystem*` 并存）。
- **`UPROPERTY`**：大量暴露给蓝图；成员默认值显式初始化（`= nullptr`、`= 0.f`、`= FVector::ZeroVector`）。
- **`const`**：getter、`IsDead() const`、`OnRep_X() const` 一律加 `const`；参数用 `const T&`；短 getter 可用 `FORCEINLINE`。
- **`struct` vs `class`**：`USTRUCT` 数据体一律用 `struct`（含 `FAuraGameplayEffectContext`）。
- **无命名空间**：全项目不使用 `namespace`，靠前缀 + 模块宏避免冲突。
- **include 顺序**：`.h` 中 `CoreMinimal.h` 在前、`*.generated.h` **永远最后**；`.cpp` 第一行 include 自己的头。
- **自定义通道 / 深度宏**：集中写在模块头 `Source/gas_aura_ll/gas_aura_ll.h`
  （`CUSTOM_DEPTH_*`、`ECC_Projectile`、`ECC_Target`、`ECC_ExcludePlayers`）。

## 5. 硬性禁忌

- 🚫 **禁止中文 / 非 ASCII 文件名**（源码、目录、资产、图片）。原因：UnrealBuildTool 解析 `git status` 输出时会因转义路径**崩溃**（`0xe0434352` / `System.Text.Json` 越界）。缓解：`git config core.quotepath false`。详见 [tips.md](./tips.md)。
- 🚫 不要在代码里硬编码 `FName("...")` 形式的标签字符串。
- 🚫 不要用 `LogTemp`。

## 6. 待清理项（TODO）

- [x] 目录 ` Input/` → `Input/`，并同步 3 处 include（2026-09-12 完成）
- [x] 统一两个 Target 的 `IncludeOrderVersion` → `Unreal5_8`（2026-09-12 完成）
- [ ] 更新 `readme.md` 顶部 `**UE VERSION:** 5.6` → `5.8`
- [ ] 处理 2 处 UE 5.8 弃用警告（`AuraAttributeSet.cpp:317`、`AuraPassiveAbility.cpp:28`）
- [ ] 评估 `GE_Data/` 源表是否迁入 `Content/`
