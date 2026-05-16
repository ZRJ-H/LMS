# Handoff — 2026-05-17

## 🎯 Goal
LMS 项目工具链搭建 + Skills 体系建设 + Day 1-4 基础模块完成

## 📁 关键文件
- `backup/main.cpp` — 入口，已加 SetForegroundWindow
- `backup/view/control.cpp` — GUI 控件库，已重构为 getmessage()
- `backup/mainWin.cpp` — 5 角色菜单 + 用户管理
- `backup/app/loginWin.cpp` — 登录认证
- `.vscode/tasks.json` — MSYS2 g++ 15.2.0 构建任务
- `.vscode/settings.json` — 编码标志 + Code Runner 命令
- `.claude/skills/lms-ui.md` — 界面生成模板
- `.claude/skills/learning-log.md` — 学习卡片记录
- `.claude/skills/dev-journal.md` — 开发历程导出
- `.claude/skills/git-project.md` — Git 管理
- `docs/dev_journal.md` — 5 个技术挑战的面试级叙事

## ✅ 已完成
- 编译器从 Dev-C++ g++ 4.9.2 迁移到 MSYS2 g++ 15.2.0
- tasks.json 配置完成 (type:process + PATH 注入 + PROGRA~2)
- window_run() 从 CRT getch() 重构为 EasyX getmessage()
- main.cpp 添加 SetForegroundWindow 抢焦点
- mainWin.cpp 修复 5 处数组初始化编译错误
- control.cpp 修复 window_show 缺少 return
- UTF-8/GBK 编码配置完成
- Git 仓库初始化 + .gitignore + 3 次提交
- 4 个专属 Skills 创建完成
- 全局知识库和全局 CLAUDE.md 配置完成

## ❌ 死胡同
- C/C++ Compile Run 单文件扩展无法编译多文件项目
- tasks.json type:shell 导致 cmd.exe 括号/引号问题
- tasks.json 缺少 PATH 时 MSYS2 g++ DLL 找不到
- CRT getch() 在 VS Code 终端下读不到图形窗口输入

## 🔜 下一步
1. Day 5: 订单管理模块 (客户下单 + admin 订单列表)
2. Day 6: 订单审核 + 订单追踪查询
3. Day 8: 仓储出入库管理
4. Day 10: 调度管理
5. 分页列表从 console 改为 GUI 版

## 💡 备忘
- 编译器: `C:\msys64\mingw64\bin\g++.exe`
- EasyX: `C:\PROGRA~2\Dev-Cpp\MinGW64`
- 编译: Ctrl+Shift+B (Build) / Ctrl+Alt+N (Run)
