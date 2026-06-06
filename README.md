# 物流管理系统（LMS）

基于 C++、Win32 和 EasyX 的桌面物流管理系统，包含用户、订单、仓储、运输、统计和数据备份等功能。

## 源码结构

```text
backup/
├── app/       窗口与界面逻辑
├── public/    公共数据结构与工具
├── service/   业务服务
├── view/      通用控件
├── main.cpp   程序入口
└── Makefile   MinGW 构建文件
```

仓库仅保留运行所需源码和构建文件，不包含 IDE 缓存、可执行文件、运行数据或本地开发记录。

## 构建环境

- Windows
- MinGW-w64（g++）
- EasyX

`backup/Makefile` 默认使用以下本地路径：

```text
C:/enviroment/mingw64/bin/g++.exe
C:/PROGRA~2/Dev-Cpp/MinGW64/include
C:/PROGRA~2/Dev-Cpp/MinGW64/lib
```

如果安装位置不同，请修改 Makefile 中的 `CXX`、`EASYX_INC` 和 `EASYX_LIB`。

## 编译

在 PowerShell 中执行：

```powershell
cd backup
C:\enviroment\mingw64\bin\mingw32-make.exe
```

生成文件：

```text
backup/output/LMS.exe
```

程序首次运行会自动创建本地数据。背景图片为可选资源；未提供 `image/bg.bmp` 时不影响核心业务功能。
