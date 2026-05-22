#include <graphics.h>
#include <conio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "orderWin.h"
#include "../public/common.h"
#include "../view/control.h"
#include "../service/order_service.h"

/* ============================================================
 *  内部辅助函数 — 仅本文件使用
 * ============================================================ */

/* ------- 订单表格行绘制回调 -------
 * 供 window_show_table 调用，在图形窗口绘制单行订单数据
 * 列顺序：订单号 | 客户 | 状态 | 货物类型 */
static void drawOrderRow(const void *record, int row_idx, int y_base,
                         int table_x, const int *col_widths, int ncols) {
	const Order *o = (const Order *)record;
	char buf[64];
	int x = table_x;
	settextcolor(BLACK);
	settextstyle(14, 8, _T("宋体"));

	outtextxy(x + 5, y_base + 5, o->order_id);
	x += col_widths[0];

	outtextxy(x + 5, y_base + 5, o->customer_name);
	x += col_widths[1];

	sprintf(buf, "%s", order_status_to_string(o->status));
	outtextxy(x + 5, y_base + 5, buf);
	x += col_widths[2];

	outtextxy(x + 5, y_base + 5, o->goods_type);
}

/* ------- 订单分页表格 -------
 * 以 window_show_table 展示订单链表，每页 8 行
 * head 可以是全局链表或临时筛选链表 */
static void showOrderList(const Order *head) {
	const char *headers[] = {"订单号", "客户", "状态", "货物类型"};
	const int col_widths[] = {180, 100, 90, 100};
	window_show_table("订单列表", headers, col_widths, 4,
	                  head, offsetof(Order, next), drawOrderRow, 5);
}

/* ============================================================
 *  对外接口
 * ============================================================ */

