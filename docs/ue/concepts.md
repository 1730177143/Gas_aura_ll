# 概念与原理

> UE / GAS / CommonUI 等概念性知识的沉淀。格式与维护约定见 [README.md](./README.md)。
> 新条目追加到文件末尾。

## Interaction 接口层（本项目）

- 结论：本项目把"可交互能力"拆成多个 `UInterface`，集中放在 `Source/gas_aura_ll/Public/Interaction/`。
- 要点：
  - 目前已有：`UPlayerInterface`、`UEnemyInterface`、`UHighlightInterface`、`UCombatInterface`、`USaveInterface`
  - 命名：UInterface 用 `U` 前缀（如 `UCombatInterface`），实现类用 `I` 前缀（Epic 规范）
  - 目的：让不同系统（交互、高亮、敌人、存档）以接口解耦，而非互相直接依赖
- 出处：`Source/gas_aura_ll/Public/Interaction/`
- 日期：2026-09-11
- 状态：**待补充** —— 各接口的职责与关键方法清单

## GameplayTag 的三种来源，本项目以"原生单例"为主

- 结论：本项目标签**主要**由 C++ 原生单例 `FAuraGameplayTags` 提供，代码里通过 `FAuraGameplayTags::Get().Xxx` 读取。
- 要点：
  - **原生注册**：`InitializeNativeGameplayTags()` → `UGameplayTagsManager::Get().AddNativeGameplayTag(FName("..."), FString("中文说明"))`，约 70 个；在 `UAuraAssetManager::StartInitialLoading()`（引擎早期）调用，避免用到标签时为空
  - **补充声明**：`Config/DefaultGameplayTags.ini`（`Message.*`、`GameplayCue.*`、`Event.Montage.*`…）+ 标签表 `DT_PrimaryAttributs`
  - **反例**：`FGameplayTag::RequestGameplayTag(FName("..."))` 运行时按名查找，全项目仅 1 处；新代码不要这样写
  - 另有 `DamageTypesToResistances` / `DamageTypesToDebuffs` 两张 `TMap`，用于伤害类型 → 抗性/减益的映射
- 出处：`Source/gas_aura_ll/Public/AuraGameplayTags.h`、`AuraGameplayTags.cpp`、`Config/DefaultGameplayTags.ini`
- 日期：2026-09-12

## 玩家的 GAS 挂在 PlayerState（OwnerActor ≠ AvatarActor）

- 结论：敌人把 ASC 放在自身(`AAuraEnemy`)；**玩家把 ASC 和 AttributeSet 挂在 `AAuraPlayerState`**，因此玩家的 OwnerActor 是 PlayerState、AvatarActor 是 Character。
- 要点：
  - 目的：角色重生/切换时**不希望 GAS 组件被销毁重建**，挂在 PlayerState 上可跨角色保持
  - 设置时机分两端：**服务端**在 Pawn 被控制时（`PossessedBy`）；**客户端**在收到带 PlayerState 的回调时（`OnRep_PlayerState`）
  - 复制模式为 `Mixed`
  - 因为客户端与服务器的实例不一定同时一致，取值时要注意"在哪个端调用"
  - 因 `UAuraAttributeSet` 需要访问 PlayerState 数据，本项目通过 `IPlayerInterface` 间接访问以避免循环依赖
- 出处：`readme.md`（GAS 章节）、`Source/gas_aura_ll/Public/Player/AuraPlayerState.h`、`Character/AuraCharacterBase.h`
- 日期：2026-09-12
- 状态：待补充（`InitAbilityActorInfo` 的完整调用链）

<!-- 新条目追加到下方 -->
