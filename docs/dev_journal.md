# LMS 物流管理系统 — 开发历程

> **周期**: 2026-05-13 ~ 至今
> **角色**: 独立全栈（需求分析 + 架构设计 + C 语言编码 + GUI 控件系统 + 编译器工具链 + VS Code 环境配置）
> **技术栈**: C语言, EasyX 图形库, MSYS2 g++ 15.2.0, VS Code, Dev-C++ (旧版)

## 项目概述

LMS 是一个基于 C 语言和 EasyX 图形库的物流管理系统，包含 5 种角色（管理员/客服/仓储员/调度员/客户）和 10 个业务模块。项目最初在 Dev-C++ (g++ 4.9.2) 上开发，后迁移到 VS Code + MSYS2 g++ 15.2.0 现代化工具链。GUI 控件系统（WINDOW_T/CONTROL_T）从零自主设计，包含按钮、文本输入框、密码框、标签等控件类型，支持键盘导航和事件驱动。

---

## 技术挑战日志

### 挑战 1: VS Code 无法编译运行 C++ 文件

- **现象**: VS Code 打开项目后，按 F5/F6 编译按钮毫无反应，也没有错误提示
- **排查路径**: 首先检查 g++ 是否存在 → 确认 Dev-C++ 自带 g++ 4.9.2 在 PATH 中能工作 → 怀疑 VS Code 缺少构建配置 → 发现项目缺少 `tasks.json` → 创建后报 `undefined reference` → 意识到这是多文件项目（需链接 6 个 .cpp + EasyX 库）→ C/C++ Runner 扩展只支持单文件编译 → 最终结论：需要完整的构建任务配置
- **根因**: 
  1. 缺少 `.vscode/tasks.json`（VS Code 不知道用什么命令编译）
  2. 项目是 6 个 .cpp 文件 + EasyX 图形库的多文件项目，不能单文件编译
  3. C/C++ Compile Run 扩展 (`danielpinto8zz6.c-cpp-compile-run`) 只能编译单个文件
- **解决方案**:
  1. 创建 `tasks.json` 定义完整构建命令，包含所有 .cpp 文件和 EasyX 链接参数
  2. 配置 Code Runner 扩展的 `executorMap`，让 Ctrl+Alt+N 也能执行完整编译
  3. 用 `Ctrl+Shift+B`（Build Task）替代播放按钮
- **关键代码**:
```json
"command": "g++.exe",
"args": ["main.cpp", "common.cpp", "control.cpp", "startWin.cpp", "loginWin.cpp", "mainWin.cpp",
         "-o", "output/LMS.exe", "-I", "...include", "-L", "...lib", "-leasyx", "-static-libgcc"]
```
- 💡 **面试要点**: VS Code 的构建系统需要显式配置 `tasks.json` 才能编译 C++ 项目；理解项目结构（多文件链接 + 静态库依赖）是配置任何 IDE 的前提，这个排查思路适用于 CMake/XMake 等构建工具

---

### 挑战 2: 中文显示全是"锟斤拷"

- **现象**: 程序编译运行后，所有中文界面文字全部变成乱码（锟斤拷、烫烫烫），但程序逻辑正常
- **排查路径**: 注意到 VS Code 右下角显示文件编码为 `UTF-8` → 检查原 Dev-C++ 项目发现源文件是 `GBK` 编码 → VS Code 编辑时自动转换成了 UTF-8 → 老 g++ 4.9.2 按系统默认 GBK 读取 UTF-8 字节 → 每个中文字符的 UTF-8 多字节序列被当作多个 GBK 字符 → 产生乱码
- **根因**: 源文件编码 (UTF-8) 与编译器期望的输入编码 (GBK，g++ 4.9.2 在中文 Windows 上的默认值) 不匹配，且运行时 Windows GDI 也期望 GBK
- **解决方案**: 在编译命令中添加两个标志：
  - `-finput-charset=UTF-8`：告诉编译器源文件是 UTF-8
  - `-fexec-charset=GBK`：告诉编译器把字符串字面量转成 GBK 写入 .exe
