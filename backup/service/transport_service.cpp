#include "transport_service.h"
#include "order_service.h"
#include "warehouse_service.h"
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
 *  种子数据
 * ============================================================ */
static void seed_vehicles() {
    Vehicle *v;
    v = (Vehicle *)malloc(sizeof(Vehicle)); memset(v, 0, sizeof(Vehicle));
    v->id = ++vehicle_id_counter;
    strcpy(v->plate_no, "辽A10001");
    v->type = VEHICLE_BOX;
    v->load_capacity = 5.0f;
    v->volume = 30.0f;
    v->capacity = 30.0f;
    v->status = VEHICLE_IDLE;
    v->next = vehicle_list_head; vehicle_list_head = v;

    v = (Vehicle *)malloc(sizeof(Vehicle)); memset(v, 0, sizeof(Vehicle));
    v->id = ++vehicle_id_counter;
    strcpy(v->plate_no, "辽A10002");
    v->type = VEHICLE_COLD_CHAIN;
    v->load_capacity = 3.0f;
    v->volume = 20.0f;
    v->capacity = 20.0f;
    v->status = VEHICLE_IDLE;
    v->next = vehicle_list_head; vehicle_list_head = v;

    v = (Vehicle *)malloc(sizeof(Vehicle)); memset(v, 0, sizeof(Vehicle));
    v->id = ++vehicle_id_counter;
    strcpy(v->plate_no, "辽A10003");
    v->type = VEHICLE_BOX;
    v->load_capacity = 8.0f;
    v->volume = 40.0f;
    v->capacity = 40.0f;
    v->status = VEHICLE_MAINTENANCE;
    v->next = vehicle_list_head; vehicle_list_head = v;
}

static void seed_drivers() {
    Driver *d;
    d = (Driver *)malloc(sizeof(Driver)); memset(d, 0, sizeof(Driver));
    d->id = ++driver_id_counter;
    strcpy(d->name, "张三");
    strcpy(d->id_card, "210101199001011234");
    strcpy(d->phone, "13800001111");
    d->status = DRIVER_ON_DUTY;
    d->next = driver_list_head; driver_list_head = d;

    d = (Driver *)malloc(sizeof(Driver)); memset(d, 0, sizeof(Driver));
    d->id = ++driver_id_counter;
    strcpy(d->name, "李四");
    strcpy(d->id_card, "210101199202022345");
    strcpy(d->phone, "13800002222");
    d->status = DRIVER_ON_DUTY;
    d->next = driver_list_head; driver_list_head = d;

    d = (Driver *)malloc(sizeof(Driver)); memset(d, 0, sizeof(Driver));
    d->id = ++driver_id_counter;
    strcpy(d->name, "王五");
    strcpy(d->id_card, "210101199303033456");
    strcpy(d->phone, "13800003333");
    d->status = DRIVER_ON_LEAVE;
    d->next = driver_list_head; driver_list_head = d;
}

static void seed_routes() {
    Route *r;
    r = (Route *)malloc(sizeof(Route)); memset(r, 0, sizeof(Route));
    r->id = ++route_id_counter;
    strcpy(r->origin, "上海");
    strcpy(r->destination, "北京");
    r->distance = 1200.0f;
    strcpy(r->estimated_time, "24h");
    r->next = route_list_head; route_list_head = r;

    r = (Route *)malloc(sizeof(Route)); memset(r, 0, sizeof(Route));
    r->id = ++route_id_counter;
    strcpy(r->origin, "广州");
    strcpy(r->destination, "深圳");
    r->distance = 140.0f;
    strcpy(r->estimated_time, "3h");
    r->next = route_list_head; route_list_head = r;
}

/* ============================================================
 *  初始化 / 持久化
 * ============================================================ */
