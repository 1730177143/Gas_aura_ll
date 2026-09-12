# CODEBUDDY.md — gas_aura_ll 协作约定（CodeBuddy / opencode 共用）

> 本文件是**常驻薄入口**，两个工具每次对话都会自动加载（CodeBuddy 原生加载；opencode 通过 `opencode.json` 的 `instructions` 加载）。
> 保持精简：只放"项目背景 + 怎么协助我 + 知识库索引"。具体知识放 `docs/ue/`，**需要时再读**。

## 1. 项目背景

- 项目：`gas_aura_ll`，UE5 C++ 项目（Gameplay Ability System + CommonUI），**由我本人编写**，我正在此基础上继续学习。
- 代码：`Source/gas_aura_ll/`（Public / Private）；资产 `Content/`；配置 `Config/`。
- 当前分支：`commonui`。
- 可用工具：Rider MCP（真实 IDE 索引，两个工具均已接入）；UE 技能 `unreal-engine`、`unreal-engine-cpp-pro`、`ue-python-scripting`。

## 2. 我的目标

以**学习 UE** 为主：理解引擎机制、GAS、CommonUI、网络与性能，并沉淀可复用的**知识、技巧、规范、流程**。

## 3. 你要怎么协助我

- **教学优先**：先讲清"原理 / 调用链 / 为什么"，再给代码；不要一上来就大改代码。
- **先查证再回答**：涉及本项目的结论，先用 Rider MCP（`search_symbol` / `get_class_hierarchy` / `analyze_calls` / `find_references`）或读源码确认。**不确定就明说，不要编造 API 或类名。**
- **讲解类问题默认输出结构**：
  1. 一句话结论
  2. 原理 / 调用链（引用 `文件:行`）
  3. 最小可运行示例
  4. 常见坑
  5. 延伸（官方文档 / 相关类）
- 涉及 UE 最佳实践（GC、`UPROPERTY`、网络复制、Tick 性能等）时，参考已装技能。
- 解释与代码注释用**中文**。

## 4. 知识库（跨对话沉淀，仅本项目）

索引见 `docs/ue/README.md`。

- **读取方式**：需要具体知识时，用文件读取工具**按需**打开下面对应的文件；**不要一次性全部读入上下文**。
- **写或修改本项目 C++ 代码前，先读 `docs/ue/conventions.md`。**

| 文件 | 放什么 |
|---|---|
| `docs/ue/concepts.md` | UE / GAS / CommonUI 等**概念与原理** |
| `docs/ue/tips.md` | **技巧与踩坑** |
| `docs/ue/conventions.md` | 本项目**规范**（命名、目录、风格） |
| `docs/ue/workflows.md` | 可复用的**流程 / SOP** |

## 5. 记录规则（重要）

- 出现值得长期保留的新知识时，**在回答末尾主动询问**："要我把这条记到 `docs/ue/xxx.md` 吗？"；我确认后**追加**到对应文件（**不要覆盖**已有内容）。
- 记录统一格式：

  ```markdown
  ## <标题>
  - 结论：
  - 要点：
  - 出处：`文件:行` 或官方链接
  - 日期：YYYY-MM-DD
  ```

- 出处必须可核查，项目相关的写清**文件路径**。
- 零散的个人草稿写 `参考.md`（已被 .gitignore 忽略），**不进**知识库。

## 6. 边界

- 不要自动修改 `.gitignore`、构建配置或删除文件，除非我明确要求。
- 大范围改动先给计划，我确认后再动手。
