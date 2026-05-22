# 物流管理系统（LMS）

## 项目结构

```
LMS/
├── include/
│   └── common.h              # 全局契约：结构体、枚举、宏、extern 声明
├── src/
│   ├── common.c              # 公共工具实现：时间、密码、分页、二进制读写
│   ├── main.c                # 入口 + 主菜单调度 + 启动/退出数据加载
│   ├── sys_manage.c          # 系统管理：登录、角色、配置
│   ├── order_manage.c        # 订单管理：创建、审核、查询、跟踪
│   ├── warehouse_manage.c    # 仓储管理：入库、出库、盘点
│   ├── transport_manage.c    # 运输管理：调度、轨迹跟踪
│   └── stat_manage.c         # 统计分析：报表计算与 TXT 导出
└── data/                     # 运行时自动生成，存放 .dat 二进制数据文件
```

## 核心设计决策

### 1. "next 作为第一个字段"的链表约定

每个结构体的第一个成员都是 `struct Xxx *next`。这不是巧合，而是有意为之：

```c
typedef struct Goods {
    struct Goods *next;   // <-- 始终在偏移 0
    int   id;
    char  name[32];
    ...
} Goods;
```

**为什么？** C 语言保证：**指向结构体的指针，等于指向其第一个成员的指针**。这意味着：

```c
Goods *goods = ...;
// goods == &goods->next，两者地址相同
// (void*)goods 解引用第一个字段，恰好就是 next 指针
```

这条约定让 `bin_save_list`、`bin_load_list`、`show_paginated_list`、`list_count` 四个通用函数**不需要知道自己在操作 Goods 还是 Order**。它们只需要知道两件事：首地址和结构体大小。如果 `next` 不在偏移 0，每个模块都得写自己的遍历循环——16 个结构体就是 16 份几乎一模一样的代码。

> **代价**：每个结构体多了 8 字节（64 位指针），且访问链表时多一次解引用。但在 2 万条规模下，这点开销无关紧要——换来的是一份代码服务所有模块。

### 2. 通用二进制文件读写

```c
int bin_save_list(const char *filename, const void *head,
                  size_t record_size, size_t next_offset);

int bin_load_list(const char *filename, void **head,
                  size_t record_size, size_t next_offset);
```

**文件格式**：`[4 字节: 记录数 N] [record_size × N]`

没有序列化框架，没有 schema 描述，就是直接把内存中的 `struct` 字节搬进文件。这不是偷懒——对于这个系统，它是正确选择：

| 对比维度 | 二进制直接写入 | JSON/XML | 文本 CSV | SQLite |
|---------|-------------|---------|---------|--------|
| 代码复杂度 | 1 个通用函数，0 行解析代码 | 需引入 cJSON，每个结构体手写序列化 | 需手写格式化和 printf/scanf 解析 | 需链接 sqlite3.dll |
| 加载 2 万条 | 一次 fread，O(n) | 逐条解析 JSON，O(n) 但常数大 | 逐行 scanf，慢且脆弱 | SQL 查询，最灵活 |
| 人工可调试 | 需 hexdump | 直接用记事本打开 | 可以用记事本 | 需 sqlite3 CLI |
| 空间效率 | 最高，无冗余 | 中，有 key 和括号 | 低，数字转文本 | 高 |

**为什么要记录数放在文件头？** 加载时一次 `fread(&count, ...)` 就知道要 malloc 多少次，可以提前分配连续内存块，也可以在一次循环中精确读 N 条后停止——不会读到文件尾才停下，也不会被恶意文件里残留的垃圾字节糊弄。

**为什么不在保存时把 next 指针置零？** 在 `bin_load_list` 中，每条记录读完立刻用 `*(void**)(node + next_offset) = NULL` 覆盖刚从文件中读到的垃圾指针，然后链到链表尾巴上。保存时额外清零是徒劳的——反正加载时会覆盖。

### 3. 权限位掩码

```c
#define PERM_USER_MANAGE    0x0001
#define PERM_ORDER_MANAGE   0x0002
#define PERM_ORDER_AUDIT    0x0004
#define PERM_WAREHOUSE      0x0008
...
#define ROLE_PERM_ADMIN     0xFFFF
#define ROLE_PERM_SERVICE   (PERM_ORDER_MANAGE)
```

**为什么不用角色字符串比较？** 每个菜单入口写 `if (role == ROLE_ADMIN || role == ROLE_SERVICE)` 是脆弱的——新增一个角色就要改散落在各处的条件判断。位掩码让判断变成 `if (perm & PERM_XXX)`，一个整数一次位运算，O(1) 且不随角色数量增长。