int transport_svc_init() {
    CreateDirectoryA("data", NULL);
    init_dispatch_sequence();

    /* 从 txt 加载，失败则从旧 dat 迁移 */
    int v_count = txt_load_list(VEHICLE_TXT_FILE,
        (void **)&vehicle_list_head, sizeof(Vehicle), offsetof(Vehicle, next));
    if (v_count <= 0) {
        v_count = bin_load_list(VEHICLE_DAT_FILE,
            (void **)&vehicle_list_head, sizeof(Vehicle), offsetof(Vehicle, next));
        if (v_count > 0) txt_save_list(VEHICLE_TXT_FILE, vehicle_list_head,
            sizeof(Vehicle), offsetof(Vehicle, next));
    }

    int d_count = txt_load_list(DRIVER_TXT_FILE,
        (void **)&driver_list_head, sizeof(Driver), offsetof(Driver, next));
    if (d_count <= 0) {
        d_count = bin_load_list(DRIVER_DAT_FILE,
            (void **)&driver_list_head, sizeof(Driver), offsetof(Driver, next));
        if (d_count > 0) txt_save_list(DRIVER_TXT_FILE, driver_list_head,
            sizeof(Driver), offsetof(Driver, next));
    }

    int r_count = txt_load_list(ROUTE_TXT_FILE,
        (void **)&route_list_head, sizeof(Route), offsetof(Route, next));
    if (r_count <= 0) {
        r_count = bin_load_list(ROUTE_DAT_FILE,
            (void **)&route_list_head, sizeof(Route), offsetof(Route, next));
        if (r_count > 0) txt_save_list(ROUTE_TXT_FILE, route_list_head,
            sizeof(Route), offsetof(Route, next));
    }

    int dp_count = txt_load_list(DISPATCH_TXT_FILE,
        (void **)&dispatch_list_head, sizeof(Dispatch), offsetof(Dispatch, next));
    if (dp_count <= 0) {
        dp_count = bin_load_list(DISPATCH_DAT_FILE,
            (void **)&dispatch_list_head, sizeof(Dispatch), offsetof(Dispatch, next));
        if (dp_count > 0) txt_save_list(DISPATCH_TXT_FILE, dispatch_list_head,
            sizeof(Dispatch), offsetof(Dispatch, next));
    }

    int t_count = txt_load_list(TRACKING_TXT_FILE,
        (void **)&tracking_list_head, sizeof(TransportTracking), offsetof(TransportTracking, next));
    if (t_count <= 0) {
        t_count = bin_load_list(TRACKING_DAT_FILE,
            (void **)&tracking_list_head, sizeof(TransportTracking), offsetof(TransportTracking, next));
        if (t_count > 0) txt_save_list(TRACKING_TXT_FILE, tracking_list_head,
            sizeof(TransportTracking), offsetof(TransportTracking, next));
    }

    /* 恢复 ID 计数器 */
    if (v_count > 0)
        vehicle_id_counter = recover_max_id(vehicle_list_head,
            offsetof(Vehicle, next), offsetof(Vehicle, id));
    if (d_count > 0)
        driver_id_counter = recover_max_id(driver_list_head,
            offsetof(Driver, next), offsetof(Driver, id));
    if (r_count > 0)
        route_id_counter = recover_max_id(route_list_head,
            offsetof(Route, next), offsetof(Route, id));
    if (t_count > 0)
        tracking_id_counter = recover_max_id(tracking_list_head,
            offsetof(TransportTracking, next), offsetof(TransportTracking, track_id));

    /* 首次启动 → 种子数据 */
    int need_save = 0;
    if (vehicle_list_head == NULL)  { seed_vehicles(); need_save = 1; }
    if (driver_list_head == NULL)   { seed_drivers();  need_save = 1; }
    if (route_list_head == NULL)    { seed_routes();   need_save = 1; }

    if (need_save) transport_svc_save_all();

    return v_count + d_count + r_count + dp_count + t_count;
}

void transport_svc_save_all() {
    txt_save_list(VEHICLE_TXT_FILE, vehicle_list_head,
        sizeof(Vehicle), offsetof(Vehicle, next));
    txt_save_list(DRIVER_TXT_FILE, driver_list_head,
        sizeof(Driver), offsetof(Driver, next));
    txt_save_list(ROUTE_TXT_FILE, route_list_head,
        sizeof(Route), offsetof(Route, next));
    txt_save_list(DISPATCH_TXT_FILE, dispatch_list_head,
        sizeof(Dispatch), offsetof(Dispatch, next));
    txt_save_list(TRACKING_TXT_FILE, tracking_list_head,
        sizeof(TransportTracking), offsetof(TransportTracking, next));
}

/* ============================================================
 *  车辆
 * ============================================================ */
Vehicle *vehicle_svc_list_all() {
    return vehicle_list_head;
}

Vehicle *vehicle_svc_list_available() {
    Vehicle *filtered = NULL, *tail = NULL;
    Vehicle *p = vehicle_list_head;
    while (p) {
        if (p->status == VEHICLE_IDLE) {
            Vehicle *copy = (Vehicle *)malloc(sizeof(Vehicle));
            memcpy(copy, p, sizeof(Vehicle));
            copy->next = NULL;
            if (!filtered) filtered = copy;
            else tail->next = copy;
            tail = copy;
        }
        p = p->next;
    }
    return filtered;
}

