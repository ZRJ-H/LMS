#include "order_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

int order_svc_init() {
	int count = bin_load_list(ORDER_DAT_FILE, (void **)&order_list_head, sizeof(Order), offsetof(Order, next));
	if (count > 0) {
		/* 从已有订单中恢复 order_sequence 的最大当日序号 */
		char today[16];
		get_current_date_str(today);
		Order *p = order_list_head;
		int max_seq = 0;
		while (p) {
			/* 订单号格式: WL + YYYYMMDD + 6位序号，检查是否为今日订单 */
			if (strncmp(p->order_id + 2, today, 8) == 0) {
				int seq = atoi(p->order_id + 10);
				if (seq > max_seq) max_seq = seq;
			}
			p = p->next;
		}
		if (max_seq > order_sequence) order_sequence = max_seq;
		return count;
	}
	return 0;
}

int order_svc_save() {
	return bin_save_list(ORDER_DAT_FILE, order_list_head, sizeof(Order), offsetof(Order, next));
}

Order *order_svc_find_by_id(const char *order_id) {
	Order *p = order_list_head;
	while (p) {
		if (strcmp(p->order_id, order_id) == 0) return p;
		p = p->next;
	}
	return NULL;
}

Order *order_svc_list_by_status(OrderStatus status) {
	Order *filtered = NULL, *tail = NULL;
	Order *p = order_list_head;
	while (p) {
		if (p->status == status) {
			Order *copy = (Order *)malloc(sizeof(Order));
			memcpy(copy, p, sizeof(Order));
			copy->next = NULL;
			if (!filtered) filtered = copy;
			else tail->next = copy;
			tail = copy;
		}
		p = p->next;
	}
	return filtered;
}

Order *order_svc_list_all() {
	return order_list_head;
}

int order_svc_count() {
	return list_count(order_list_head, offsetof(Order, next));
}