- **关键代码**: `g++ -finput-charset=UTF-8 -fexec-charset=GBK ...`
- 💡 **面试要点**: C/C++ 编译器对不同编码的处理涉及两个独立步骤——输入编码（源文件）和执行编码（二进制中的字符串字面量）；在中文 Windows 上开发跨编码项目时，需要用编译器标志显式指定，而不是依赖默认行为

---

### 挑战 3: mainWin.cpp 编译报 5 处 "invalid conversion from char* to char"

- **现象**: 新增的 `mainWin.cpp` 编译报 5 处同类型错误，都指向 WINDOW_T 初始化的大括号 `};` 行
- **排查路径**: 错误信息说 `invalid conversion from 'char*' to 'char'` → 检查 WINDOW_T 和 CONTROL_T 结构体定义 → CONTROL_T 中 `text` 字段是 `char text[100]`（固定数组）→ mainWin.cpp 中用了 `char title[128]` 变量来初始化它 → C++14 中，`char[]` 变量在 brace initializer 中退化为 `char*`，无法用于初始化 `char[100]` → 但 startWin.cpp 里用字面量 `"1-登录"` 是可以的，因为 `const char[N]` 可以直接初始化 `char[]`
- **根因**: C++ 中，brace 初始化 `char[]` 成员时，只有字符串字面量（`const char[N]`）可以直接初始化，而 `char[]` 变量退化为指针后不允许
- **解决方案**: 在 brace init 中用空字符串 `""` 占位，然后在 `window_show()` 之前用 `strcpy(win.controls[0].text, title)` 动态赋值。5 个管理菜单窗口都需要这样修改
- **关键代码**:
```c
{230, 100, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0},  // 占位
strcpy(win.controls[0].text, title);  // 动态赋值
```
- 💡 **面试要点**: C 和 C++ 在聚合初始化上的差异——C 允许用 `char[]` 变量初始化同类型数组，但 C++ 不允许，因为 C++ 的 brace init 语义更严格。理解这种差异能避免跨语言移植时的隐蔽 bug

---

### 挑战 4: 图形窗口在 VS Code 终端下键盘无响应

- **现象**: 程序编译运行后，EasyX 图形窗口正常显示，但键盘完全无法操作——方向键无反应、回车无效、无法输入文字
- **排查路径**: 先怀疑是 `getch()` 的问题 → 在代码中加 `printf` 调试发现 `getch()` 根本没有读到按键 → 怀疑 CRT 的 `getch()` 读的是控制台而非图形窗口 → 查 EasyX 文档发现它有自己独立的 `getmessage()` API → 将事件循环从 CRT `getch()` 重构为 EasyX `getmessage(EX_KEY | EX_CHAR)` → 编译后还是不行 → 进一步排查发现 VS Code 的集成终端捕获了键盘焦点 → 图形窗口在 VS Code 窗口后面，不是前台窗口 → Windows 只把键盘消息发给前台激活窗口
- **根因**: 两层问题——(1) 输入源错误：CRT `getch()` 读控制台，不是图形窗口；(2) 焦点问题：图形窗口没有获得前台焦点
- **解决方案**:
  1. 将 `window_run()` 完全重写，用 EasyX 的 `getmessage(EX_KEY | EX_CHAR)` 替代 CRT `getch()`
  2. 映射全部键盘操作：`VK_RETURN` 回车、`VK_TAB` 切换密码可见性、`VK_BACK` 退格、`VK_UP`/`VK_DOWN` 控件导航
  3. `initgraph` 后加 `SetForegroundWindow(GetHWnd())` 强制图形窗口置顶抢焦点