**扩展性**：如果未来需要"某个仓储员额外拥有订单查看权限"，只需给那个 User 记录加一个 `custom_perm` 字段，实际权限 = `role_get_permissions(role) | custom_perm`。

### 4. 内存优先，退出时持久化

程序运行时，所有数据在 RAM 中以链表形式存在。用户退出时才调用 `bin_save_list` 把整个链表写回 `.dat` 文件。

**为什么不实时写磁盘？**

- 控制台程序是单用户单线程的，不存在并发冲突
- 链表天然支持频繁插入/删除，而每次都写文件需要先遍历找位置再全量重写，I/O 开销远超操作本身
- 这种模式下，一次"修改订单状态"就是修改链表节点的 `status` 字段——纯内存操作，微秒级

**代价**：如果程序崩溃，本次修改丢失。对这个系统而言，真正的验收场景是演示操作流程，而非 7×24 高可用。如果未来需要持久化保证，可以在关键操作（审核、入库）后加一行 `bin_save_list(...)` 完成即时落盘。

### 5. 枚举驱动，字符串转换统一出口

```c
OrderStatus status = ORDER_PENDING_REVIEW;
printf("%s", order_status_to_string(status));  // 输出 "待审核"
```

**为什么不直接在代码里写中文字符串？**

- 如果要支持英文版，只需替换 5 个 `*_to_string()` 函数
- 编译器可以检查 `switch` 是否覆盖了所有枚举值（`-Wall` 会警告遗漏的 case）
- 打印格式统一：不会出现某处写了"已送达"、另一处写了"已送到"这种不一致

### 6. 分页显示的 Callback 模式

```c
typedef void (*PrintRowFn)(const void *record, int index);

int show_paginated_list(const void *head, size_t next_offset,
                        PrintRowFn print_row, int page_size);
```

分页逻辑（翻页、计数、跳页）和显示逻辑（这个结构体有哪些字段、怎么排版）被彻底分离。每个模块只写一个"怎么打印本模块的一行"的函数，传给分页引擎即可。换分页键位（比如改成 ← → 翻页）只需改一处。

## 后续延伸空间

### 短期（学期内）

- **MD5 密码加密**：当前密码以明文存储和比较（`verify_password` 就是 `strcmp`）。后续可在 `common.c` 中添加 `md5_hash(const char *input, char *output)`，修改 `User` 的 `password` 字段存放 32 位 MD5 摘要而非明文
- **数据备份与恢复**：`bin_save_list` 已经能写文件，备份功能只需把 `data/*.dat` 整体复制到备份目录；恢复就是反向复制。可以利用 `system("copy ...")` 或用 C 标准库逐文件复制
- **键盘快捷键导航**：在 `show_paginated_list` 中增加数字键直接跳页，或在主菜单中使用 `↑↓ Enter` 替代 `1/2/3` 数字选择

### 中期（课程设计后续学期）

- **替换为 SQLite**：将 `bin_load_list` 替换为 `SELECT * FROM xxx`，`bin_save_list` 替换为 `INSERT/UPDATE`。得益于所有数据操作已经收敛在通用函数中，切换存储层不影响任何业务模块——只需修改 `common.c`
- **网络化改造**：将链表操作抽成 Server 端，增加 `ctrl_*` 与 `svc_*` 之间的 TCP 通信层。当前的单链表天然适合转为 Server 端内存缓存
- **并发支持**：链表 → 哈希表 + 读写锁。当前的单链表查找是 O(n)，在 2 万条下可以接受（~0.1ms），但如果是 200 万条就需要索引结构

### 长期（参照真实系统）

- **Web 化**：C 后端 + MySQL + 前端页面。当前枚举和结构体定义可以直接翻译为数据库 DDL 和后端 Model 定义
- **实时轨迹跟踪**：`TransportTracking` 结构体已预留了 `node_status` 和 `update_time` 字段，对接 GPS 设备只需增加经纬度字段和推送机制
- **机器学习预测**：利用 `OrderStatResult` 的历史数据训练模型预测运输时效，统计结构体就是特征工程的起点

## 构建

```bash
gcc -Wall -I include -o bin/lms.exe src/main.c src/common.c src/sys_manage.c \
    src/order_manage.c src/warehouse_manage.c src/transport_manage.c \
    src/stat_manage.c
```
