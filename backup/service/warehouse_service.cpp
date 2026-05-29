#include "warehouse_service.h"
#include "order_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <windows.h>

/* ---- 内部辅助：从链表恢复最大 ID ---- */
static int recover_max_id(const void *head, size_t next_offset,
                          size_t id_offset) {
    int max_id = 0;
    const char *p = (const char *)head;
    while (p) {
        int id = *(const int *)(p + id_offset);
        if (id > max_id) max_id = id;
        p = *(const char **)(p + next_offset);
    }
    return max_id;
}

/* ============================================================
 *  初始化 / 保存
 * ============================================================ */

int warehouse_svc_init() {
    CreateDirectoryA("data", NULL);

    int w_count = txt_load_list(WAREHOUSE_TXT_FILE,
        (void **)&warehouse_list_head, sizeof(Warehouse), offsetof(Warehouse, next));
    if (w_count <= 0) {
        w_count = bin_load_list(WAREHOUSE_DAT_FILE,
            (void **)&warehouse_list_head, sizeof(Warehouse), offsetof(Warehouse, next));
        if (w_count > 0) txt_save_list(WAREHOUSE_TXT_FILE, warehouse_list_head,
            sizeof(Warehouse), offsetof(Warehouse, next));
    }

    int r_count = txt_load_list(INOUT_RECORD_TXT_FILE,
        (void **)&inout_record_list_head, sizeof(InOutRecord), offsetof(InOutRecord, next));
    if (r_count <= 0) {
        r_count = bin_load_list(INOUT_RECORD_DAT_FILE,
            (void **)&inout_record_list_head, sizeof(InOutRecord), offsetof(InOutRecord, next));
        if (r_count > 0) txt_save_list(INOUT_RECORD_TXT_FILE, inout_record_list_head,
            sizeof(InOutRecord), offsetof(InOutRecord, next));
    }

    int v_count = txt_load_list(INVENTORY_TXT_FILE,
        (void **)&inventory_list_head, sizeof(Inventory), offsetof(Inventory, next));
    if (v_count <= 0) {
        v_count = bin_load_list(INVENTORY_DAT_FILE,
            (void **)&inventory_list_head, sizeof(Inventory), offsetof(Inventory, next));
        if (v_count > 0) txt_save_list(INVENTORY_TXT_FILE, inventory_list_head,
            sizeof(Inventory), offsetof(Inventory, next));
    }

    int l_count = txt_load_list(LOG_TXT_FILE,
        (void **)&log_list_head, sizeof(OperationLog), offsetof(OperationLog, next));
    if (l_count <= 0) {
        l_count = bin_load_list(LOG_DAT_FILE,
            (void **)&log_list_head, sizeof(OperationLog), offsetof(OperationLog, next));
        if (l_count > 0) txt_save_list(LOG_TXT_FILE, log_list_head,
            sizeof(OperationLog), offsetof(OperationLog, next));
    }

    if (w_count > 0)
        warehouse_id_counter = recover_max_id(warehouse_list_head,
            offsetof(Warehouse, next), offsetof(Warehouse, id));
    if (r_count > 0)
        inout_record_id_counter = recover_max_id(inout_record_list_head,
            offsetof(InOutRecord, next), offsetof(InOutRecord, id));
    if (v_count > 0)
        inventory_id_counter = recover_max_id(inventory_list_head,
            offsetof(Inventory, next), offsetof(Inventory, id));
    if (l_count > 0)
        log_id_counter = recover_max_id(log_list_head,
            offsetof(OperationLog, next), offsetof(OperationLog, id));

    /* 种子数据：创建默认仓库 */
    if (warehouse_list_head == NULL) {
        Warehouse *w = (Warehouse *)malloc(sizeof(Warehouse));
        memset(w, 0, sizeof(Warehouse));
        w->id = ++warehouse_id_counter;
        strcpy(w->name, "默认仓库");
        strcpy(w->address, "主仓库");
        w->manager_id = 0;
        w->next = NULL;
        warehouse_list_head = w;
        warehouse_svc_save_all();
    }

    return (w_count > 0 ? w_count : 0) +
           (r_count > 0 ? r_count : 0) +
           (v_count > 0 ? v_count : 0) +
           (l_count > 0 ? l_count : 0);
}

void warehouse_svc_save_all() {
    txt_save_list(WAREHOUSE_TXT_FILE, warehouse_list_head,
        sizeof(Warehouse), offsetof(Warehouse, next));
    txt_save_list(INOUT_RECORD_TXT_FILE, inout_record_list_head,
        sizeof(InOutRecord), offsetof(InOutRecord, next));
    txt_save_list(INVENTORY_TXT_FILE, inventory_list_head,
        sizeof(Inventory), offsetof(Inventory, next));
    txt_save_list(LOG_TXT_FILE, log_list_head,
        sizeof(OperationLog), offsetof(OperationLog, next));
}

