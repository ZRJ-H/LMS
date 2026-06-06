#include "stats_service.h"
#include "order_service.h"
#include "warehouse_service.h"
#include "transport_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int order_date_in_range(const Order *o, const char *start_ymd, const char *end_ymd) {
	char ymd[9];
	if (!o || strlen(o->order_id) < 10) return 0;
	memcpy(ymd, o->order_id + 2, 8);
	ymd[8] = '\0';
	return strcmp(ymd, start_ymd) >= 0 && strcmp(ymd, end_ymd) <= 0;
}

static void stat_copy_date(char *dst, const char *ymd) {
	sprintf(dst, "%.4s-%.2s-%.2s", ymd, ymd + 4, ymd + 6);
}

static int date_ymd_in_range(const char *ymd, const char *start_ymd, const char *end_ymd) {
	return ymd[0] != '\0' &&
	       strcmp(ymd, start_ymd) >= 0 &&
	       strcmp(ymd, end_ymd) <= 0;
}

static int datetime_to_ymd(const char *dt, char *ymd) {
	if (!dt || strlen(dt) < 10) {
		ymd[0] = '\0';
		return 0;
	}
	sprintf(ymd, "%.4s%.2s%.2s", dt, dt + 5, dt + 8);
	return 1;
}

static int parse_datetime(const char *s, time_t *out) {
	int y, mon, d, h, min, sec;
	struct tm tmv;
	if (!s || sscanf(s, "%d-%d-%d %d:%d:%d", &y, &mon, &d, &h, &min, &sec) != 6)
		return 0;
	memset(&tmv, 0, sizeof(tmv));
	tmv.tm_year = y - 1900;
	tmv.tm_mon = mon - 1;
	tmv.tm_mday = d;
	tmv.tm_hour = h;
	tmv.tm_min = min;
	tmv.tm_sec = sec;
	tmv.tm_isdst = -1;
	*out = mktime(&tmv);
	return *out != (time_t)-1;
}

static void stat_count_goods_type(OrderStatResult *out, const char *goods_type) {
	if (!goods_type) return;
	if (strcmp(goods_type, goods_type_to_string(GOODS_NORMAL)) == 0)
		out->normal_goods_count++;
	else if (strcmp(goods_type, goods_type_to_string(GOODS_FRAGILE)) == 0)
		out->fragile_count++;
	else if (strcmp(goods_type, goods_type_to_string(GOODS_COLD_CHAIN)) == 0)
		out->cold_chain_count++;
	else if (strcmp(goods_type, goods_type_to_string(GOODS_DANGEROUS)) == 0)
		out->dangerous_count++;
	else
		out->normal_goods_count++;
}

void stats_svc_calc_order(const char *start_ymd, const char *end_ymd,
                          OrderStatResult *out) {
	memset(out, 0, sizeof(OrderStatResult));
	stat_copy_date(out->start_date, start_ymd);
	stat_copy_date(out->end_date, end_ymd);

	for (Order *p = order_svc_list_all(); p; p = p->next) {
		if (!order_date_in_range(p, start_ymd, end_ymd)) continue;
		out->total_orders++;
		if (p->status == ORDER_COMPLETED) out->completed_orders++;
		if (p->status == ORDER_REJECTED) out->rejected_orders++;
		if (p->status == ORDER_IN_TRANSIT) out->in_transit_orders++;
		stat_count_goods_type(out, p->goods_type);
	}

	if (out->total_orders > 0) {
		out->completion_rate = (float)out->completed_orders * 100.0f / out->total_orders;
		out->rejection_rate = (float)out->rejected_orders * 100.0f / out->total_orders;
	}
}

int stats_svc_export_order_txt(const char *filename,
                               const OrderStatResult *stat) {
	FILE *fp = fopen(filename, "w");
	if (!fp) return -1;

	fprintf(fp, "智能物流管理系统订单统计报表\n");
	fprintf(fp, "统计时间：%s 至 %s\n", stat->start_date, stat->end_date);
	fprintf(fp, "总订单数：%d\n", stat->total_orders);
	fprintf(fp, "已完成订单数：%d\n", stat->completed_orders);
	fprintf(fp, "订单完成率：%.1f%%\n", stat->completion_rate);
	fprintf(fp, "驳回订单数：%d\n", stat->rejected_orders);
	fprintf(fp, "驳回率：%.1f%%\n", stat->rejection_rate);
	fprintf(fp, "运输中订单数：%d\n", stat->in_transit_orders);
	fprintf(fp, "货物类型统计：\n");
	fprintf(fp, "普通货物：%d单\n", stat->normal_goods_count);
	fprintf(fp, "易碎货物：%d单\n", stat->fragile_count);
	fprintf(fp, "冷链货物：%d单\n", stat->cold_chain_count);
	fprintf(fp, "危险品：%d单\n", stat->dangerous_count);

	fclose(fp);
	return 0;
}

