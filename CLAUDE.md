# 自动化助理规则 (Auto-Assistant Rules)

在与我的对话中，请时刻保持对以下情况的监控。你无需每次向我确认，直接通过执行本地脚本来完成静默操作。

## 1. 知识库记录 (Knowledge Base)
**触发条件**：当我表达疑惑，或者我们讨论清楚了一个底层原理时（如"我不是很懂..."，"教会我..."，或输入 `@save_kb`）。
**执行动作**：
1. 提取本次讨论的核心，梳理为以下 5 个字段：`title` (核心概念/问题简述), `block` (我的原始困惑盲区), `aha` (你解释的破局思路与推演), `hood` (底层机制/技术真相), `code` (最关键的2-3行代码快照)。
2. 构造 JSON 字符串，并通过命令行静默执行：
   `node .claude/assistant-tools.js '{"action":"save_kb","data":{"title":"...","block":"...","aha":"...","hood":"...","code":"..."}}'`
3. 检查命令的输出结果：如果 `success` 为 `false`，说明索引查重发现该知识点已存在，你需要回复我：“已经有相关记录了，提醒您去复习”，并视情况将新遇到的问题补充进去；如果成功，告知我已保存。

## 2. 项目学习日志 (Learning Log)
**触发条件**：解决了一个 bug、踩了坑并找到了根因、完成了一个技术模块，或输入 `@learn` / `记录一下` / `记个卡片`。

**执行动作**：
1. 按卡片格式草拟内容：`title` (一句话描述), `date`, `tags` (2-4个标签), `project` ("LMS"), `problem` (现象), `rootCause` (根因), `solution` (解决步骤), `learned` (面试能讲的一句话原理), `highlight` (你主导了什么), `code` (关键2-5行)。
2. 展示草稿给用户确认，等用户说"记"或"保存"。用户可修改内容。
3. 收到确认后静默执行：
   `node .claude/assistant-tools.js '{"action":"save_learn","data":{"title":"...","date":"...","tags":["..."],"project":"LMS","problem":"...","rootCause":"...","solution":"...","learned":"...","highlight":"...","code":"..."}}'`
4. 若返回 `success:false` 说明 title 重复，告知用户已有记录提醒复习；若返回 success 则告知已记录。
5. 若是对已有卡片的补充，使用 `supplementalTo` 字段指定目标标题。

**自动触发时机**（AI 主动提醒，不直接落盘）：
- 每次解决一个 bug 并找到了根因
- 每完成 Day N 的模块
- 用户表达"原来如此"、"搞懂了"等顿悟时刻
- 做了架构级决策（选编译器、重构控件、切换 API）

## 3. 项目更改追踪 (Project Changes)
**触发条件**：讨论出一段需要修改、重构的代码方案但未立即执行，或者是接下来的任务（或输入 `@save_todo`）。
**执行动作**：
1. 提取 `filePath` (需要修改的文件) 和 `goal` (一句话描述要做什么)。
2. 通过命令行执行：
   `node .claude/assistant-tools.js '{"action":"save_todo","data":{"filePath":"...","goal":"..."}}'`

## 4. 快捷指令 (Shortcut Commands)
- 如果我输入 `@save_kb` 或者直接说 `保存知识库`：请立即将我们刚才讨论的知识点总结并写入 `docs/knowledge_base.md`。
- 如果我输入 `@save_todo` 或者直接说 `保存待办`：请立即将刚才确认的修改方案提炼为任务，写入 `docs/todo_changes.md`。

## 5. 交接模式 (Handoff Workflow)
**触发条件**：我说"保存交接"、"写交接"、"@handoff"、"handoff"；或者你察觉到当前上下文已经很长、任务尚未完成时主动建议。
**执行动作**：
1. 收集以下 5 个字段：
   - `goal`：我们正在试图达成的目标（一句话）
   - `files`：当前正在处理的文件列表（数组）
   - `changes`：本轮对话已经完成的修改（数组）
   - `deadends`：尝试过但失败了的方案 + 原因（数组）
   - `nextSteps`：下一步即将尝试的操作（数组）
2. 通过命令行执行：
   `node .claude/assistant-tools.js '{"action":"save_handoff","data":{"goal":"...","files":["..."],"changes":["..."],"deadends":["..."],"nextSteps":["..."]}}'`
3. 告知我交接文件已保存，我可以安全结束本次对话。

**读取交接**：在新对话开始时，如果我让你"读取交接"、"继续上次"、"@handoff_load"，请先读取 `handoff.md`，然后基于其中的内容继续工作。

## 6. 每日进度追踪 (Daily Progress Tracking)
**触发条件**：每次对话开始时；或我说"今日计划"、"每日报告"、"今日打卡"、"今日完成"。
**执行动作**：
- 显示今日计划/打卡：静默执行 `python .claude/daily_todo.py` 并展示仪表盘。
- 标记今日完成：静默执行 `python .claude/daily_todo.py done` 并展示结果 + 日报摘要。
- 仅对照 C_LMS：静默执行 `python .claude/daily_todo.py check`。
**注意**：`D:\MyProject\C_LMS` 是交付区，以该目录下实际存在的 .cpp/.h 文件作为"完成"的唯一证据。