/* ========== 创建物流订单窗口 ========== */
void createOrderWin() {
	WINDOW_T win = {
	    180, 60, 460, 480, WHITE, 15, {
	        {190, 70, 440, 30, "创建物流订单", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {190, 120, 80, 35, "客户姓名：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 120, 200, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0},
	        {190, 170, 80, 35, "联系电话：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 170, 200, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 0, 0, 0},
	        {190, 220, 80, 35, "发货地址：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 220, 360, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 0, 0, 0},
	        {190, 270, 80, 35, "收货地址：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 270, 360, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 0, 0, 0},
	        {190, 320, 80, 35, "货物类型：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 320, 200, 35, "普通|易碎|冷链|危险品", CYAN, LIGHTCYAN, WHITE, COMBO, 0, 0, 0},
	        {190, 370, 80, 35, "预计送达：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 370, 200, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 0, 0, 0},
	        {210, 430, 180, 50, "确认创建", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {410, 430, 180, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 13) {
			char *name   = win.controls[2].text;
			char *phone  = win.controls[4].text;
			char *from   = win.controls[6].text;
			char *to     = win.controls[8].text;
			char *time   = win.controls[12].text;
			int  gtype   = win.controls[10].sel_index;

			/* 必填校验 */
			if (strlen(name) == 0 || strlen(phone) == 0 || strlen(from) == 0 || strlen(to) == 0) {
				MessageBoxA(NULL, "客户姓名、电话、地址不能为空！", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			/* 构造订单对象 */
			Order *o = (Order *)malloc(sizeof(Order));
			memset(o, 0, sizeof(Order));
			generate_order_id(o->order_id);
			strncpy(o->customer_name, name, NAME_LEN - 1);
			strncpy(o->customer_phone, phone, PHONE_LEN - 1);
			strncpy(o->from_addr, from, ADDR_LEN - 1);
			strncpy(o->to_addr, to, ADDR_LEN - 1);
			strcpy(o->goods_type, goods_type_to_string(gtype));
			if (strlen(time) > 0) strncpy(o->expected_delivery_time, time, 19);
			o->status = ORDER_PENDING_REVIEW;
			o->user_id = current_user->id;

			/* 插入全局链表头部 + 落盘 */
			o->next = order_list_head;
			order_list_head = o;
			order_svc_save();

			char msg[256];
			sprintf(msg, "订单创建成功！\n订单号: %s\n货物: %s\n状态: 待审核",
			        o->order_id, o->goods_type);
			MessageBoxA(NULL, msg, "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}
		else if (win.current == 14) {
			return;  /* 返回上级菜单 */
		}
	}
}

/* ========== 订单查询窗口 ========== */
void searchOrderWin() {
	WINDOW_T win = {
	    180, 60, 460, 450, WHITE, 11, {
	        {190, 70, 440, 30, "订单查询", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {190, 115, 80, 35, "订单号：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 115, 200, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0},
	        {190, 160, 80, 35, "客户名：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 160, 200, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 0, 0, 0},
	        {190, 205, 80, 35, "状态：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {270, 205, 200, 35, "全部|待审核|已驳回|待出库|运输中|已送达|已完成", CYAN, LIGHTCYAN, WHITE, COMBO, 0, 0, 0},
	        {190, 280, 160, 50, "查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {380, 280, 160, 50, "全部订单", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {190, 360, 160, 50, "我的订单", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {380, 360, 160, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 7) {
			/* "查询"按钮 — 按条件筛选 */
			char *order_id  = win.controls[2].text;
			char *cust_name = win.controls[4].text;
			int   sel       = win.controls[6].sel_index;

			Order *filtered = NULL, *tail = NULL;
			Order *p = order_list_head;
			while (p) {
				int match = 1;
				if (strlen(order_id) > 0 && strstr(p->order_id, order_id) == NULL)
					match = 0;
				if (strlen(cust_name) > 0 && strstr(p->customer_name, cust_name) == NULL)
					match = 0;
				/* sel==0 表示"全部"，否则映射到枚举值 (sel-1) */
				if (sel > 0 && (int)p->status != sel - 1)
					match = 0;

				if (match) {
					Order *copy = (Order *)malloc(sizeof(Order));
					memcpy(copy, p, sizeof(Order));
					copy->next = NULL;
					if (!filtered) filtered = copy;
					else tail->next = copy;
					tail = copy;
				}
				p = p->next;
			}

			if (!filtered) {
				MessageBoxA(NULL, "未找到匹配的订单", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				showOrderList(filtered);
				/* 释放临时筛选链表 */
				while (filtered) {
					Order *tmp = filtered;
					filtered = filtered->next;
					free(tmp);
				}
			}
		}
		else if (win.current == 8) {
			/* "全部订单"按钮 — 直接展示全局链表 */
			showOrderList(order_list_head);
		}
		else if (win.current == 9) {
			/* "我的订单"按钮 — 按当前用户ID筛选 */
			Order *my = order_svc_list_by_user(current_user->id);
			if (!my) {
				MessageBoxA(NULL, "你还没有创建过订单", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				showOrderList(my);
				while (my) {
					Order *tmp = my;
					my = my->next;
					free(tmp);
				}
			}
		}
		else if (win.current == 10) {
			return;  /* 返回上级菜单 */
		}
	}
}

/* ========== 订单审核窗口 ========== */
void auditOrderWin() {
	while (1) {
		/* 第一步：展示所有待审核订单 */
		Order *pending = order_svc_list_by_status(ORDER_PENDING_REVIEW);
		if (!pending) {
			MessageBoxA(NULL, "暂无待审核订单", "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}

		showOrderList(pending);

		/* 释放临时链表 */
		while (pending) {
			Order *tmp = pending;
			pending = pending->next;
			free(tmp);
		}

		/* 第二步：输入订单号并选择审核操作 */
		WINDOW_T win = {
		    200, 200, 400, 280, WHITE, 7, {
		        {210, 210, 380, 30, "订单审核操作", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {210, 255, 80, 35, "订单号：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {290, 255, 290, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0},
		        {210, 305, 80, 35, "驳回原因：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {290, 305, 290, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT, 0, 0, 0},
		        {210, 370, 180, 50, "审核通过", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {410, 370, 180, 50, "驳回", LIGHTRED, RED, WHITE, BUTTON, 0, 0, 0},
		    }
		};

		window_show(win);
		win = window_run(win);

		if (win.current == 5) {
			/* 审核通过 */
			char *oid = win.controls[2].text;
			if (strlen(oid) == 0) {
				MessageBoxA(NULL, "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			Order *o = order_svc_find_by_id(oid);
			if (!o) {
				MessageBoxA(NULL, "订单不存在", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (o->status != ORDER_PENDING_REVIEW) {
				MessageBoxA(NULL, "该订单不在待审核状态", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			o->status = ORDER_PENDING_OUT;
			order_svc_save();
			char msg[256];
			sprintf(msg, "订单 %s 已审核通过 → 待出库", oid);
			MessageBoxA(NULL, msg, "提示", MB_OK | MB_ICONINFORMATION);
		}
		else if (win.current == 6) {
			/* 驳回 */
			char *oid    = win.controls[2].text;
			char *reason = win.controls[4].text;
			if (strlen(oid) == 0) {
				MessageBoxA(NULL, "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (strlen(reason) == 0) {
				MessageBoxA(NULL, "驳回必须填写原因", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			Order *o = order_svc_find_by_id(oid);
			if (!o) {
				MessageBoxA(NULL, "订单不存在", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (o->status != ORDER_PENDING_REVIEW) {
				MessageBoxA(NULL, "该订单不在待审核状态", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			o->status = ORDER_REJECTED;
			strncpy(o->reject_reason, reason, REASON_LEN - 1);
			o->reject_reason[REASON_LEN - 1] = '\0';
			order_svc_save();
			char msg[256];
			sprintf(msg, "订单 %s 已驳回\n原因: %s", oid, reason);
			MessageBoxA(NULL, msg, "提示", MB_OK | MB_ICONINFORMATION);
		}
		else {
			/* 关闭窗口 → 返回上级菜单 */
			return;
		}
	}
}