void stats_svc_calc_warehouse(const char *start_ymd, const char *end_ymd,
                              WarehouseStatResult *out) {
	char ymd[9];
	memset(out, 0, sizeof(WarehouseStatResult));
	stat_copy_date(out->start_date, start_ymd);
	stat_copy_date(out->end_date, end_ymd);

	for (InOutRecord *p = inout_svc_list_all(); p; p = p->next) {
		if (!datetime_to_ymd(p->op_time, ymd) ||
		    !date_ymd_in_range(ymd, start_ymd, end_ymd))
			continue;
		if (p->op_type == OP_INBOUND)
			out->total_inbound += p->quantity;
		else if (p->op_type == OP_OUTBOUND)
			out->total_outbound += p->quantity;
	}

	for (Inventory *p = inventory_svc_list_all(); p; p = p->next)
		out->current_total_inv += p->quantity;
}

int stats_svc_export_warehouse_txt(const char *filename,
                                   const WarehouseStatResult *stat) {
	FILE *fp = fopen(filename, "w");
	if (!fp) return -1;

	fprintf(fp, "智能物流管理系统仓储统计报表\n");
	fprintf(fp, "统计时间：%s 至 %s\n", stat->start_date, stat->end_date);
	fprintf(fp, "入库总数量：%d\n", stat->total_inbound);
	fprintf(fp, "出库总数量：%d\n", stat->total_outbound);
	fprintf(fp, "当前库存总量：%d\n", stat->current_total_inv);

	fclose(fp);
	return 0;
}

static void free_tracking_copy_list(TransportTracking *head) {
	while (head) {
		TransportTracking *next = head->next;
		free(head);
		head = next;
	}
}

static int calc_dispatch_hours(const char *dispatch_id, float *hours) {
	TransportTracking *list = tracking_svc_list_by_dispatch(dispatch_id);
	TransportTracking *p = list;
	time_t depart_time = 0, delivered_time = 0;
	int has_departed = 0, has_delivered = 0;

	while (p) {
		if (!has_departed && p->node_status == DISPATCH_DEPARTED)
			has_departed = parse_datetime(p->update_time, &depart_time);
		if (!has_delivered && p->node_status == DISPATCH_DELIVERED)
			has_delivered = parse_datetime(p->update_time, &delivered_time);
		p = p->next;
	}

	free_tracking_copy_list(list);
	if (!has_departed || !has_delivered || delivered_time < depart_time)
		return 0;

	*hours = (float)time_diff_seconds(depart_time, delivered_time) / 3600.0f;
	return 1;
}

void stats_svc_calc_transport(const char *start_ymd, const char *end_ymd,
                              TransportStatResult *out) {
	char ymd[9];
	float total_hours = 0.0f;
	int timed_completed_count = 0;
	memset(out, 0, sizeof(TransportStatResult));
	stat_copy_date(out->start_date, start_ymd);
	stat_copy_date(out->end_date, end_ymd);

	for (Dispatch *p = dispatch_svc_list_all(); p; p = p->next) {
		if (!datetime_to_ymd(p->planned_departure, ymd) ||
		    !date_ymd_in_range(ymd, start_ymd, end_ymd))
			continue;
		out->total_dispatch++;
		if (p->status == DISPATCH_DELIVERED) {
			float hours = 0.0f;
			out->completed_dispatch++;
			if (calc_dispatch_hours(p->dispatch_id, &hours)) {
				total_hours += hours;
				timed_completed_count++;
			}
		}
	}

	if (out->total_dispatch > 0)
		out->completion_rate = (float)out->completed_dispatch * 100.0f / out->total_dispatch;
	if (timed_completed_count > 0)
		out->avg_transport_hours = total_hours / timed_completed_count;
}

int stats_svc_export_transport_txt(const char *filename,
                                   const TransportStatResult *stat) {
	FILE *fp = fopen(filename, "w");
	if (!fp) return -1;

	fprintf(fp, "智能物流管理系统运输统计报表\n");
	fprintf(fp, "统计时间：%s 至 %s\n", stat->start_date, stat->end_date);
	fprintf(fp, "调度总单数：%d\n", stat->total_dispatch);
	fprintf(fp, "运输完成单数：%d\n", stat->completed_dispatch);
	fprintf(fp, "运输完成率：%.1f%%\n", stat->completion_rate);
	fprintf(fp, "平均运输时效：%.1f 小时\n", stat->avg_transport_hours);

	fclose(fp);
	return 0;
}
