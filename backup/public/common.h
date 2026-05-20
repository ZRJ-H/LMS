#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
/* ============================================================
 * 字符串长度约定
 * ============================================================ */
#define NAME_LEN        32      /* 姓名 */
#define ADDR_LEN       128      /* 地址 */
#define PHONE_LEN       16      /* 电话号码 */
#define ID_CARD_LEN     20      /* 身份证号 */
#define PASSWORD_LEN    33      /* MD5 哈希 32 位 + \0 */
#define ORDER_ID_LEN    24      /* 订单号: WL + YYYYMMDD + 6位序号 + \0 */
#define DISPATCH_ID_LEN 24      /* 调度单号 */
#define GOODS_TYPE_LEN  32      /* 货物类型名称 */
#define LOCATION_LEN    16      /* 货位编号: A01, B02... */
#define PLATE_LEN       12      /* 车牌号 */
#define REASON_LEN     256      /* 驳回/异常原因 */
#define ACTION_LEN     256      /* 操作日志内容 */


/* ============================================================
 * 权限位掩码 + 角色 — 菜单可见性
 * 用整数的每个 bit 代表一种权限（0x0001=用户管理, 0x0002=订单管理...），
 * 角色 = 预设的 bit 组合。菜单渲染只需一次位与运算 if (perm & PERM_XXX)，
 * O(1) 且新增角色不影响现有判断逻辑。
 * ============================================================ */
#define PERM_USER_MANAGE    0x0001  /* 用户管理 */
#define PERM_ORDER_MANAGE   0x0002  /* 订单管理（创建、查询） */
#define PERM_ORDER_AUDIT    0x0004  /* 订单审核 */
#define PERM_WAREHOUSE      0x0008  /* 仓储管理（出入库、盘点） */
#define PERM_TRANSPORT      0x0010  /* 运输调度与跟踪 */
#define PERM_STATISTICS     0x0020  /* 统计分析 */
#define PERM_SYSTEM_CONFIG  0x0040  /* 系统配置、备份恢复 */

/* 角色默认权限组合 */
#define ROLE_PERM_ADMIN      0xFFFF
#define ROLE_PERM_SERVICE    (PERM_ORDER_MANAGE)
#define ROLE_PERM_WAREHOUSE  (PERM_WAREHOUSE)
#define ROLE_PERM_DISPATCHER (PERM_TRANSPORT)
#define ROLE_PERM_CUSTOMER   (PERM_ORDER_MANAGE)

/* 用枚举来管理状态机 */

/* ============================================================
 * 角色枚举
 * ============================================================ */
typedef enum {
    ROLE_ADMIN      = 0,    /* 管理员 */
    ROLE_SERVICE    = 1,    /* 客服 */
    ROLE_WAREHOUSE  = 2,    /* 仓储员 */
    ROLE_DISPATCHER = 3,    /* 调度员 */
    ROLE_CUSTOMER   = 4     /* 客户 */
} UserRole;

/* ============================================================
 * 订单状态枚举
 * ============================================================ */
typedef enum {
    ORDER_PENDING_REVIEW = 0,   /* 待审核 */
    ORDER_REJECTED       = 1,   /* 已驳回 */
    ORDER_PENDING_OUT    = 2,   /* 待出库（审核通过） */
    ORDER_IN_TRANSIT     = 3,   /* 运输中 */
    ORDER_DELIVERED      = 4,   /* 已送达 */
    ORDER_COMPLETED      = 5    /* 已完成 */
} OrderStatus;

/* ============================================================
 * 货物类型枚举
 * ============================================================ */
typedef enum {
    GOODS_NORMAL     = 0,  /* 普通 */
    GOODS_FRAGILE    = 1,  /* 易碎 */
    GOODS_COLD_CHAIN = 2,  /* 冷链 */
    GOODS_DANGEROUS  = 3   /* 危险品 */
} GoodsType;

/* ============================================================
 * 出入库操作类型
 * ============================================================ */
