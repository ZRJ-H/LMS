#include <graphics.h>
#include <conio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "orderWin.h"
#include "../public/common.h"
#include "../public/ui_config.h"
#include "../view/control.h"
#include "../service/order_service.h"
#include "../service/warehouse_service.h"

/* ============================================================
 *  内部辅助函数
 * ============================================================ */

/* ------- 订单表格行绘制回调 ------- */
static void drawOrderRow(const void *record, int row_idx, int y_base,
                         int table_x, const int *col_widths, int ncols) {
	const Order *o = (const Order *)record;
	char buf[64];
	int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));

	outtextxy(x + CTRL_PADDING, y_base + 8, o->order_id);
	x += col_widths[0];

	outtextxy(x + CTRL_PADDING, y_base + 8, o->customer_name);
	x += col_widths[1];

	sprintf(buf, "%s", order_status_to_string(o->status));
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[2];

	outtextxy(x + CTRL_PADDING, y_base + 8, o->goods_type);
}

/* ------- 订单分页表格 ------- */
void showOrderList(const Order *head) {
	const char *headers[] = {"订单号", "客户", "状态", "货物类型"};
	const int col_widths[] = {180, 100, 90, 100};
	window_show_table("订单列表", headers, col_widths, 4,
	                  head, offsetof(Order, next), drawOrderRow, PAGE_SIZE);
}

/* ============================================================
 *  对外接口
 * ============================================================ */

