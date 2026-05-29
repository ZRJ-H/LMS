#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H
#include "../public/common.h"

#define ORDER_TXT_FILE "data/orders.txt"
#define ORDER_DAT_FILE "data/orders.dat"

/* 初始化：优先从 data/orders.txt 加载，恢复最大 ID；旧 data/orders.dat 仅用于一次性迁移 */
int    order_svc_init();

/* 持久化当前订单链表到文件 */
int    order_svc_save();

/* 按订单号查找 */
Order *order_svc_find_by_id(const char *order_id);

/* 按状态筛选（返回新链表头，调用者负责释放） */
Order *order_svc_list_by_status(OrderStatus status);

/* 按用户ID筛选（返回新链表头，调用者负责释放） */
Order *order_svc_list_by_user(int user_id);

/* 获取订单链表头 */
Order *order_svc_list_all();

/* 订单总数 */
int    order_svc_count();

/* 修改订单 — 仅待审核状态可改，new_data 中的可编辑字段覆盖到原订单
   返回 0=成功, -1=不存在, -2=状态不允许 */
int    order_svc_update(const char *order_id, const Order *new_data);

/* 确认完成 — 已送达→已完成
   返回 0=成功, -1=不存在, -2=状态不允许 */
int    order_svc_complete(const char *order_id);

#endif
