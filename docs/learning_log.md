
---
title: "EasyX 图形窗口在 VS Code 终端下键盘无响应"
date: 2026-05-16
tags: [easyx, keyboard, vscode]
project: LMS
---

## 问题
EasyX 图形窗口显示正常但键盘完全不响应，方向键和回车都无效

## 根因
VS Code 集成终端捕获了键盘焦点，EasyX 窗口未获得前台焦点。更深层：CRT getch() 从控制台读输入，EasyX getmessage() 从图形窗口消息队列读

## 解决
1. 事件循环从 CRT getch() 重构为 EasyX getmessage(EX_KEY | EX_CHAR)
2. initgraph 后加 SetForegroundWindow(GetHWnd()) 强制置顶

## 学到
Windows GUI 消息分发机制：键盘事件发送到前台激活窗口，而非最近创建的窗口。SetForegroundWindow 是 Win32 API 中显式抢焦点的函数

## 亮点
独立排查出 CRT getch() vs EasyX getmessage() 的根因差异，并完成完整事件循环重构

## 代码
```c
SetForegroundWindow(GetHWnd());  // initgraph 之后
msg = getmessage(EX_KEY | EX_CHAR);  // 替代 getch()
```

---
title: "MSYS2 g++ 15.2.0 编译 EasyX 项目配置"
date: 2026-05-17
tags: [easyx, compiler, vscode]
project: LMS
---

## 问题
Dev-C++ g++ 4.9.2 太老，需要换 MSYS2 g++ 15.2.0 但编译失败

## 根因
1. cmd.exe 误解 Program Files (x86) 中的括号；2. MSYS2 g++ 依赖 mingw64in 下的 DLL；3. 旧 Dev-C++ intrinsic 头不兼容新编译器

## 解决
1. 用 PROGRA~2 短路径；2. tasks.json 加 PATH 环境变量；3. 用 type:process 而不是 shell

## 学到
Windows cmd.exe 的参数解析规则：括号是命令分组符，即使在引号内。MSYS2 的 MinGW 编译器需要自己的运行时 DLL。VS Code tasks 用 process 模式可绕过 shell。

## 亮点
排查了 4 层问题（路径括号/PATH/process/短路径），成功把 2014 年编译器切换到 2025 年

## 代码
```c
"type": "process",
"options": { "env": { "PATH": "C:\msys64\mingw64\bin;${env:PATH}" } }
```
