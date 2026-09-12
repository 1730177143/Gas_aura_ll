# 流程 / SOP

> 可复用的操作步骤。格式与维护约定见 [README.md](./README.md)。
> 新条目追加到文件末尾。

## 学习一个类 / 系统的推荐流程

1. 查结构：Rider MCP `search_symbol` / `get_class_hierarchy`
2. 查调用关系：`analyze_calls` / `find_references`
3. 读关键源码：`read_file`（确认入口与生命周期，记下 `文件:行`）
4. 请 AI 按"结论 → 调用链 → 最小示例 → 常见坑 → 延伸"讲解
5. 沉淀：新概念写 `concepts.md`，坑/技巧写 `tips.md`

## 排查编译 / 运行问题

1. `get_project_problems` 查看 IDE 的 Problems（不触发新分析）
2. 需要最新构建诊断：先 `build_solution_start`，再轮询 `build_solution_state`
3. 定位到文件后 `read_file` 相关源码，再请 AI 分析
4. 结论沉淀到 `tips.md`

<!-- 新条目追加到下方 -->