/* ========== 创建物流订单窗口 ========== */
void createOrderWin() {
	WINDOW_T win = {
		180, 60, 460, 470, WHITE_COLOR, 15, {
			{190, 70, 440, 30, "创建物流订单",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 120, 80, INPUT_H, "客户姓名：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 120, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{190, 170, 80, INPUT_H, "联系电话：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 170, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 220, 80, INPUT_H, "发货地址：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 220, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 270, 80, INPUT_H, "收货地址：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 270, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 320, 80, INPUT_H, "货物类型：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 320, 200, INPUT_H, "普通|易碎|冷链|危险品",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
			{190, 370, 80, INPUT_H, "预计送达：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 370, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{210, 430, BTN_W, BTN_H, "确认创建",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 430, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 13) {
			char *name  = win.controls[2].text;
			char *phone = win.controls[4].text;
			char *from  = win.controls[6].text;
			char *to    = win.controls[8].text;
			char *time_str = win.controls[12].text;
			int  gtype  = win.controls[10].sel_index;

			if (strlen(name) == 0 || strlen(phone) == 0 ||
			    strlen(from) == 0 || strlen(to) == 0) {
				MessageBoxA(GetHWnd(), "客户姓名、电话、地址不能为空！",
				            "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			Order *o = (Order *)malloc(sizeof(Order));
			memset(o, 0, sizeof(Order));
			generate_order_id(o->order_id);
			strncpy(o->customer_name, name, NAME_LEN - 1);
			strncpy(o->customer_phone, phone, PHONE_LEN - 1);
			strncpy(o->from_addr, from, ADDR_LEN - 1);
			strncpy(o->to_addr, to, ADDR_LEN - 1);
			strcpy(o->goods_type, goods_type_to_string(gtype));
			if (strlen(time_str) > 0) strncpy(o->expected_delivery_time, time_str, 19);
			o->status = ORDER_PENDING_REVIEW;
			o->user_id = current_user->id;

			o->next = order_list_head;
			order_list_head = o;
			order_svc_save();

			char msg[256];
			sprintf(msg, "订单创建成功！\n订单号: %s\n货物: %s\n状态: 待审核",
			        o->order_id, o->goods_type);
			MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}
		else if (win.current == 14) {
			return;
		}
	}
}

/* ========== 订单查询窗口 ========== */
void searchOrderWin() {
	WINDOW_T win = {
		180, 60, 460, 450, WHITE_COLOR, 11, {
			{190, 70, 440, 30, "订单查询",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 115, 80, INPUT_H, "订单号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 115, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{190, 160, 80, INPUT_H, "客户名：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 160, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 205, 80, INPUT_H, "状态：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 205, 200, INPUT_H, "全部|待审核|已驳回|待出库|运输中|已送达|已完成",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
			{190, 280, BTN_W, BTN_H, "查询",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 280, BTN_W, BTN_H, "全部订单",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{190, 360, BTN_W, BTN_H, "我的订单",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 360, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
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
				MessageBoxA(GetHWnd(), "未找到匹配的订单", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				showOrderList(filtered);
				while (filtered) {
					Order *tmp = filtered;
					filtered = filtered->next;
					free(tmp);
				}
			}
		}
		else if (win.current == 8) {
			showOrderList(order_list_head);
		}
		else if (win.current == 9) {
			/* "我的订单" — 按当前用户ID筛选 */
			Order *my = order_svc_list_by_user(current_user->id);
			if (!my) {
				MessageBoxA(GetHWnd(), "你还没有创建过订单", "提示", MB_OK | MB_ICONINFORMATION);
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
			return;
		}
	}
}

/* ========== 订单审核窗口 ========== */
void auditOrderWin() {
	while (1) {
		Order *pending = order_svc_list_by_status(ORDER_PENDING_REVIEW);
		if (!pending) {
			MessageBoxA(GetHWnd(), "暂无待审核订单", "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}

		showOrderList(pending);
		while (pending) {
			Order *tmp = pending;
			pending = pending->next;
			free(tmp);
		}

		WINDOW_T win = {
			200, 200, 400, 280, WHITE_COLOR, 7, {
				{210, 210, 380, 30, "订单审核操作",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{210, 255, 80, INPUT_H, "订单号：",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{290, 255, 290, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
				{210, 305, 80, INPUT_H, "驳回原因：",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{290, 305, 290, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
				{210, 370, BTN_W, BTN_H, "审核通过",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
				{410, 370, BTN_W, BTN_H, "驳回",
				 RED_BTN, LIGHT_RED, WHITE_COLOR, BUTTON, 0, 0, 0, 0},
			}
		};

		window_show(win);
		win = window_run(win);

		if (win.current == 5) {
			char *oid = win.controls[2].text;
			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			Order *o = order_svc_find_by_id(oid);
			if (!o) {
				MessageBoxA(GetHWnd(), "订单不存在", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (o->status != ORDER_PENDING_REVIEW) {
				MessageBoxA(GetHWnd(), "该订单不在待审核状态", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			o->status = ORDER_PENDING_OUT;
			order_svc_save();
			char msg[256];
			sprintf(msg, "订单 %s 已审核通过 -> 待出库", oid);
			MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
		}
		else if (win.current == 6) {
			char *oid    = win.controls[2].text;
			char *reason = win.controls[4].text;
			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (strlen(reason) == 0) {
				MessageBoxA(GetHWnd(), "驳回必须填写原因", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			Order *o = order_svc_find_by_id(oid);
			if (!o) {
				MessageBoxA(GetHWnd(), "订单不存在", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (o->status != ORDER_PENDING_REVIEW) {
				MessageBoxA(GetHWnd(), "该订单不在待审核状态", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			o->status = ORDER_REJECTED;
			strncpy(o->reject_reason, reason, REASON_LEN - 1);
			o->reject_reason[REASON_LEN - 1] = '\0';
			order_svc_save();
			char msg[256];
			sprintf(msg, "订单 %s 已驳回\n原因: %s", oid, reason);
			MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
		}
		else {
			return;
		}
	}
}



/* ========== 出入库记录表格（订单跟踪用） ========== */
static void drawInOutRow(const void *record, int row_idx, int y_base,
                         int table_x, const int *col_widths, int ncols) {
	const InOutRecord *r = (const InOutRecord *)record;
	char buf[64];
	int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));

	sprintf(buf, "%d", r->id);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[0];

	outtextxy(x + CTRL_PADDING, y_base + 8, r->order_id);
	x += col_widths[1];

	outtextxy(x + CTRL_PADDING, y_base + 8, r->goods_type);
	x += col_widths[2];

	sprintf(buf, "%d", r->quantity);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[3];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          (char *)operation_type_to_string(r->op_type));
}

static void showInOutList(const InOutRecord *head) {
	const char *headers[] = {"ID", "订单号", "货物类型", "数量", "操作"};
	const int col_widths[] = {50, 180, 100, 60, 60};
	window_show_table("出入库记录", headers, col_widths, 5,
	                  head, offsetof(InOutRecord, next), drawInOutRow, PAGE_SIZE);
}

/* ========== 订单跟踪窗口 ========== */
void trackOrderWin() {
	WINDOW_T win = {
		180, 80, 460, 420, WHITE_COLOR, 9, {
			{190, 90, 440, 30, "订单跟踪",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 140, 80, INPUT_H, "订单号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 140, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{190, 210, BTN_W, BTN_H, "查询订单详情",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 210, BTN_W, BTN_H, "查看出入库记录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{190, 280, BTN_W, BTN_H, "查看操作日志",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 280, BTN_W, BTN_H, "出入库流水",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{210, 370, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 3) {
			char *oid = win.controls[2].text;
			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			Order *o = order_svc_find_by_id(oid);
			if (!o) {
				MessageBoxA(GetHWnd(), "订单不存在", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			char info[1024];
			sprintf(info,
			        "订单号：%s\n客户：%s\n电话：%s\n发货：%s\n收货：%s\n货物：%s\n状态：%s\n预计送达：%s\n驳回原因：%s\n用户ID：%d",
			        o->order_id, o->customer_name, o->customer_phone,
			        o->from_addr, o->to_addr, o->goods_type,
			        order_status_to_string(o->status),
			        strlen(o->expected_delivery_time) ? o->expected_delivery_time : "--",
			        strlen(o->reject_reason) ? o->reject_reason : "无",
			        o->user_id);
			MessageBoxA(GetHWnd(), info, "订单跟踪", MB_OK | MB_ICONINFORMATION);
		}
		else if (win.current == 4) {
			char *oid = win.controls[2].text;
			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请先输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			InOutRecord *filtered = NULL, *tail = NULL;
			InOutRecord *p = inout_record_list_head;
			while (p) {
				if (strcmp(p->order_id, oid) == 0) {
					InOutRecord *copy = (InOutRecord *)malloc(sizeof(InOutRecord));
					memcpy(copy, p, sizeof(InOutRecord));
					copy->next = NULL;
					if (!filtered) filtered = copy;
					else tail->next = copy;
					tail = copy;
				}
				p = p->next;
			}
			if (!filtered) {
				MessageBoxA(GetHWnd(), "该订单暂无出入库记录", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				showInOutList(filtered);
				while (filtered) {
					InOutRecord *tmp = filtered;
					filtered = filtered->next;
					free(tmp);
				}
			}
		}
		else if (win.current == 5) {
			char *oid = win.controls[2].text;
			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请先输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			OperationLog *filtered = NULL, *tail = NULL;
			OperationLog *p = log_list_head;
			while (p) {
				if (strstr(p->action, oid)) {
					OperationLog *copy = (OperationLog *)malloc(sizeof(OperationLog));
					memcpy(copy, p, sizeof(OperationLog));
					copy->next = NULL;
					if (!filtered) filtered = copy;
					else tail->next = copy;
					tail = copy;
				}
				p = p->next;
			}
			if (!filtered) {
				MessageBoxA(GetHWnd(), "该订单暂无操作日志", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				char log_text[1024] = "";
				OperationLog *lp = filtered;
				int shown = 0;
				while (lp && shown < 10) {
					char line[256];
					sprintf(line, "[%s] %s: %s", lp->timestamp, lp->operator_name, lp->action);
					strncat(log_text, line, sizeof(log_text) - strlen(log_text) - 1);
					strncat(log_text, "\n", sizeof(log_text) - strlen(log_text) - 1);
					lp = lp->next;
					shown++;
				}
				MessageBoxA(GetHWnd(), log_text, "操作日志", MB_OK | MB_ICONINFORMATION);
				while (filtered) {
					OperationLog *tmp = filtered;
					filtered = filtered->next;
					free(tmp);
				}
			}
		}
		else if (win.current == 6) {
			if (!inout_record_list_head)
				MessageBoxA(GetHWnd(), "暂无出入库记录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				showInOutList(inout_record_list_head);
		}
		else if (win.current == 7) {
			return;
		}
	}
}