Vehicle *vehicle_svc_find_by_id(int id) {
    Vehicle *p = vehicle_list_head;
    while (p) {
        if (p->id == id) return p;
        p = p->next;
    }
    return NULL;
}

int vehicle_svc_count() {
    return list_count(vehicle_list_head, offsetof(Vehicle, next));
}

/* ============================================================
 *  司机
 * ============================================================ */
Driver *driver_svc_list_all() {
    return driver_list_head;
}

Driver *driver_svc_list_available() {
    Driver *filtered = NULL, *tail = NULL;
    Driver *p = driver_list_head;
    while (p) {
        if (p->status == DRIVER_ON_DUTY) {
            Driver *copy = (Driver *)malloc(sizeof(Driver));
            memcpy(copy, p, sizeof(Driver));
            copy->next = NULL;
            if (!filtered) filtered = copy;
            else tail->next = copy;
            tail = copy;
        }
        p = p->next;
    }
    return filtered;
}

Driver *driver_svc_find_by_id(int id) {
    Driver *p = driver_list_head;
    while (p) {
        if (p->id == id) return p;
        p = p->next;
    }
    return NULL;
}

int driver_svc_count() {
    return list_count(driver_list_head, offsetof(Driver, next));
}

/* ============================================================
 *  线路
 * ============================================================ */
Route *route_svc_list_all() {
    return route_list_head;
}

Route *route_svc_find_by_id(int id) {
    Route *p = route_list_head;
    while (p) {
        if (p->id == id) return p;
        p = p->next;
    }
    return NULL;
}

int route_svc_count() {
    return list_count(route_list_head, offsetof(Route, next));
}

/* ============================================================
 *  调度单
 * ============================================================ */
int dispatch_svc_create(const char *order_id, int vehicle_id,
                         int driver_id, int route_id,
                         const char *planned_departure,
                         const char *planned_arrival,
                         char *err_msg, int err_len) {
    /* 1. 校验订单 */
    Order *order = order_svc_find_by_id(order_id);
    if (!order) {
        snprintf(err_msg, err_len, "订单 %s 不存在", order_id);
        return -1;
    }
    if (order->status != ORDER_PENDING_TRANSPORT) {
        snprintf(err_msg, err_len, "订单状态不是待运输（当前：%s），无法调度",
                 order_status_to_string(order->status));
        return -2;
    }

    /* 2. 校验车辆 */
    Vehicle *v = vehicle_svc_find_by_id(vehicle_id);
    if (!v) {
        snprintf(err_msg, err_len, "车辆 ID=%d 不存在", vehicle_id);
        return -3;
    }
    if (v->status != VEHICLE_IDLE) {
        snprintf(err_msg, err_len, "车辆 %s 当前不可用（状态：%s）",
                 v->plate_no, vehicle_status_to_string(v->status));
        return -4;
    }

    /* 3. 校验司机 */
    Driver *d = driver_svc_find_by_id(driver_id);
    if (!d) {
        snprintf(err_msg, err_len, "司机 ID=%d 不存在", driver_id);
        return -5;
    }
    if (d->status != DRIVER_ON_DUTY) {
        snprintf(err_msg, err_len, "司机 %s 当前不在岗（状态：%s）",
                 d->name, driver_status_to_string(d->status));
        return -6;
    }

    /* 4. 校验线路 */
    Route *r = route_svc_find_by_id(route_id);
    if (!r) {
        snprintf(err_msg, err_len, "线路 ID=%d 不存在", route_id);
        return -7;
    }

    /* 5. 生成调度单号 + 创建调度单 */
    Dispatch *dp = (Dispatch *)malloc(sizeof(Dispatch));
    memset(dp, 0, sizeof(Dispatch));
    generate_dispatch_id(dp->dispatch_id);
    strncpy(dp->order_id, order_id, ORDER_ID_LEN - 1);
    dp->vehicle_id = vehicle_id;
    dp->driver_id = driver_id;
    dp->route_id = route_id;
    strncpy(dp->planned_departure, planned_departure, sizeof(dp->planned_departure) - 1);
    strncpy(dp->planned_arrival, planned_arrival, sizeof(dp->planned_arrival) - 1);
    dp->status = DISPATCH_PENDING;
    dp->next = dispatch_list_head;
    dispatch_list_head = dp;

    /* 6. 车辆状态 → 运输中 */
    v->status = VEHICLE_IN_TRANSIT;

    /* 7. 操作日志 */
    char action[ACTION_LEN];
    snprintf(action, sizeof(action),
             "调度: 调度单%s, 订单%s, 车辆%s, 司机%s, 线路%s→%s",
             dp->dispatch_id, order_id, v->plate_no, d->name,
             r->origin, r->destination);
    log_svc_add(current_user->id, current_user->name, action);

    transport_svc_save_all();
    return 0;
}

