#include "order_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* 创建一条种子订单的辅助函数 */
static void seed_order(const char *cust_name, OrderStatus status,
                       const char *from, const char *to, const char *goods) {
	User *u = find_user_by_name(cust_name);
	Order *o = (Order *)malloc(sizeof(Order));
	memset(o, 0, sizeof(Order));
	generate_order_id(o->order_id);
	o->user_id = u ? u->id : 0;
	strncpy(o->customer_name, cust_name, NAME_LEN - 1);
	strncpy(o->customer_phone, "13800000000", PHONE_LEN - 1);
	strncpy(o->customer_addr, from, ADDR_LEN - 1);
	strncpy(o->from_addr, from, ADDR_LEN - 1);
	strncpy(o->to_addr, to, ADDR_LEN - 1);
	strncpy(o->goods_name, goods, NAME_LEN - 1);
	strcpy(o->goods_type, goods);
	strcpy(o->goods_weight, "1kg");
	o->goods_quantity = 1;
	strcpy(o->goods_volume, "1m3");
	strcpy(o->expected_delivery_time, "2026-05-25 18:00");
	o->status = status;
	o->next = order_list_head;
	order_list_head = o;
}

int order_svc_init() {
	int count = txt_load_list(ORDER_TXT_FILE, (void **)&order_list_head, sizeof(Order), offsetof(Order, next));
	if (count <= 0) {
		count = bin_load_list(ORDER_DAT_FILE, (void **)&order_list_head, sizeof(Order), offsetof(Order, next));
		if (count > 0) txt_save_list(ORDER_TXT_FILE, order_list_head, sizeof(Order), offsetof(Order, next));
	}
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

	/* 无订单文件 → 创建种子订单（7条，覆盖多用户多状态） */
	seed_order("user1", ORDER_PENDING_REVIEW, "北京市海淀区", "上海市浦东新区", "普通");
	seed_order("user1", ORDER_PENDING_OUT,    "广州市天河区", "深圳市南山区", "易碎");
	seed_order("user1", ORDER_IN_TRANSIT,     "成都市武侯区", "重庆市渝中区", "冷链");
	seed_order("user2", ORDER_PENDING_REVIEW, "杭州市西湖区", "南京市鼓楼区", "普通");
	seed_order("user2", ORDER_REJECTED,       "武汉市洪山区", "西安市雁塔区", "危险品");
	seed_order("user1", ORDER_DELIVERED,      "长沙市岳麓区", "郑州市金水区", "普通");
	seed_order("user3", ORDER_PENDING_REVIEW, "天津市和平区", "济南市历下区", "冷链");
	order_svc_save();
	return 7;
}

int order_svc_save() {
	return txt_save_list(ORDER_TXT_FILE, order_list_head, sizeof(Order), offsetof(Order, next));
}

Order *order_svc_find_by_id(const char *order_id) {
	Order *p = order_list_head;
	while (p) {
		if (strcmp(p->order_id, order_id) == 0) return p;
		p = p->next;
	}
	return NULL;
}

Order *order_svc_list_by_user(int user_id) {
	Order *filtered = NULL, *tail = NULL;
	Order *p = order_list_head;
	while (p) {
		if (p->user_id == user_id) {
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

int order_svc_update(const char *order_id, const Order *new_data) {
	Order *o = order_svc_find_by_id(order_id);
	if (!o) return -1;
	if (o->status != ORDER_PENDING_REVIEW) return -2;

	/* 仅覆盖可编辑字段，保留 order_id / user_id / status / reject_reason */
	strncpy(o->customer_name,           new_data->customer_name,           NAME_LEN - 1);
	strncpy(o->customer_phone,          new_data->customer_phone,          PHONE_LEN - 1);
	strncpy(o->customer_addr,           new_data->customer_addr,           ADDR_LEN - 1);
	strncpy(o->from_addr,               new_data->from_addr,               ADDR_LEN - 1);
	strncpy(o->to_addr,                 new_data->to_addr,                 ADDR_LEN - 1);
	strncpy(o->goods_name,              new_data->goods_name,              NAME_LEN - 1);
	strncpy(o->goods_type,              new_data->goods_type,              GOODS_TYPE_LEN - 1);
	strncpy(o->goods_weight,            new_data->goods_weight,            sizeof(o->goods_weight) - 1);
	o->goods_quantity = new_data->goods_quantity;
	strncpy(o->goods_volume,            new_data->goods_volume,            sizeof(o->goods_volume) - 1);
	strncpy(o->expected_delivery_time,  new_data->expected_delivery_time,  sizeof(o->expected_delivery_time) - 1);

	order_svc_save();
	return 0;
}

int order_svc_complete(const char *order_id) {
	Order *o = order_svc_find_by_id(order_id);
	if (!o) return -1;
	if (o->status != ORDER_DELIVERED) return -2;

	o->status = ORDER_COMPLETED;
	order_svc_save();
	return 0;
}