typedef enum {
    OP_INBOUND  = 1,    /* 入库 */
    OP_OUTBOUND = 2     /* 出库 */
} OperationType;

/* ============================================================
 * 车辆类型
 * ============================================================ */
typedef enum {
    VEHICLE_BOX        = 1,  /* 厢式货车 */
    VEHICLE_COLD_CHAIN = 2   /* 冷链车 */
} VehicleType;

/* ============================================================
 * 车辆状态
 * ============================================================ */
typedef enum {
    VEHICLE_IDLE        = 0,  /* 空闲 */
    VEHICLE_IN_TRANSIT  = 1,  /* 运输中 */
    VEHICLE_MAINTENANCE = 2   /* 维修中 */
} VehicleStatus;

/* ============================================================
 * 司机状态
 * ============================================================ */
typedef enum {
    DRIVER_ON_DUTY  = 1,    /* 在岗 */
    DRIVER_ON_LEAVE = 2     /* 休假 */
} DriverStatus;

/* ============================================================
 * 调度单/运输跟踪 结点状态
 * ============================================================ */
typedef enum {
    DISPATCH_PENDING   = 0,  /* 未执行 */
    DISPATCH_DEPARTED  = 1,  /* 已出发 */
    DISPATCH_TRANSIT   = 2,  /* 中转 */
    DISPATCH_DELIVERED = 3,  /* 已送达 */
    DISPATCH_ABNORMAL  = 4   /* 异常 */
} DispatchStatus;


/* ============================================================
 *                       结构体定义
 * 约定：每个结构体的第一个字段是 struct Xxx *next，
 *       用于通用链表操作和二进制文件读写。
 * ============================================================ */

/* ---- 货物 ---- */
typedef struct Goods {
    struct Goods *next;
    int   id;
    char  name[NAME_LEN];
    char  type[GOODS_TYPE_LEN];
    float weight;
    float volume;
} Goods;

/* ---- 用户/人员 ---- */
typedef struct User {
    struct User *next;
    int      id;
    char     name[NAME_LEN];
    UserRole role;
    char     phone[PHONE_LEN];
    char     password[PASSWORD_LEN];
    int      failed_attempts;    /* 连续登录失败次数 */
    time_t   lockout_until;      /* 锁定到期时间戳，0 表示未锁定 */
} User;

/* ---- 订单 ---- */
typedef struct Order {
    struct Order *next;
    char        order_id[ORDER_ID_LEN];   /* WL + 日期 + 6位序号 */
    char        customer_name[NAME_LEN];
    char        customer_phone[PHONE_LEN];
    char        from_addr[ADDR_LEN];
    char        to_addr[ADDR_LEN];
    char        goods_type[GOODS_TYPE_LEN]; /* 货物类型 */
    char        expected_delivery_time[20]; /* YYYY-MM-DD HH:MM:SS */
    OrderStatus status;
    char        reject_reason[REASON_LEN];  /* 驳回原因 */
} Order;

/* ---- 订单明细 ---- */
typedef struct OrderDetail {
    struct OrderDetail *next;
    int   id;
    char  order_id[ORDER_ID_LEN];   /* 外键 - 关联 Order.order_id */
    int   goods_id;                 /* 外键 - 关联 Goods.id */
    int   quantity;
} OrderDetail;

/* ---- 仓库 ---- */
typedef struct Warehouse {
    struct Warehouse *next;
    int  id;
    char name[NAME_LEN];
    char address[ADDR_LEN];
    int  manager_id;    /* 负责人 User.id */
} Warehouse;

/* ---- 出入库记录 ---- */
typedef struct InOutRecord {
    struct InOutRecord *next;
    int  id;                        /* 流水编号 */
    char order_id[ORDER_ID_LEN];    /* 关联订单号 */
    int  goods_id;                  /* 出入货物id */
    int  quantity;                  /* 出入数量 */
    OperationType op_type;          /* 1=入库, 2=出库 */
    char op_time[20];               /* YYYY-MM-DD HH:MM:SS */
    char location_id[LOCATION_LEN]; /* 货位编号 */
    int  operator_id;               /* 经办人 User.id */
} InOutRecord;

