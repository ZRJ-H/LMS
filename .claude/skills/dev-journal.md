---
name: dev-journal
description: Synthesize learning cards, git history, and conversation context into a developer journey document suitable for portfolio, resume, or interview preparation. Use when the user says @journal, 写开发日志, or 导出开发历程.
---

# Developer Journal Generator

Read all available project context and synthesize a narrative developer journey document.

## Data Sources

| Source | What to extract |
|--------|----------------|
| `docs/learning_log.md` | Bug investigations, root causes, solutions |
| `git log --oneline` | Commit history, file changes over time |
| `CLAUDE.md` | Project goals, technical constraints |
| `docs/knowledge_base.md` | Technical concepts learned |
| Handoff files | In-progress work, dead ends attempted |
| Current conversation | Decisions not yet recorded in cards |

## Output

Write to `docs/dev_journal.md`:

```markdown
# [项目名] — 开发历程

> **周期**: YYYY-MM-DD ~ 至今
> **角色**: 独立开发（需求 + 架构 + 编码 + 工具链）
> **技术栈**: [语言], [库], [编译器], [IDE]

## 项目概述
[2-3 句话描述项目是什么、为什么做、技术选型理由]

## 技术挑战日志

### 挑战 1: [一句话标题]
- **现象**: [遇到了什么]
- **排查路径**: [我尝试了 A → 发现不行 → 试了 B → 还是不行 → 定位到 C]
- **根因**: [为什么会发生]
- **解决方案**: [分步骤说明]
- **关键代码**: `[2-3 行]`
- 💡 **面试要点**: [一句话讲清原理 + 展示什么能力]

### 挑战 2: ...
```

## Processing Rules

1. **Chronological order**: sort challenges by date
2. **Fill gaps**: if learning card has "problem" and "root cause" but not the investigation path, infer it from the conversation context
3. **Interview focus**: each challenge MUST have a "面试要点" that is a self-contained 1-2 sentence explanation suitable for interview response
4. **Ownership language**: use "我" (I), not passive voice. "我排查了..." not "问题被排查..."
5. **Technical depth**: include specific API names, compiler flags, error messages — avoid vague descriptions
6. **Update mode**: on subsequent runs, append new challenges at the top, don't regenerate the whole file

## Trigger
- `@journal` or "写开发日志" or "导出开发历程"
- After completing a Day module