/* ============================================================
 *  仓库
 * ============================================================ */

Warehouse *warehouse_svc_get_default() {
    return warehouse_list_head;
}

/* ============================================================
 *  库存
 * ============================================================ */

Inventory *inventory_svc_find(const char *goods_name, const char *goods_type, int warehouse_id) {
    Inventory *p = inventory_list_head;
    while (p) {
        if (p->warehouse_id == warehouse_id &&
            strcmp(p->goods_name, goods_name) == 0 &&
            strcmp(p->goods_type, goods_type) == 0)
            return p;
        if (p->warehouse_id == warehouse_id &&
            strcmp(p->goods_name, goods_name) == 0 &&
            strcmp(p->goods_type, goods_name) == 0) {
            strncpy(p->goods_type, goods_type, GOODS_TYPE_LEN - 1);
            p->goods_type[GOODS_TYPE_LEN - 1] = '\0';
            return p;
        }
        p = p->next;
    }
    return NULL;
}

Inventory *inventory_svc_list_all() {
    return inventory_list_head;
}

int inventory_svc_count() {
    return list_count(inventory_list_head, offsetof(Inventory, next));
}

/* ============================================================
 *  出入库记录
 * ============================================================ */

InOutRecord *inout_svc_list_all() {
    return inout_record_list_head;
}

/* ============================================================
 *  操作日志
 * ============================================================ */

OperationLog *log_svc_list_all() {
    return log_list_head;
}

void log_svc_add(int operator_id, const char *operator_name,
                 const char *action) {
    OperationLog *entry = (OperationLog *)malloc(sizeof(OperationLog));
    memset(entry, 0, sizeof(OperationLog));
    entry->id = ++log_id_counter;
    entry->operator_id = operator_id;
    strncpy(entry->operator_name, operator_name, NAME_LEN - 1);
    strncpy(entry->action, action, ACTION_LEN - 1);
    get_current_time_str(entry->timestamp);
    entry->next = log_list_head;
    log_list_head = entry;
}

/* ============================================================
 *  入库
 * ============================================================ */

int inbound_svc_execute(const char *order_id, int quantity,
                        const char *location_id, char *err_msg, int err_len) {
    if (quantity <= 0) {
        snprintf(err_msg, err_len, "入库数量必须大于0");
        return -3;
    }

    Order *order = order_svc_find_by_id(order_id);
    if (!order) {
        snprintf(err_msg, err_len, "订单 %s 不存在", order_id);
        return -1;
    }
    if (order->status != ORDER_PENDING_OUT) {
        snprintf(err_msg, err_len, "订单状态不是待出库，无法入库");
        return -2;
    }

    Warehouse *wh = warehouse_svc_get_default();
    const char *stock_name = strlen(order->goods_name) ? order->goods_name : order->goods_type;
    const char *stock_type = strlen(order->goods_type) ? order->goods_type : stock_name;
    Inventory *inv = inventory_svc_find(stock_name, stock_type, wh->id);

    if (!inv) {
        inv = (Inventory *)malloc(sizeof(Inventory));
        memset(inv, 0, sizeof(Inventory));
        inv->id = ++inventory_id_counter;
        strncpy(inv->goods_name, stock_name, NAME_LEN - 1);
        strncpy(inv->goods_type, stock_type, GOODS_TYPE_LEN - 1);
        inv->warehouse_id = wh->id;
        inv->quantity = 0;
        inv->next = inventory_list_head;
        inventory_list_head = inv;
    }

    inv->quantity += quantity;
    strncpy(inv->location_id, location_id, LOCATION_LEN - 1);
    get_current_time_str(inv->in_time);

    /* 创建出入库记录 */
    InOutRecord *rec = (InOutRecord *)malloc(sizeof(InOutRecord));
    memset(rec, 0, sizeof(InOutRecord));
    rec->id = ++inout_record_id_counter;
    strncpy(rec->order_id, order_id, ORDER_ID_LEN - 1);
    strncpy(rec->goods_name, stock_name, NAME_LEN - 1);
    strncpy(rec->goods_type, stock_type, GOODS_TYPE_LEN - 1);
    rec->quantity = quantity;
    rec->op_type = OP_INBOUND;
    get_current_time_str(rec->op_time);
    strncpy(rec->location_id, location_id, LOCATION_LEN - 1);
    rec->operator_id = current_user->id;
    rec->next = inout_record_list_head;
    inout_record_list_head = rec;

    /* 操作日志 */
    char action[ACTION_LEN];
    snprintf(action, sizeof(action), "入库: 订单%s, 货物%s x%d, 货位%s→待运输",
             order_id, stock_name, quantity, location_id);
    log_svc_add(current_user->id, current_user->name, action);

    /* 订单状态流转: 待出库 → 待运输（PDF 图9） */
    order->status = ORDER_PENDING_TRANSPORT;
    order_svc_save();

    warehouse_svc_save_all();
    return 0;
}