/* ---- 库存明细 ---- */
typedef struct Inventory {
    struct Inventory *next;
    int   id;
    int   goods_id;                /* 外键 - Goods.id */
    int   warehouse_id;            /* 外键 - Warehouse.id */
    int   quantity;                /* 当前库存数量 */
    char  location_id[LOCATION_LEN];
    char  in_time[20];             /* 入库时间 */
} Inventory;

/* ---- 运输线路 ---- */
typedef struct Route {
    struct Route *next;
    int   id;
    char  origin[ADDR_LEN];       /* 起点 */
    char  destination[ADDR_LEN];  /* 终点 */
    float distance;               /* 距离(km) */
    char  estimated_time[20];     /* 预计时效 */
} Route;

/* ---- 车辆 ---- */
typedef struct Vehicle {
    struct Vehicle *next;
    int  id;
    char plate_no[PLATE_LEN];    /* 车牌号 */
    VehicleType type;             /* 1=厢式, 2=冷链 */
    float load_capacity;          /* 载重(t) */
    float volume;                 /* 体积(m3) */
    float capacity;               /* 容积(m3) */
    VehicleStatus status;         /* 0=空闲, 1=运输中, 2=维修中 */
} Vehicle;

/* ---- 司机 ---- */
typedef struct Driver {
    struct Driver *next;
    int  id;
    char name[NAME_LEN];
    char id_card[ID_CARD_LEN];
    char phone[PHONE_LEN];
    DriverStatus status;          /* 1=在岗, 2=休假 */
} Driver;

/* ---- 调度单 ---- */
typedef struct Dispatch {
    struct Dispatch *next;
    char dispatch_id[DISPATCH_ID_LEN]; /* 调度单号（主键） */
    char order_id[ORDER_ID_LEN];       /* 关联订单号 */
    int  vehicle_id;                   /* 分配车辆 */
    int  driver_id;                    /* 分配司机 */
    int  route_id;                     /* 运输路线 */
    char planned_departure[20];        /* 预计出发时间 */
    char planned_arrival[20];          /* 预计到达时间 */
    DispatchStatus status;             /* 0=未执行,1=已出发,2=中转,3=已送达,4=异常 */
} Dispatch;

/* ---- 运输跟踪明细 ---- */
typedef struct TransportTracking {
    struct TransportTracking *next;
    int  track_id;                     /* 轨迹流水号（主键） */
    char dispatch_id[DISPATCH_ID_LEN]; /* 关联调度单号 */
    DispatchStatus node_status;        /* 1=已出发,2=中转,3=已送达,4=异常 */
    char exception_reason[REASON_LEN]; /* 异常原因（仅 node_status==4 时写入） */
    char update_time[20];              /* 状态更新时间 */
    int  dispatcher_id;                /* 更新此状态的调度员ID */
} TransportTracking;

/* ---- 订单统计结果 ---- */
typedef struct OrderStatResult {
    char  start_date[20];
    char  end_date[20];
    int   total_orders;
    int   completed_orders;
    int   rejected_orders;
    int   in_transit_orders;
    float completion_rate;      /* completed / total */
    float rejection_rate;       /* rejected / total */
    int   normal_goods_count;   /* 普通货物单数 */
    int   cold_chain_count;     /* 冷链货物单数 */
    int   fragile_count;        /* 易碎货物单数 */
    int   dangerous_count;      /* 危险品单数 */
} OrderStatResult;

/* ---- 仓储统计结果 ---- */
typedef struct WarehouseStatResult {
    char start_date[20];
    char end_date[20];
    int  total_inbound;         /* 入库总量 */
    int  total_outbound;        /* 出库总量 */
    int  current_total_inv;     /* 当前总库存 */
} WarehouseStatResult;

