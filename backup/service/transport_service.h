#ifndef TRANSPORT_SERVICE_H
#define TRANSPORT_SERVICE_H
#include "../public/common.h"

#define VEHICLE_TXT_FILE   "data/vehicles.txt"
#define DRIVER_TXT_FILE    "data/drivers.txt"
#define ROUTE_TXT_FILE     "data/routes.txt"
#define DISPATCH_TXT_FILE  "data/dispatches.txt"
#define TRACKING_TXT_FILE  "data/trackings.txt"

/* ---- 旧版二进制（仅迁移用）---- */
#define VEHICLE_DAT_FILE   "data/vehicles.dat"
#define DRIVER_DAT_FILE    "data/drivers.dat"
#define ROUTE_DAT_FILE     "data/routes.dat"
#define DISPATCH_DAT_FILE  "data/dispatches.dat"
#define TRACKING_DAT_FILE  "data/trackings.dat"

/* ========== 初始化 / 持久化 ========== */
int  transport_svc_init();
void transport_svc_save_all();

/* ========== Vehicle（车辆）========== */
Vehicle *vehicle_svc_list_all();
Vehicle *vehicle_svc_list_available();   /* status == IDLE */
Vehicle *vehicle_svc_find_by_id(int id);
int      vehicle_svc_count();

/* ========== Driver（司机）========== */
Driver *driver_svc_list_all();
Driver *driver_svc_list_available();     /* status == ON_DUTY */
Driver *driver_svc_find_by_id(int id);
int     driver_svc_count();

/* ========== Route（线路）========== */
Route *route_svc_list_all();
Route *route_svc_find_by_id(int id);
int    route_svc_count();

/* ========== Dispatch（调度单）========== */
/* 创建调度单：生成 DD+日期+6位序号，关联订单/车辆/司机/线路
   校验：order 存在且 status==PENDING_TRANSPORT
         vehicle 存在且 status==IDLE
         driver 存在且 status==ON_DUTY
         route 存在
   成功后：Vehicle.status→IN_TRANSIT, 记操作日志, 持久化
   返回 0=成功, <0=失败（err_msg 填充原因） */
int  dispatch_svc_create(const char *order_id, int vehicle_id,
                         int driver_id, int route_id,
                         const char *planned_departure,
                         const char *planned_arrival,
                         char *err_msg, int err_len);

Dispatch *dispatch_svc_list_all();
Dispatch *dispatch_svc_find_by_id(const char *dispatch_id);
Dispatch *dispatch_svc_find_by_order(const char *order_id);
int       dispatch_svc_count();

/* ========== TransportTracking（运输轨迹）========== */
/* 新增一条轨迹节点，同步更新 Dispatch.status
   若 node_status==DEPARTED → Order.status→IN_TRANSIT + order_svc_save()
   若 node_status==DELIVERED → Order.status→DELIVERED + order_svc_save()
   每次操作记操作日志 + transport_svc_save_all()
   返回 0=成功, -1=dispatch 不存在 */
int  tracking_svc_add(const char *dispatch_id, DispatchStatus node_status,
                      const char *exception_reason);

TransportTracking *tracking_svc_list_by_dispatch(const char *dispatch_id);
TransportTracking *tracking_svc_list_all();
int                tracking_svc_count();

#endif