/* ============================================================
 *  出库（核心逻辑，两个入口复用）
 * ============================================================ */
static int outbound_core(const char *order_id, int quantity,
                         const char *location_id, char *err_msg, int err_len) {
    Warehouse *wh = warehouse_svc_get_default();
    const char *stock_name = NULL;
    const char *stock_type = NULL;

    /* 查找订单以获取货物名称 */
    Order *order = order_svc_find_by_id(order_id);
    if (order) {
        stock_name = strlen(order->goods_name) ? order->goods_name : order->goods_type;
        stock_type = strlen(order->goods_type) ? order->goods_type : stock_name;
    } else {
        snprintf(err_msg, err_len, "订单 %s 不存在", order_id);
        return -1;
    }

    Inventory *inv = inventory_svc_find(stock_name, stock_type, wh->id);
    if (!inv) {
        snprintf(err_msg, err_len, "货物 %s 暂无库存，请先入库", stock_name);
        return -3;
    }
    if (inv->quantity < quantity) {
        snprintf(err_msg, err_len, "库存不足：当前库存 %d，需要 %d",
                 inv->quantity, quantity);
        return -4;
    }

    inv->quantity -= quantity;
    strncpy(inv->location_id, location_id, LOCATION_LEN - 1);

    /* 创建出入库记录 */
    InOutRecord *rec = (InOutRecord *)malloc(sizeof(InOutRecord));
    memset(rec, 0, sizeof(InOutRecord));
    rec->id = ++inout_record_id_counter;
    strncpy(rec->order_id, order_id, ORDER_ID_LEN - 1);
    strncpy(rec->goods_name, stock_name, NAME_LEN - 1);
    strncpy(rec->goods_type, stock_type, GOODS_TYPE_LEN - 1);
    rec->quantity = quantity;
    rec->op_type = OP_OUTBOUND;
    get_current_time_str(rec->op_time);
    strncpy(rec->location_id, location_id, LOCATION_LEN - 1);
    rec->operator_id = current_user->id;
    rec->next = inout_record_list_head;
    inout_record_list_head = rec;

    /* 订单状态流转 → 运输中 */
    order->status = ORDER_IN_TRANSIT;
    order_svc_save();

    /* 操作日志 */
    char action[ACTION_LEN];
    snprintf(action, sizeof(action), "出库: 订单%s, 货物%s x%d→运输中, 货位%s",
             order_id, stock_name, quantity, location_id);
    log_svc_add(current_user->id, current_user->name, action);

    warehouse_svc_save_all();
    return 0;
}

/* ============================================================
 *  出库 — 入口 A: 按订单号（待出库订单直接出库）
 * ============================================================ */

int outbound_svc_execute(const char *order_id, int quantity,
                         const char *location_id, char *err_msg, int err_len) {
    if (quantity <= 0) {
        snprintf(err_msg, err_len, "出库数量必须大于0");
        return -5;
    }

    Order *order = order_svc_find_by_id(order_id);
    if (!order) {
        snprintf(err_msg, err_len, "订单 %s 不存在", order_id);
        return -1;
    }
    if (order->status != ORDER_PENDING_OUT) {
        snprintf(err_msg, err_len, "订单状态不是待出库，无法出库");
        return -2;
    }

    return outbound_core(order_id, quantity, location_id, err_msg, err_len);
}

/* ============================================================
 *  出库 — 入口 B: 按调度单号（PDF 图9 "调度指令" 路径）
 *  查调度单 → 找关联订单 → 校验待运输状态 → 扣库存 → 运输中
 * ============================================================ */
int outbound_svc_execute_by_dispatch(const char *dispatch_id, int quantity,
                                     const char *location_id, char *err_msg, int err_len) {
    if (quantity <= 0) {
        snprintf(err_msg, err_len, "出库数量必须大于0");
        return -5;
    }

    /* 查找调度单 */
    Dispatch *dp = dispatch_list_head;
    while (dp) {
        if (strcmp(dp->dispatch_id, dispatch_id) == 0) break;
        dp = dp->next;
    }
    if (!dp) {
        snprintf(err_msg, err_len, "调度单 %s 不存在", dispatch_id);
        return -1;
    }

    /* 查找关联订单 */
    Order *order = order_svc_find_by_id(dp->order_id);
    if (!order) {
        snprintf(err_msg, err_len, "调度单关联的订单 %s 不存在", dp->order_id);
        return -1;
    }
    if (order->status != ORDER_PENDING_TRANSPORT) {
        snprintf(err_msg, err_len, "订单状态不是待运输，无法按调度出库");
        return -2;
    }

    return outbound_core(dp->order_id, quantity, location_id, err_msg, err_len);
}