/* ---- 运输统计结果 ---- */
typedef struct TransportStatResult {
    char  start_date[20];
    char  end_date[20];
    int   total_dispatch;       /* 调度总单数 */
    int   completed_dispatch;   /* 运输完成单数 */
    float completion_rate;      /* 完成率 */
    float avg_transport_hours;  /* 平均运输时效(小时) */
} TransportStatResult;

/* ---- 操作日志 ---- */
typedef struct OperationLog {
    struct OperationLog *next;
    int  id;
    int  operator_id;           /* 操作人id */
    char operator_name[NAME_LEN];
    char action[ACTION_LEN];    /* 操作内容 */
    char timestamp[20];         /* 操作时间 */
} OperationLog;

/* ============================================================
 *           全局链表头指针（extern 声明）
 *           定义在 common.c 中
 * ============================================================ */
extern Goods              *goods_list_head;
extern User               *user_list_head;
extern Order              *order_list_head;
extern OrderDetail        *order_detail_list_head;
extern Warehouse          *warehouse_list_head;
extern InOutRecord        *inout_record_list_head;
extern Inventory          *inventory_list_head;
extern Route              *route_list_head;
extern Vehicle            *vehicle_list_head;
extern Driver             *driver_list_head;
extern Dispatch           *dispatch_list_head;
extern TransportTracking  *tracking_list_head;
extern OperationLog       *log_list_head;

/* 全局计数器 — 用于自增 ID */
extern int goods_id_counter;
extern int user_id_counter;
extern int order_detail_id_counter;
extern int warehouse_id_counter;
extern int inout_record_id_counter;
extern int inventory_id_counter;
extern int route_id_counter;
extern int vehicle_id_counter;
extern int driver_id_counter;
extern int tracking_id_counter;
extern int log_id_counter;
extern int order_sequence;          /* 订单号当日序号 */

/* 当前登录用户 */
extern User *current_user;

/* ============================================================
 *            公共函数声明
 * ============================================================ */

/* ---- 时间工具 ---- */
void get_current_time_str(char *buf);       /* YYYY-MM-DD HH:MM:SS */
void get_current_date_str(char *buf);       /* YYYYMMDD */
long time_diff_seconds(time_t from, time_t to);

/* ---- 密码工具 ---- */
void input_password(char *buf, int max_len);      /* 隐藏回显，支持退格 */
int  verify_password(const char *input, const char *stored);   /* 明文比较 */
int  validate_password(const char *pwd);         /* 密码长度6-10位 + 仅字母数字 */
void md5_hash(const char *input, char output[33]); /* MD5哈希，32位十六进制 */

/* ---- 订单号生成 ---- */
void generate_order_id(char *buf);
void init_order_sequence();

/* ---- 分页显示 ---- */
typedef void (*PrintRowFn)(const void *record, int index);
int show_paginated_list(const void *head,
                        size_t next_offset,
                        PrintRowFn print_row,
                        int page_size);

/* ---- 通用链表操作 ---- */
int  list_count(const void *head, size_t next_offset);  /* 返回链表的元素个数 */

/* ---- 通用二进制文件读写 ---- */
int  bin_save_list(const char *filename,
                   const void *head,
                   size_t record_size,
                   size_t next_offset);

int  bin_load_list(const char *filename,
                   void **head,
                   size_t record_size,
                   size_t next_offset);

/* ---- 用户管理 ---- */
void init_sample_users();                       /* 初始化示例用户 */
User *find_user_by_name(const char *name);       /* 按用户名查找 */
int  add_user(const char *name, const char *pwd, UserRole role);
void reset_password(User *u);                    /* 密码重置为 888888 */

/* ---- 角色工具 ---- */
/* 返回不同枚举对应的文字含义 */
const char *role_to_string(UserRole role);
const char *order_status_to_string(OrderStatus status);
const char *vehicle_status_to_string(VehicleStatus status);
const char *driver_status_to_string(DriverStatus status);
const char *dispatch_status_to_string(DispatchStatus status);
const char *goods_type_to_string(int type);
int         role_get_permissions(UserRole role);


#endif