Dispatch *dispatch_svc_list_all() {
    return dispatch_list_head;
}

Dispatch *dispatch_svc_find_by_id(const char *dispatch_id) {
    Dispatch *p = dispatch_list_head;
    while (p) {
        if (strcmp(p->dispatch_id, dispatch_id) == 0) return p;
        p = p->next;
    }
    return NULL;
}

Dispatch *dispatch_svc_find_by_order(const char *order_id) {
    Dispatch *p = dispatch_list_head;
    while (p) {
        if (strcmp(p->order_id, order_id) == 0) return p;
        p = p->next;
    }
    return NULL;
}

int dispatch_svc_count() {
    return list_count(dispatch_list_head, offsetof(Dispatch, next));
}

/* ============================================================
 *  运输轨迹
 * ============================================================ */
int tracking_svc_add(const char *dispatch_id, DispatchStatus node_status,
                      const char *exception_reason) {
    Dispatch *dp = dispatch_svc_find_by_id(dispatch_id);
    if (!dp) return -1;

    /* 创建轨迹节点 */
    TransportTracking *t = (TransportTracking *)malloc(sizeof(TransportTracking));
    memset(t, 0, sizeof(TransportTracking));
    t->track_id = ++tracking_id_counter;
    strncpy(t->dispatch_id, dispatch_id, DISPATCH_ID_LEN - 1);
    t->node_status = node_status;
    if (exception_reason)
        strncpy(t->exception_reason, exception_reason, REASON_LEN - 1);
    get_current_time_str(t->update_time);
    t->dispatcher_id = current_user->id;
    t->next = tracking_list_head;
    tracking_list_head = t;

    /* 同步 Dispatch 状态 */
    dp->status = node_status;

    /* 同步 Order 状态（PDF 图12） */
    Order *order = order_svc_find_by_id(dp->order_id);
    char action[ACTION_LEN];

    if (node_status == DISPATCH_DEPARTED && order) {
        order->status = ORDER_IN_TRANSIT;
        order_svc_save();
        snprintf(action, sizeof(action),
                 "运输出发: 调度单%s, 订单%s→运输中", dispatch_id, dp->order_id);
    } else if (node_status == DISPATCH_DELIVERED && order) {
        order->status = ORDER_DELIVERED;
        order_svc_save();
        snprintf(action, sizeof(action),
                 "运输送达: 调度单%s, 订单%s→已送达", dispatch_id, dp->order_id);
    } else if (node_status == DISPATCH_ABNORMAL) {
        snprintf(action, sizeof(action),
                 "运输异常: 调度单%s, 原因: %s", dispatch_id,
                 exception_reason ? exception_reason : "");
    } else {
        snprintf(action, sizeof(action),
                 "运输状态更新: 调度单%s→%s", dispatch_id,
                 dispatch_status_to_string(node_status));
    }

    log_svc_add(current_user->id, current_user->name, action);
    transport_svc_save_all();
    return 0;
}

TransportTracking *tracking_svc_list_by_dispatch(const char *dispatch_id) {
    TransportTracking *filtered = NULL, *tail = NULL;
    TransportTracking *p = tracking_list_head;
    while (p) {
        if (strcmp(p->dispatch_id, dispatch_id) == 0) {
            TransportTracking *copy = (TransportTracking *)malloc(sizeof(TransportTracking));
            memcpy(copy, p, sizeof(TransportTracking));
            copy->next = NULL;
            if (!filtered) filtered = copy;
            else tail->next = copy;
            tail = copy;
        }
        p = p->next;
    }
    return filtered;
}

TransportTracking *tracking_svc_list_all() {
    return tracking_list_head;
}

int tracking_svc_count() {
    return list_count(tracking_list_head, offsetof(TransportTracking, next));
}