- **关键代码**:
```c
SetForegroundWindow(GetHWnd());  // 抢焦点
ExMessage msg = getmessage(EX_KEY | EX_CHAR);
if (msg.vkcode == VK_UP) { /* 上键导航 */ }
```
- 💡 **面试要点**: Windows GUI 程序的消息分发机制——键盘/鼠标事件由系统消息队列分发给前台激活窗口，而非"最近创建的窗口"。`SetForegroundWindow` 是 Win32 API 中显式抢焦点的函数。EasyX 的 `getmessage()` 内部封装了 Windows 消息循环，在图形编程中应该用它而不是 CRT 的控制台 I/O

---

### 挑战 5: MSYS2 g++ 15.2.0 编译配置（4 层问题递进排查）

- **现象**: 将编译器从 Dev-C++ (g++ 4.9.2) 切换到 MSYS2 (g++ 15.2.0) 后，VS Code 编译一直报错
- **排查路径**:
  1. **第 1 层**：编译命令中引用了 Dev-C++ 4.9.2 的旧 intrinsic 头文件 → g++ 15.2.0 的内建函数签名已变 → 报 `__builtin_ia32_pbroadcastq512_mem_mask` 未声明 → 删除旧 include 路径，只保留 EasyX 头目录
  2. **第 2 层**：采用 `"type": "shell"` 任务模式 → cmd.exe 把 `C:\Program Files (x86)` 中的括号 `()` 当作命令分组符 → 路径被截断 → 报"系统找不到指定的路径" → 改用 8.3 短路径名 `C:\PROGRA~2`
  3. **第 3 层**：改用 `"type": "process"` 模式绕过 shell → 进程能启动但退出码 1 且无任何错误输出 → 怀疑 MSYS2 g++ 的 DLL 依赖找不到 → 运行 `ntldd` 确认 g++.exe 依赖 `mingw64\bin` 下的运行时 DLL
  4. **第 4 层**：添加 `"options": { "env": { "PATH": "C:\\msys64\\mingw64\\bin;${env:PATH}" } }` → g++ 找到所有依赖 → 编译成功
- **根因**: 四层独立问题叠加——(1) 旧头文件 ABI 不兼容 (2) cmd.exe 括号解析 (3) shell 引号转义 (4) MSYS2 运行时 DLL 不在系统 PATH
- **解决方案**:
```json
{
    "type": "process",
    "command": "C:\\msys64\\mingw64\\bin\\g++.exe",
    "args": ["-I", "C:\\PROGRA~2\\Dev-Cpp\\MinGW64\\include", ...],
    "options": {
        "env": {
            "PATH": "C:\\msys64\\mingw64\\bin;${env:PATH}"
        }
    }
}
```
- 💡 **面试要点**: GCC 编译器升级时，不能混用旧版本的系统头文件（intrinsic 定义不兼容）。Windows cmd.exe 中括号 `()` 是保留字符，即使在引号内也可能被解析——用 8.3 短路径名是最简单的绕行方案。MSYS2 的 MinGW 编译器依赖自己的运行时 DLL，必须在 PATH 中能访问到

---

## 技术栈总结

| 层面 | 选型 | 理由 |
|------|------|------|
| 语言 | C (C99) | Dev-C++ 原始项目为 C，保持兼容 |
| 图形库 | EasyX (Dev-C++ 版) | 项目原始依赖，提供 GDI 封装 |
| 编译器 | MSYS2 g++ 15.2.0 | 从 Dev-C++ 4.9.2 升级，支持 C++17 |
| IDE | VS Code | 替代 Dev-C++，更好的代码编辑和调试 |
| 构建 | VS Code tasks.json (process 模式) | 直接启动进程，绕过 shell 转义问题 |
| 版本控制 | Git (本地) | 简历项目需要展示规范管理 |
| AI 辅助 | Claude Code + 自定义 Skills | 界面生成、学习日志、开发历程导出 |

---

> 最后更新: 2026-05-17
> 本文档由 `@journal` / `dev-journal` skill 自动生成
