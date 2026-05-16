---
name: git-project
description: Initialize git repository, generate .gitignore and README, and create smart commit messages. Use when the user says @git, 提交一下, gitignore, or needs repo setup.
---

# Git Project Manager

Manage git repository setup, .gitignore, README, and smart commits for the current project.

## .gitignore Generator

### C++ / EasyX / VS Code Project Template
```gitignore
# 编译产物
*.exe
*.o
*.obj
*.out
backup/output/

# IDE 配置
.vscode/

# 系统垃圾文件
Thumbs.db
Desktop.ini
~$*
*.tmp

# 大型二进制资源（非源码）
*.pdf
*.xlsx
*.doc
*.docx

# 本地数据备份
*.bat
backup/data/*.bat

# 项目交付区（只读源）
C_LMS/
```

**Rules for customizing:**
- If project uses `output/` directory → add it
- If project has image assets that ARE source → `!image/*.png` to unignore
- If project uses CMake → add `build/`, `CMakeCache.txt`
- If project uses Makefile → add `*.d`, keep `Makefile`

## README Generator

### Template
```markdown
# [项目名]

[一句话描述，从 CLAUDE.md 提取 goal]

## 技术栈

- 语言: C (C99/C11)
- 图形库: EasyX
- 编译器: MSYS2 MinGW g++ 15.2.0
- IDE: VS Code

## 快速开始

### 编译
\`\`\`bash
g++ -finput-charset=UTF-8 -fexec-charset=GBK \
    backup/main.cpp backup/common.cpp backup/view/control.cpp \
    backup/app/startWin.cpp backup/app/loginWin.cpp backup/mainWin.cpp \
    -o backup/output/LMS.exe \
    -I backup -I backup/view -I backup/app \
    -I "C:/Program Files (x86)/Dev-Cpp/MinGW64/include" \
    -L "C:/Program Files (x86)/Dev-Cpp/MinGW64/lib" \
    -leasyx -static-libgcc -static-libstdc++
\`\`\`

### 运行
\`\`\`bash
./backup/output/LMS.exe
\`\`\`

## 项目结构
\`\`\`
LMS/
├── backup/                # 源代码
│   ├── main.cpp           # 入口
│   ├── common.cpp/.h      # 数据结构和工具
│   ├── mainWin.cpp/.h     # 主菜单+角色窗口
│   ├── app/               # 应用层窗口
│   │   ├── startWin.cpp   # 启动界面
│   │   └── loginWin.cpp   # 登录界面
│   ├── view/              # UI 控件库
│   │   └── control.cpp/.h # WINDOW_T/CONTROL_T
│   ├── output/            # 编译产物
│   └── image/             # 图片资源
├── docs/                  # 文档
│   ├── knowledge_base.md  # 知识库
│   └── learning_log.md    # 学习日志
├── .claude/               # Claude Code 配置
│   └── skills/            # 项目专属 skills
├── .vscode/               # VS Code 配置
└── CLAUDE.md              # 自动化助理规则
\`\`\`

## 角色与模块

| 角色 | 权限 | 已实现功能 |
|------|------|-----------|
| 管理员 | 全部 | 用户管理、系统导航 |
| 客服 | 订单管理 | 订单受理(待)、订单查询(待) |
| 仓储员 | 仓库操作 | 出入库管理(待) |
| 调度员 | 运输调度 | 调度管理(待) |
| 客户 | 查订单 | 查看订单(待) |

## 进度

- [x] Day 1-4: 基础框架、GUI 控件、登录认证、用户管理
- [ ] Day 5: 订单管理
- [ ] Day 6: 订单审核与查询
- [ ] Day 8: 仓储管理
- [ ] Day 10: 调度管理
\`\`\`
```

## Smart Commit

### Commit Message Format
```
type(scope): 中文简述

- 变更点1
- 变更点2

Co-Authored-By: Claude Code <noreply@anthropic.com>
```

### Type Detection Rules
| 文件/变更 | type |
|-----------|------|
| 新增 .cpp/.h 文件 | `feat` |
| 修改已有 .cpp/.h 逻辑 | `fix` 或 `feat` |
| 修改 .claude/ 配置 | `chore` |
| 修改 docs/ | `docs` |
| 重命名/移动文件 | `refactor` |
| 修改 .vscode/ | `chore` |
| 修改 CLAUDE.md | `chore` |

### Scope
- `DayN` — 对应进度模块
- `ui` — 界面相关
- `build` — 编译配置
- `kb` — 知识库/学习日志
- `repo` — git/项目配置

### Workflow
1. User says `@git` or "提交一下"
2. Run `git status` and `git diff --stat`
3. Detect changed files, determine `type` and `scope`
4. Draft commit message, show to user
5. User confirms → `git add` + `git commit`

## Git Init Workflow
1. `git init`
2. Generate `.gitignore` based on project type
3. Generate/update `README.md`
4. `git add .gitignore README.md`
5. `git add` all source files (not ignored)
6. First commit: `chore(repo): 初始化项目仓库`
