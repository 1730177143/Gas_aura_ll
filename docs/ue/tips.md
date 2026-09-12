# 技巧与踩坑

> 报错、坑、更优写法的沉淀。格式与维护约定见 [README.md](./README.md)。
> 新条目追加到文件末尾。

## UObject 指针必须加 UPROPERTY，否则会被 GC 回收

- 结论：`UObject*` 成员若不加 `UPROPERTY()`，垃圾回收器不追踪它，指针会变成悬空指针（dangling）。
- 要点：
  - 成员：`UPROPERTY() UMyComponent* Comp;`
  - 可选引用用 `TWeakObjectPtr<T>`（不阻止回收，用前 `IsValid()` 检查）
  - 创建 UObject 用 `NewObject<T>()`，不要裸 `new`
- 出处：UE 官方 GC 文档 / 已装技能 `unreal-engine`
- 日期：2026-09-11

## 不需要 Tick 就别开

- 结论：默认关闭 Tick 可省可观的性能开销。
- 要点：
  - 构造函数里 `PrimaryActorTick.bCanEverTick = false;`
  - 用定时器代替 `Tick + 计数器`：`GetWorldTimerManager().SetTimer(...)`
  - 蓝图 Tick 较贵，热逻辑尽量下沉到 C++
- 出处：已装技能 `unreal-engine`
- 日期：2026-09-11

## 中文文件名会让 UnrealBuildTool 崩溃（本项目已踩过）

- 结论：工作区里存在**中文/非 ASCII 文件名**时，UBT 解析 `git status` 输出会抛异常直接崩溃，编译失败。
- 现象：
  - `UnrealBuildTool failed with exit code 0xe0434352`（.NET CLR 未处理异常）
  - 或 `IndexOutOfRangeException ... System.Text.Json.JsonWriterHelper.EscapeString`
  - UBT 日志 `%localappdata%\UnrealBuildTool\Log.txt` 中途戛然而止
- 原因：Git 默认 `core.quotepath=true`，把非 ASCII 路径转义为八进制并加引号（如 `"\346\225\214..."`），UBT 的 `GitSourceFileWorkingSet` 解析时把引号误判为非法路径分隔符。
- 解决：
  1. **首选**：改名 —— 源码/资产/目录一律用英文（本项目规范已列为硬性禁忌）
  2. 缓解：`git config core.quotepath false`（当前仓库，或 `--global`）
  3. 若已残留：`git reset --hard HEAD` + `git clean -fd` 清理工作区
- 出处：`readme.md` → "UE 5.6 编译错误记录 / Git 中文文件名"
- 日期：2026-09-12

## 输入目录曾带前导空格（已修正）

- 结论：`Public/ Input/`、`Private/ Input/` 原目录名**开头带一个空格**，导致 include 必须写成 `#include " Input/AuraInputComponent.h"`。**2026-09-12 已重命名为 `Input/`**，include 统一为 `#include "Input/..."`。
- 要点：
  - 教训：目录名里的空格不会报错，但会让所有 include 变诡异，且极易被"顺手改回"而编译失败
  - 在旧提交、旧笔记里看到 `" Input/...` 属历史写法，**不要**再照抄
  - 重命名用 `git mv` 保留历史；本次涉及 4 个文件、3 处 include
- 出处：`git log`（2026-09-12 重命名）、`docs/ue/conventions.md`
- 日期：2026-09-12

<!-- 新条目追加到下方 -->
