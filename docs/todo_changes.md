
- [ ] **文件路径**：include/ui/ (新建EditBox/PasswordBox控件)
  **修改目标**：GUI输入框控件复用common.h中input_password()的Tab键切换明文/密文显示逻辑，将Console模式下的密码输入机制合并到GUI控件中，新建EditBox和PasswordBox类，主题色已在uictrl.h中预留

- [ ] **文件路径**：LMS/ (新建备份文件夹)
  **修改目标**：Day3工作流迭代：在LMS中新建文件夹存放C_LMS代码备份，实现简陋的并行代码版本管理。Vibe coding在此文件夹展开。C_LMS严格只读（仅允许用户本人编写，特殊情况下经允许可加注释）。

- [ ] **文件路径**：LMS/ (编码适配)
  **修改目标**：Day3工作流迭代：解决C_LMS（Dev-C++，推测GBK/GB2312）与Claude写入（UTF-8）之间的中文编码不一致问题，避免用户在Dev-C++中看到乱码。

- [ ] **文件路径**：C_LMS/common.cpp
  **修改目标**：[Day1未完成] 补全 bin_load_list()：分配内存、逐条 fread 记录、链表拼接、fclose + return count

- [ ] **文件路径**：C_LMS/common.cpp
  **修改目标**：[Day1未完成] 实现 show_paginated_list()：当前只有 TODO 注释和 return 0

- [ ] **文件路径**：C_LMS/control.cpp
  **修改目标**：[Day2未完成] 补全 window_run() 键盘输入：回车确认、字母数字输入、退格删除（当前只有空壳和箭头导航）

- [ ] **文件路径**：C_LMS/control.cpp
  **修改目标**：[Day2未完成] control_show() 的 EDIT_PWD 分支嵌入 common.h 的密码输入机制（Tab切换明文/密文），当前只做了静态*遮罩
