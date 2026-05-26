#ifndef WAREHOUSE_SERVICE_H
#define WAREHOUSE_SERVICE_H
#include "../public/common.h"

#define WAREHOUSE_DAT_FILE    "data/warehouses.dat"
#define INOUT_RECORD_DAT_FILE "data/inout_records.dat"
#define INVENTORY_DAT_FILE    "data/inventories.dat"
#define LOG_DAT_FILE          "data/logs.dat"

/* ---- Init / Save ---- */
int  warehouse_svc_init();
void warehouse_svc_save_all();

/* ---- Warehouse ---- */
Warehouse *warehouse_svc_get_default();

/* ---- Inbound (入库) ---- */
/* Returns 0=success, -1=order not found, -2=wrong status, -3=invalid qty */
int  inbound_svc_execute(const char *order_id, int quantity,
                         const char *location_id, char *err_msg, int err_len);

/* ---- Outbound (出库) ---- */
/* Returns 0=success, -1=order not found, -2=wrong status,
   -3=no inventory, -4=insufficient qty, -5=invalid qty */
int  outbound_svc_execute(const char *order_id, int quantity,
                          const char *location_id, char *err_msg, int err_len);

/* ---- Inventory ---- */
Inventory *inventory_svc_find(const char *goods_type, int warehouse_id);
Inventory *inventory_svc_list_all();
int         inventory_svc_count();

/* ---- InOutRecord ---- */
InOutRecord *inout_svc_list_all();

/* ---- Log ---- */
OperationLog *log_svc_list_all();
void          log_svc_add(int operator_id, const char *operator_name,
                          const char *action);

#endif
