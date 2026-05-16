---
name: learning-log
description: Record project learning cards when the user solves a bug, completes a module, or encounters a technical insight. Use when the user says @learn, 记录一下, 记个卡片, or after a significant debugging session.
---

# Learning Log

Record every significant technical moment in this project for future review and interview preparation.

## Card Format

```markdown
---
title: "[一句话描述]"
date: YYYY-MM-DD
tags: ["tag1", "tag2"]
---

## 问题
[遇到了什么现象、什么报错]

## 根因
[为什么会发生、底层机制]

## 解决
[具体做了什么、步骤]

## 学到
[提炼一个面试能讲的技术原理，用通俗语言]

## 亮点
[你主导了什么、做了什么决策、有什么独特贡献]

## 代码
\`\`\`c
[最关键的 2-5 行代码]
\`\`\`
```

## Storage

Cards are stored in `docs/learning_log.md` with an index in `docs/learn_index.json`.

### Index Structure
```json
[
  {"title": "EasyX 图形窗口在 VS Code 终端下键盘无响应", "date": "2026-05-16", "tags": ["easyx", "keyboard", "vscode"], "project": "LMS"},
  ...
]
```

### Deduplication
- **Exact title match** → reject with "已经有相关记录了，提醒您去复习"
- **New title** → append to learning_log.md, update index
- **Same tags** → mention related cards for context

## Triggers

### Automatic (I will detect these)
- A bug is resolved after investigation
- A Day/module is completed
- A significant technical decision is made
- User expresses "aha moment" or "原来如此"

### Manual (User says these)
- `@learn` or `记录一下` or `记个卡片` or `写卡片`

### Workflow
1. When triggered, I draft the card content and show it to the user
2. User reviews and says "记" to save, or edits the content
3. I run `node .claude/assistant-tools.js '{"action":"save_learn","data":{...}}'`
4. Report result (success / duplicate)

## Action
To save a card, use:
```
node .claude/assistant-tools.js '{"action":"save_learn","data":{"title":"...","date":"...","tags":["..."],"project":"LMS","problem":"...","rootCause":"...","solution":"...","learned":"...","highlight":"...","code":"..."}}'
```

## Cross-Project Aggregation
Each card has a `"project"` field. After completing multiple projects, cards can be aggregated:
```bash
cat */docs/learning_log.md > all_learning.md
```
All use the same format.
