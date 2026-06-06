#ifndef STATS_SERVICE_H
#define STATS_SERVICE_H

#include "../public/common.h"

/* start_ymd/end_ymd 为 YYYYMMDD，按订单号日期闭区间统计 */
void stats_svc_calc_order(const char *start_ymd, const char *end_ymd,
                          OrderStatResult *out);

/* 导出订单统计 TXT 报表，返回 0=成功，-1=文件写入失败 */
int  stats_svc_export_order_txt(const char *filename,
                                const OrderStatResult *stat);

/* 仓储统计：按日期范围统计出入库总量，当前库存不按日期过滤 */
void stats_svc_calc_warehouse(const char *start_ymd, const char *end_ymd,
                              WarehouseStatResult *out);
int  stats_svc_export_warehouse_txt(const char *filename,
                                    const WarehouseStatResult *stat);

/* 运输统计：按预计出发日期统计调度完成率和平均运输时效 */
void stats_svc_calc_transport(const char *start_ymd, const char *end_ymd,
                              TransportStatResult *out);
int  stats_svc_export_transport_txt(const char *filename,
                                    const TransportStatResult *stat);

#endif
