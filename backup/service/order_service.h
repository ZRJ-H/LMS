#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H
#include "../public/common.h"

#define ORDER_DAT_FILE "data/orders.dat"

/* 初始化：从 data/orders.dat 加载，恢复最大 ID */
int    order_svc_init();

/* 持久化当前订单链表到文件 */
int    order_svc_save();

/* 按订单号查找 */
Order *order_svc_find_by_id(const char *order_id);

/* 按状态筛选（返回新链表头，调用者负责释放） */
Order *order_svc_list_by_status(OrderStatus status);

/* 获取订单链表头 */
Order *order_svc_list_all();

/* 订单总数 */
int    order_svc_count();

#endif
