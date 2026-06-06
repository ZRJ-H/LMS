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

static void drawQueryFrame(const char *title, int panel_h) {
	setfillcolor(WHITE_COLOR);
	fillrectangle(UI_PANEL_X, UI_PANEL_Y, UI_PANEL_X + UI_PANEL_W, UI_PANEL_Y + panel_h);
	setlinecolor(FRAME_BLUE);
	rectangle(UI_PANEL_X, UI_PANEL_Y, UI_PANEL_X + UI_PANEL_W, UI_PANEL_Y + panel_h);

	settextstyle(FONT_HEADER_H, 0, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + (UI_PANEL_W - textwidth(title)) / 2, UI_PANEL_Y + 18, title);

	char left[128], right[128];
	sprintf(left, "当前用户: %s(%s)", current_user->name, role_to_string(current_user->role));
	sprintf(right, "登录时间: %s", login_time_str[0] ? login_time_str : "----");
	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	outtextxy(UI_PANEL_X + 35, UI_PANEL_Y + 62, left);
	outtextxy(UI_PANEL_X + UI_PANEL_W - 35 - textwidth(right), UI_PANEL_Y + 62, right);
}

static void drawTextBox(int x, int y, int w, int h, const char *text, int active) {
	setlinecolor(active ? PRIMARY : INPUT_BORDER);
	setfillcolor(WHITE_COLOR);
	fillrectangle(x, y, x + w, y + h);
	rectangle(x, y, x + w, y + h);
	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(x + 5, y + (h - textheight(text)) / 2, (char *)text);
}

static void drawQueryButton(int x, int y, int w, int h) {
	setlinecolor(INPUT_BORDER);
	setfillcolor(RGB(165, 175, 185));
	fillrectangle(x, y, x + w, y + h);
	rectangle(x, y, x + w, y + h);
	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(WHITE_COLOR);
	outtextxy(x + (w - textwidth("查询")) / 2, y + (h - textheight("查询")) / 2, "查询");
}

static int inRect(int mx, int my, int x, int y, int w, int h) {
	return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

static void drawPageText(int y, int pages, int page) {
	char page_text[96];
	sprintf(page_text, "<-上页     共%d页 当前第%d页     ->下页", pages, page + 1);
	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + (UI_PANEL_W - textwidth(page_text)) / 2, y, page_text);
}

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

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(o->goods_name) ? o->goods_name : o->goods_type);
}

/* ------- 订单分页表格 ------- */
void showOrderList(const Order *head) {
	const char *headers[] = {"订单号", "客户", "状态", "货物名称"};
	const int col_widths[] = {180, 100, 90, 100};
	window_show_table("订单列表", headers, col_widths, 4,
	                  head, offsetof(Order, next), drawOrderRow, PAGE_SIZE);
}

/* ============================================================
 *  对外接口
 * ============================================================ */

/* ========== 创建物流订单窗口 ========== */
void createOrderWin() {
	window_clear_frame();
	window_set_card(0);
	window_set_frame(135, 55, 520, 500);

	char preview_order_id[32];
	char date_buf[16];
	get_current_date_str(date_buf);
	sprintf(preview_order_id, "WL%s%06d", date_buf, order_sequence + 1);
	char meta_left[100], meta_right[100];
	sprintf(meta_left, "当前用户: %s(%s)", current_user->name, role_to_string(current_user->role));
	sprintf(meta_right, "登录时间: %s", login_time_str[0] ? login_time_str : "----");

	WINDOW_T win = {
		135, 55, 520, 500, WHITE_COLOR, 27, {
			{275, 155, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0, 1},
			{275, 183, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, INPUT_FILTER_PHONE},
			{275, 211, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{275, 239, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{275, 267, 185, 24, "普通|易碎|冷链|危险品",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
			{275, 295, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{275, 323, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, INPUT_FILTER_DIGITS},
			{275, 351, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{275, 379, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{275, 407, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{225, 460, 100, 30, "提交",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{390, 460, 100, 30, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{245, 78, 300, 26, "智能物流管理系统订单创建界面",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{155, 115, 205, 22, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{385, 115, 240, 22, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 140, 100, 22, "订单号:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{275, 140, 230, 22, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 160, 100, 22, "客户姓名:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 188, 100, 22, "联系电话:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 216, 100, 22, "客户地址:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 244, 100, 22, "货物名称:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 272, 100, 22, "货物类型:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 300, 100, 22, "货物重量:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 328, 100, 22, "货物数量:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 356, 100, 22, "货物体积:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 384, 100, 22, "发货地址:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 412, 100, 22, "收货地址:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
		}
	};
	strcpy(win.controls[13].text, meta_left);
	strcpy(win.controls[14].text, meta_right);
	strcpy(win.controls[16].text, preview_order_id);

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 10) {
			char *name  = win.controls[0].text;
			char *phone = win.controls[1].text;
			char *cust_addr = win.controls[2].text;
			char *goods_name = win.controls[3].text;
			int  gtype  = win.controls[4].sel_index;
			char *weight = win.controls[5].text;
			char *qty = win.controls[6].text;
			char *volume = win.controls[7].text;
			char *from  = win.controls[8].text;
			char *to    = win.controls[9].text;

			if (strlen(name) == 0 || strlen(phone) == 0 ||
			    strlen(cust_addr) == 0 || strlen(goods_name) == 0 ||
			    strlen(weight) == 0 || strlen(qty) == 0 ||
			    strlen(volume) == 0 || strlen(from) == 0 || strlen(to) == 0) {
				MessageBoxA(GetHWnd(), "订单创建信息不能为空",
				            "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (atoi(qty) <= 0) {
				MessageBoxA(GetHWnd(), "货物数量必须为正整数",
				            "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			Order *o = (Order *)malloc(sizeof(Order));
			memset(o, 0, sizeof(Order));
			generate_order_id(o->order_id);
			strncpy_gbk_safe(o->customer_name, name, NAME_LEN);
			strncpy(o->customer_phone, phone, PHONE_LEN - 1);
			strncpy_gbk_safe(o->customer_addr, cust_addr, ADDR_LEN);
			strncpy_gbk_safe(o->from_addr, from, ADDR_LEN);
			strncpy_gbk_safe(o->to_addr, to, ADDR_LEN);
			strncpy_gbk_safe(o->goods_name, goods_name, NAME_LEN);
			strcpy(o->goods_type, goods_type_to_string(gtype));
			strncpy(o->goods_weight, weight, sizeof(o->goods_weight) - 1);
			o->goods_quantity = atoi(qty);
			strncpy(o->goods_volume, volume, sizeof(o->goods_volume) - 1);
			o->expected_delivery_time[0] = '\0';
			o->status = ORDER_PENDING_REVIEW;
			o->user_id = current_user->id;

			o->next = order_list_head;
			order_list_head = o;
			order_svc_save();

			char msg[256];
			sprintf(msg, "订单创建成功！\n订单号: %s\n货物: %s\n数量: %d\n状态: 待审核",
			        o->order_id,
			        strlen(o->goods_name) ? o->goods_name : o->goods_type,
			        o->goods_quantity);
			MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}
		else if (win.current == 11) {
			return;
		}
	}
}

/* ========== 订单查询窗口 ========== */
static int order_matches_query_pdf(const Order *o, const char *order_id, const char *cust_name) {
	if (order_id[0] && strstr(o->order_id, order_id) == NULL) return 0;
	if (cust_name[0] && strstr(o->customer_name, cust_name) == NULL) return 0;
	return 1;
}

static int order_query_total_pages(const char *order_id, const char *cust_name) {
	int total = 0;
	for (Order *p = order_list_head; p; p = p->next) {
		if (order_matches_query_pdf(p, order_id, cust_name)) total++;
	}
	return total ? (total + 3) / 4 : 1;
}

static void draw_order_query_pdf(const char *order_id, const char *cust_name,
                                 int focus, int page) {
	const int col_w[4] = {150, 85, 100, 75};
	const char *headers[4] = {"订单号", "客户姓名", "联系电话", "订单状态"};
	int header_h = 22, row_h = 22;
	int total_w = col_w[0] + col_w[1] + col_w[2] + col_w[3];
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = UI_PANEL_Y + 122;
	int start = page * 4;
	int shown = 0, idx = 0, x;

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统订单查询界面", 315);

	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 25, UI_PANEL_Y + 91, "订单号:");
	drawTextBox(UI_PANEL_X + 82, UI_PANEL_Y + 84, 120, 24, order_id, focus == 0);
	outtextxy(UI_PANEL_X + 215, UI_PANEL_Y + 91, "客户姓名:");
	drawTextBox(UI_PANEL_X + 290, UI_PANEL_Y + 84, 78, 24, cust_name, focus == 1);
	drawQueryButton(UI_PANEL_X + 378, UI_PANEL_Y + 84, 52, 26);

	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	setfillcolor(BG_TABLE_HDR);
	x = table_x;
	for (int c = 0; c < 4; c++) {
		fillrectangle(x, table_y, x + col_w[c], table_y + header_h);
		setlinecolor(GRAY_LINE);
		rectangle(x, table_y, x + col_w[c], table_y + header_h);
		outtextxy(x + 5, table_y + 6, (char *)headers[c]);
		x += col_w[c];
	}

	for (Order *p = order_list_head; p && shown < 4; p = p->next) {
		if (!order_matches_query_pdf(p, order_id, cust_name)) continue;
		if (idx++ < start) continue;
		int y = table_y + header_h + shown * row_h;
		setfillcolor(WHITE_COLOR);
		fillrectangle(table_x, y, table_x + total_w, y + row_h);
		setlinecolor(GRAY_LINE);
		rectangle(table_x, y, table_x + total_w, y + row_h);
		x = table_x;
		settextcolor(TEXT_MAIN);
		outtextxy(x + 5, y + 5, p->order_id); x += col_w[0];
		outtextxy(x + 5, y + 5, p->customer_name); x += col_w[1];
		outtextxy(x + 5, y + 5, p->customer_phone); x += col_w[2];
		outtextxy(x + 5, y + 5, (char *)order_status_to_string(p->status));
		int vx = table_x;
		for (int c = 0; c < 4; c++) {
			vx += col_w[c];
			line(vx, y, vx, y + row_h);
		}
		shown++;
	}
	if (shown == 0) {
		settextcolor(TEXT_MUTED);
		outtextxy(table_x + 130, table_y + header_h + 42, "暂无匹配订单");
	}

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	drawPageText(UI_PANEL_Y + 240, order_query_total_pages(order_id, cust_name), page);
}

static void searchOrderWinPdf() {
	char order_id[64] = {0};
	char cust_name[64] = {0};
	int focus = 0, page = 0;
	const int oid_x = UI_PANEL_X + 82, oid_y = UI_PANEL_Y + 84, oid_w = 120, oid_h = 24;
	const int name_x = UI_PANEL_X + 290, name_y = UI_PANEL_Y + 84, name_w = 78, name_h = 24;
	const int query_x = UI_PANEL_X + 378, query_y = UI_PANEL_Y + 84, query_w = 52, query_h = 26;
	int need_redraw = 1;

	while (1) {
		int pages = order_query_total_pages(order_id, cust_name);
		if (page >= pages) page = pages - 1;
		if (need_redraw) {
			draw_order_query_pdf(order_id, cust_name, focus, page);
			need_redraw = 0;
		}
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *buf = focus == 0 ? order_id : cust_name;
		int max_len = focus == 0 ? 23 : 31;

		if (msg.message == WM_LBUTTONDOWN) {
			if (inRect(msg.x, msg.y, oid_x, oid_y, oid_w, oid_h)) {
				if (focus != 0) { focus = 0; input_reset_pending(); need_redraw = 1; }
			}
			else if (inRect(msg.x, msg.y, name_x, name_y, name_w, name_h)) {
				if (focus != 1) { focus = 1; input_reset_pending(); need_redraw = 1; }
			}
			else if (inRect(msg.x, msg.y, query_x, query_y, query_w, query_h)) {
				page = 0;
				need_redraw = 1;
			}
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB || msg.vkcode == VK_UP || msg.vkcode == VK_DOWN) {
				focus = 1 - focus;
				input_reset_pending();
				need_redraw = 1;
			}
			if (msg.vkcode == VK_BACK) {
				if (input_delete_last_char(buf)) {
					page = 0;
					need_redraw = 1;
				}
			}
			if (msg.vkcode == VK_LEFT && page > 0) {
				page--;
				need_redraw = 1;
			}
			if (msg.vkcode == VK_RIGHT && page < pages - 1) {
				page++;
				need_redraw = 1;
			}
		}
		else if (msg.message == WM_CHAR) {
			int filter = (focus == 1) ? INPUT_FILTER_CHINESE : INPUT_FILTER_ALNUM;
			if (input_append_char(buf, max_len, msg.ch, filter)) {
				page = 0;
				need_redraw = 1;
			}
		}
	}
}

void searchOrderWin() {
	searchOrderWinPdf();
}

/* ========== 通用订单筛选匹配 ========== */
static int order_matches_query(const Order *o, const char *order_id,
                                const char *cust_name, const char *phone,
                                const char *goods_name) {
	if (order_id[0] && strstr(o->order_id, order_id) == NULL) return 0;
	if (cust_name[0] && strstr(o->customer_name, cust_name) == NULL) return 0;
	if (phone[0] && strstr(o->customer_phone, phone) == NULL) return 0;
	if (goods_name[0]) {
		const char *gname = strlen(o->goods_name) ? o->goods_name : o->goods_type;
		if (strstr(gname, goods_name) == NULL) return 0;
	}
	return 1;
}

/* ---- 6列订单表格（序号/订单号/客户/电话/货物/数量）---- */
static void drawOrderRow6Col(const void *record, int row_idx, int y_base,
                              int table_x, const int *col_widths, int ncols) {
	const Order *o = (const Order *)record;
	char buf[64]; int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	sprintf(buf, "%d", row_idx + 1);
	outtextxy(x + 3, y_base + 8, buf); x += col_widths[0];
	outtextxy(x + 3, y_base + 8, o->order_id); x += col_widths[1];
	outtextxy(x + 3, y_base + 8, o->customer_name); x += col_widths[2];
	outtextxy(x + 3, y_base + 8, o->customer_phone); x += col_widths[3];
	const char *gname = strlen(o->goods_name) ? o->goods_name : o->goods_type;
	outtextxy(x + 3, y_base + 8, (char *)gname); x += col_widths[4];
	sprintf(buf, "%d", o->goods_quantity);
	outtextxy(x + 3, y_base + 8, buf);
}

/* ---- 构建筛选订单链表（调用者负责释放）---- */
static Order *build_filtered_order_list(const char *order_id,
                                         const char *cust_name,
                                         const char *phone,
                                         const char *goods_name,
                                         OrderStatus required_status) {
	Order *head = NULL, *tail = NULL;
	for (Order *p = order_list_head; p; p = p->next) {
		if (p->status != required_status) continue;
		if (!order_matches_query(p, order_id, cust_name, phone, goods_name)) continue;
		Order *copy = (Order *)malloc(sizeof(Order));
		memcpy(copy, p, sizeof(Order));
		copy->next = NULL;
		if (!head) head = copy;
		else tail->next = copy;
		tail = copy;
	}
	return head;
}

/* ---- 释放筛选链表 ---- */
static void free_filtered_order_list(Order *head) {
	while (head) { Order *t = head; head = head->next; free(t); }
}

/* ---- 从筛选链表中取第N个订单（1-based）---- */
static Order *get_nth_order(Order *head, int n) {
	while (head && --n > 0) head = head->next;
	return head;
}

static int count_orders_by_query(const char *order_id, const char *cust_name,
                                 const char *phone, const char *goods_name,
                                 OrderStatus required_status) {
	int total = 0;
	for (Order *p = order_list_head; p; p = p->next) {
		if (p->status != required_status) continue;
		if (order_matches_query(p, order_id, cust_name, phone, goods_name)) total++;
	}
	return total;
}

static Order *nth_order_by_query(const char *order_id, const char *cust_name,
                                 const char *phone, const char *goods_name,
                                 OrderStatus required_status, int index) {
	int cur = 0;
	for (Order *p = order_list_head; p; p = p->next) {
		if (p->status != required_status) continue;
		if (!order_matches_query(p, order_id, cust_name, phone, goods_name)) continue;
		if (cur++ == index) return p;
	}
	return NULL;
}

static void draw_order_select_table(const char *order_id, const char *cust_name,
                                    const char *phone, const char *goods_name,
                                    OrderStatus required_status,
                                    int page, int selected, const char *empty_text) {
	const int col_w[6] = {30, 132, 54, 88, 76, 40};
	const char *headers[6] = {"序号", "订单号", "客户", "电话", "货物", "数量"};
	const int header_h = 24, row_h = 25, page_size = 4;
	int total_w = 0, x, shown = 0, idx = 0;
	char buf[64];
	for (int i = 0; i < 6; i++) total_w += col_w[i];
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = UI_PANEL_Y + 145;

	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	setfillcolor(BG_TABLE_HDR);
	x = table_x;
	for (int c = 0; c < 6; c++) {
		fillrectangle(x, table_y, x + col_w[c], table_y + header_h);
		setlinecolor(GRAY_LINE);
		rectangle(x, table_y, x + col_w[c], table_y + header_h);
		outtextxy(x + 4, table_y + 6, (char *)headers[c]);
		x += col_w[c];
	}

	int start = page * page_size;
	for (Order *p = order_list_head; p && shown < page_size; p = p->next) {
		if (p->status != required_status) continue;
		if (!order_matches_query(p, order_id, cust_name, phone, goods_name)) continue;
		if (idx++ < start) continue;

		int y = table_y + header_h + shown * row_h;
		setfillcolor(shown == selected ? LIGHT_CYAN : WHITE_COLOR);
		fillrectangle(table_x, y, table_x + total_w, y + row_h);
		setlinecolor(GRAY_LINE);
		rectangle(table_x, y, table_x + total_w, y + row_h);

		x = table_x;
		sprintf(buf, "%d", start + shown + 1);
		outtextxy(x + 4, y + 6, buf); x += col_w[0];
		outtextxy(x + 4, y + 6, p->order_id); x += col_w[1];
		outtextxy(x + 4, y + 6, p->customer_name); x += col_w[2];
		outtextxy(x + 4, y + 6, p->customer_phone); x += col_w[3];
		outtextxy(x + 4, y + 6, strlen(p->goods_name) ? p->goods_name : p->goods_type); x += col_w[4];
		sprintf(buf, "%d", p->goods_quantity);
		outtextxy(x + 4, y + 6, buf);

		int vx = table_x;
		for (int c = 0; c < 6; c++) {
			vx += col_w[c];
			line(vx, y, vx, y + row_h);
		}
		shown++;
	}

	if (shown == 0) {
		settextcolor(TEXT_MUTED);
		outtextxy(table_x + 170, table_y + header_h + 42, (char *)empty_text);
	}
}

static void draw_order_filter_inputs(const char *title, int focus,
                                     const char *order_id, const char *cust_name,
                                     const char *phone, const char *goods_name) {
	const int label1_x = UI_PANEL_X + 24;
	const int input1_x = UI_PANEL_X + 82;
	const int label2_x = UI_PANEL_X + 228;
	const int input2_x = UI_PANEL_X + 272;
	const int if_w1 = 108, if_w2 = 86, if_h = 22;
	const int if_y0 = UI_PANEL_Y + 86, if_y1 = if_y0 + 28;
	drawQueryFrame(title, 405);
	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(label1_x, if_y0 + 2, "订单号:");
	drawTextBox(input1_x, if_y0, if_w1, if_h, order_id, focus == 0);
	outtextxy(label2_x, if_y0 + 2, "客户:");
	drawTextBox(input2_x, if_y0, if_w2, if_h, cust_name, focus == 1);
	outtextxy(label1_x, if_y1 + 2, "电话:");
	drawTextBox(input1_x, if_y1, if_w1, if_h, phone, focus == 2);
	outtextxy(label2_x, if_y1 + 2, "货物:");
	drawTextBox(input2_x, if_y1, if_w2, if_h, goods_name, focus == 3);
	drawQueryButton(UI_PANEL_X + 374, if_y0, 46, 26);
}


static void approve_selected_order(const char *order_id, const char *cust_name,
                                   const char *phone, const char *goods_name,
                                   int page, int selected, int page_size) {
	Order *real = nth_order_by_query(order_id, cust_name, phone, goods_name,
	                                 ORDER_PENDING_REVIEW, page * page_size + selected);
	if (!real) {
		MessageBoxA(GetHWnd(), "请先选择订单", "提示", MB_OK | MB_ICONWARNING);
		return;
	}
	real->status = ORDER_PENDING_OUT;
	order_svc_save();
	char action[ACTION_LEN];
	snprintf(action, sizeof(action), "审核通过: 订单%s", real->order_id);
	log_svc_add(current_user->id, current_user->name, action);
	MessageBoxA(GetHWnd(), "审核通过，订单已转为待出库", "提示", MB_OK | MB_ICONINFORMATION);
}

static void reject_selected_order(const char *order_id, const char *cust_name,
                                  const char *phone, const char *goods_name,
                                  const char *reject_reason,
                                  int page, int selected, int page_size) {
	if (strlen(reject_reason) == 0) {
		MessageBoxA(GetHWnd(), "请先填写驳回原因", "提示", MB_OK | MB_ICONWARNING);
		return;
	}
	Order *real = nth_order_by_query(order_id, cust_name, phone, goods_name,
	                                 ORDER_PENDING_REVIEW, page * page_size + selected);
	if (!real) {
		MessageBoxA(GetHWnd(), "请先选择订单", "提示", MB_OK | MB_ICONWARNING);
		return;
	}
	real->status = ORDER_REJECTED;
	strncpy_gbk_safe(real->reject_reason, reject_reason, REASON_LEN);
	real->reject_reason[REASON_LEN - 1] = '\0';
	order_svc_save();
	char action[ACTION_LEN];
	snprintf(action, sizeof(action), "驳回: 订单%s, 原因:%s", real->order_id, reject_reason);
	log_svc_add(current_user->id, current_user->name, action);
	MessageBoxA(GetHWnd(), "订单已驳回", "提示", MB_OK | MB_ICONINFORMATION);
}

/* ========== 订单审核窗口 ========== */
void auditOrderWin() {
	char order_id[64] = {0}, cust_name[64] = {0};
	char phone[64] = {0}, goods_name[64] = {0};
	char reject_reason[100] = {0};
	int focus = 0, input_active = 1, page = 0, selected = 0, need_redraw = 1;
	const int input1_x = UI_PANEL_X + 82, input2_x = UI_PANEL_X + 272;
	const int if_w1 = 108, if_w2 = 86, if_h = 22;
	const int if_y0 = UI_PANEL_Y + 86, if_y1 = if_y0 + 28;
	const int btn_query_x = UI_PANEL_X + 374, btn_query_y = if_y0;
	const int table_x = UI_PANEL_X + 10, table_y = UI_PANEL_Y + 145;
	const int table_w = 420, header_h = 24, row_h = 25, page_size = 4;
	const int approve_x = UI_PANEL_X + 34, reject_x = UI_PANEL_X + 144;
	const int reason_x = UI_PANEL_X + 254, action_y = UI_PANEL_Y + 320;

	while (1) {
		int total = count_orders_by_query(order_id, cust_name, phone, goods_name, ORDER_PENDING_REVIEW);
		int pages = total ? (total + page_size - 1) / page_size : 1;
		if (page >= pages) page = pages - 1;
		int rows = total - page * page_size;
		if (rows > page_size) rows = page_size;
		if (selected >= rows) selected = rows > 0 ? rows - 1 : 0;
		if (need_redraw) {
			cleardevice();
			redraw_bg();
			window_clear_frame();
			window_set_card(1);
			draw_order_filter_inputs("智能物流管理系统订单审核界面", focus, order_id, cust_name, phone, goods_name);
			draw_order_select_table(order_id, cust_name, phone, goods_name, ORDER_PENDING_REVIEW,
			                        page, selected, "暂无待审核订单");
			drawPageText(UI_PANEL_Y + 285, pages, page);
			settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
			setfillcolor(PRIMARY); settextcolor(WHITE_COLOR); setlinecolor(BLACK_COLOR);
			fillrectangle(approve_x, action_y, approve_x + 95, action_y + 28);
			rectangle(approve_x, action_y, approve_x + 95, action_y + 28);
			outtextxy(approve_x + 15, action_y + 7, "审核通过");
			setfillcolor(RED_BTN);
			fillrectangle(reject_x, action_y, reject_x + 95, action_y + 28);
			rectangle(reject_x, action_y, reject_x + 95, action_y + 28);
			outtextxy(reject_x + 19, action_y + 7, "驳回订单");
			settextcolor(TEXT_MAIN);
			outtextxy(reason_x, action_y + 7, "原因:");
			drawTextBox(reason_x + 44, action_y + 3, 106, 24, reject_reason, focus == 4);
			settextcolor(TEXT_MUTED);
			outtextxy(UI_PANEL_X + 55, UI_PANEL_Y + 355, "Tab切换输入框，点击表格或按↑↓选择订单");
			outtextxy(UI_PANEL_X + 90, UI_PANEL_Y + 373, "←→翻页，A通过，R驳回，Esc返回");
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *fields[5] = {order_id, cust_name, phone, goods_name, reject_reason};
		int max_lens[5] = {23, 31, 15, 31, 90};
		char *buf = fields[focus];
		int max_len = max_lens[focus];
		if (msg.message == WM_LBUTTONDOWN) {
			int mx = msg.x, my = msg.y;
			if (inRect(mx, my, input1_x, if_y0, if_w1, if_h)) { focus = 0; input_reset_pending(); input_active = 1; need_redraw = 1; }
			else if (inRect(mx, my, input2_x, if_y0, if_w2, if_h)) { focus = 1; input_reset_pending(); input_active = 1; need_redraw = 1; }
			else if (inRect(mx, my, input1_x, if_y1, if_w1, if_h)) { focus = 2; input_reset_pending(); input_active = 1; need_redraw = 1; }
			else if (inRect(mx, my, input2_x, if_y1, if_w2, if_h)) { focus = 3; input_reset_pending(); input_active = 1; need_redraw = 1; }
			else if (inRect(mx, my, reason_x + 44, action_y + 3, 106, 24)) { focus = 4; input_reset_pending(); input_active = 1; need_redraw = 1; }
			else if (inRect(mx, my, btn_query_x, btn_query_y, 46, 26)) { input_active = 0; page = 0; selected = 0; need_redraw = 1; }
			else if (inRect(mx, my, table_x, table_y + header_h, table_w, row_h * rows)) {
				selected = (my - table_y - header_h) / row_h;
				input_active = 0;
				need_redraw = 1;
			}
			else if (inRect(mx, my, approve_x, action_y, 95, 28) && total > 0) {
				approve_selected_order(order_id, cust_name, phone, goods_name, page, selected, page_size);
				selected = 0; input_active = 0; need_redraw = 1;
			}
			else if (inRect(mx, my, reject_x, action_y, 95, 28) && total > 0) {
				reject_selected_order(order_id, cust_name, phone, goods_name, reject_reason,
				                      page, selected, page_size);
				reject_reason[0] = '\0';
				selected = 0; input_active = 0; need_redraw = 1;
			}
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB) { focus = (focus + 1) % 5; input_reset_pending(); input_active = 1; need_redraw = 1; }
			else if (msg.vkcode == VK_UP && selected > 0) { input_active = 0; selected--; need_redraw = 1; }
			else if (msg.vkcode == VK_DOWN && selected < rows - 1) { input_active = 0; selected++; need_redraw = 1; }
			else if (msg.vkcode == VK_LEFT && page > 0) { input_active = 0; page--; selected = 0; need_redraw = 1; }
			else if (msg.vkcode == VK_RIGHT && page < pages - 1) { input_active = 0; page++; selected = 0; need_redraw = 1; }
			else if (!input_active && (msg.vkcode == 'A' || msg.vkcode == 'a') && total > 0) {
				approve_selected_order(order_id, cust_name, phone, goods_name, page, selected, page_size);
				selected = 0; need_redraw = 1;
			}
			else if (!input_active && (msg.vkcode == 'R' || msg.vkcode == 'r') && total > 0) {
				reject_selected_order(order_id, cust_name, phone, goods_name, reject_reason,
				                      page, selected, page_size);
				reject_reason[0] = '\0';
				selected = 0; need_redraw = 1;
			}
			else if (msg.vkcode == VK_RETURN) {
				input_active = 0;
				page = 0;
				selected = 0;
				need_redraw = 1;
			}
			else if (msg.vkcode == VK_BACK && input_active) {
				if (input_delete_last_char(buf)) { page = 0; selected = 0; need_redraw = 1; }
			}
		}
		else if (msg.message == WM_CHAR) {
			if (input_active) {
				int filters[] = {INPUT_FILTER_ALNUM, INPUT_FILTER_CHINESE, INPUT_FILTER_PHONE, INPUT_FILTER_CHINESE, INPUT_FILTER_CHINESE};
				int filter = filters[focus];
				if (input_append_char(buf, max_len, msg.ch, filter)) {
					page = 0;
					selected = 0;
					need_redraw = 1;
				}
			}
		}
	}
}



/* ========== 订单修改窗口 ========== */
static int goods_type_to_combo_index(const char *gtype) {
	const char *names[] = {"普通", "易碎", "冷链", "危险品"};
	for (int i = 0; i < 4; i++)
		if (strcmp(gtype, names[i]) == 0) return i;
	return 0;
}

/* ---- 进入编辑表单（预填充）---- */
static void modifyOrderEditForm(Order *o) {
	char meta_left[100], meta_right[100];
	sprintf(meta_left, "当前用户: %s(%s)", current_user->name, role_to_string(current_user->role));
	sprintf(meta_right, "登录时间: %s", login_time_str[0] ? login_time_str : "----");

	window_clear_frame();
	window_set_card(0);
	window_set_frame(135, 55, 520, 500);

	char goods_weight_buf[16], goods_qty_buf[16], goods_volume_buf[16];
	sprintf(goods_weight_buf, "%s", o->goods_weight);
	sprintf(goods_qty_buf, "%d", o->goods_quantity);
	sprintf(goods_volume_buf, "%s", o->goods_volume);

	WINDOW_T form = {
		135, 55, 520, 500, WHITE_COLOR, 27, {
			{275, 155, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0, 1},
			{275, 183, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, INPUT_FILTER_PHONE},
			{275, 211, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{275, 239, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{275, 267, 185, 24, "普通|易碎|冷链|危险品",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
			{275, 295, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{275, 323, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, INPUT_FILTER_DIGITS},
			{275, 351, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{275, 379, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{275, 407, 245, 24, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{225, 460, 100, 30, "保存",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{390, 460, 100, 30, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{245, 78, 300, 26, "智能物流管理系统订单修改界面",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{155, 115, 205, 22, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{385, 115, 240, 22, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 140, 100, 22, "订单号:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{275, 140, 230, 22, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 160, 100, 22, "客户姓名:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 188, 100, 22, "联系电话:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 216, 100, 22, "客户地址:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 244, 100, 22, "货物名称:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 272, 100, 22, "货物类型:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 300, 100, 22, "货物重量:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 328, 100, 22, "货物数量:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 356, 100, 22, "货物体积:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 384, 100, 22, "发货地址:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{165, 412, 100, 22, "收货地址:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
		}
	};

	/* 预填充所有字段 */
	strcpy(form.controls[0].text, o->customer_name);
	strcpy(form.controls[1].text, o->customer_phone);
	strcpy(form.controls[2].text, o->customer_addr);
	strcpy(form.controls[3].text, o->goods_name);
	form.controls[4].sel_index = goods_type_to_combo_index(o->goods_type);
	strcpy(form.controls[5].text,  goods_weight_buf);
	strcpy(form.controls[6].text,  goods_qty_buf);
	strcpy(form.controls[7].text,  goods_volume_buf);
	strcpy(form.controls[8].text,  o->from_addr);
	strcpy(form.controls[9].text,  o->to_addr);
	strcpy(form.controls[13].text, meta_left);
	strcpy(form.controls[14].text, meta_right);
	strcpy(form.controls[16].text, o->order_id);

	while (1) {
		window_show(form);
		form = window_run(form);

		if (form.current == 10) {
			char *name  = form.controls[0].text;
			char *phone = form.controls[1].text;
			char *cust_addr = form.controls[2].text;
			char *goods_name = form.controls[3].text;
			int  gtype  = form.controls[4].sel_index;
			char *weight = form.controls[5].text;
			char *qty = form.controls[6].text;
			char *volume = form.controls[7].text;
			char *from  = form.controls[8].text;
			char *to    = form.controls[9].text;

			if (strlen(name) == 0 || strlen(phone) == 0 ||
			    strlen(cust_addr) == 0 || strlen(goods_name) == 0 ||
			    strlen(weight) == 0 || strlen(qty) == 0 ||
			    strlen(volume) == 0 || strlen(from) == 0 || strlen(to) == 0) {
				MessageBoxA(GetHWnd(), "订单修改信息不能为空", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (atoi(qty) <= 0) {
				MessageBoxA(GetHWnd(), "货物数量必须为正整数", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			/* 二次校验状态 */
			Order *real = order_svc_find_by_id(o->order_id);
			if (!real || real->status != ORDER_PENDING_REVIEW) {
				MessageBoxA(GetHWnd(), "仅待审核状态可修改", "提示", MB_OK | MB_ICONWARNING);
				return;
			}

			Order new_data;
			memset(&new_data, 0, sizeof(Order));
			strncpy_gbk_safe(new_data.customer_name, name, NAME_LEN);
			strncpy(new_data.customer_phone, phone, PHONE_LEN - 1);
			strncpy_gbk_safe(new_data.customer_addr, cust_addr, ADDR_LEN);
			strncpy_gbk_safe(new_data.from_addr, from, ADDR_LEN);
			strncpy_gbk_safe(new_data.to_addr, to, ADDR_LEN);
			strncpy_gbk_safe(new_data.goods_name, goods_name, NAME_LEN);
			strcpy(new_data.goods_type, goods_type_to_string(gtype));
			strncpy(new_data.goods_weight, weight, sizeof(new_data.goods_weight) - 1);
			new_data.goods_quantity = atoi(qty);
			strncpy(new_data.goods_volume, volume, sizeof(new_data.goods_volume) - 1);

			int ret = order_svc_update(o->order_id, &new_data);
			if (ret == 0) {
				char msg[256];
				sprintf(msg, "订单 %s 修改成功", o->order_id);
				MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
				return;
			} else {
				MessageBoxA(GetHWnd(), "修改失败，订单状态可能已变更", "提示", MB_OK | MB_ICONWARNING);
			}
		}
		else if (form.current == 11) {
			return;    /* 返回查询列表 */
		}
		else {
			return;    /* Esc */
		}
	}
}

void modifyOrderWin() {
	char order_id[64] = {0}, cust_name[64] = {0};
	char phone[64] = {0}, goods_name[64] = {0};
	int focus = 0, page = 0, selected = 0, need_redraw = 1;
	const int input1_x = UI_PANEL_X + 82, input2_x = UI_PANEL_X + 272;
	const int if_w1 = 108, if_w2 = 86, if_h = 22;
	const int if_y0 = UI_PANEL_Y + 86, if_y1 = if_y0 + 28;
	const int btn_query_x = UI_PANEL_X + 374, btn_query_y = if_y0;
	const int table_x = UI_PANEL_X + 10, table_y = UI_PANEL_Y + 145;
	const int table_w = 420, header_h = 24, row_h = 25, page_size = 4;
	const int edit_x = UI_PANEL_X + 140, edit_y = UI_PANEL_Y + 320;

	while (1) {
		int total = count_orders_by_query(order_id, cust_name, phone, goods_name, ORDER_PENDING_REVIEW);
		int pages = total ? (total + page_size - 1) / page_size : 1;
		if (page >= pages) page = pages - 1;
		int rows = total - page * page_size;
		if (rows > page_size) rows = page_size;
		if (selected >= rows) selected = rows > 0 ? rows - 1 : 0;
		if (need_redraw) {
			cleardevice();
			redraw_bg();
			window_clear_frame();
			window_set_card(1);
			draw_order_filter_inputs("智能物流管理系统订单修改界面", focus, order_id, cust_name, phone, goods_name);
			draw_order_select_table(order_id, cust_name, phone, goods_name, ORDER_PENDING_REVIEW,
			                        page, selected, "暂无可修改订单");
			drawPageText(UI_PANEL_Y + 285, pages, page);
			settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
			setfillcolor(PRIMARY); settextcolor(WHITE_COLOR); setlinecolor(BLACK_COLOR);
			fillrectangle(edit_x, edit_y, edit_x + 150, edit_y + 28);
			rectangle(edit_x, edit_y, edit_x + 150, edit_y + 28);
			outtextxy(edit_x + 31, edit_y + 7, "修改选中订单");
			settextcolor(TEXT_MUTED);
			outtextxy(UI_PANEL_X + 65, UI_PANEL_Y + 355, "Tab切换输入框，点击表格或按↑↓选择订单");
			outtextxy(UI_PANEL_X + 110, UI_PANEL_Y + 373, "←→翻页，Enter修改，Esc返回");
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *fields[4] = {order_id, cust_name, phone, goods_name};
		int max_lens[4] = {23, 31, 15, 31};
		char *buf = fields[focus];
		int max_len = max_lens[focus];
		int do_edit = 0;

		if (msg.message == WM_LBUTTONDOWN) {
			int mx = msg.x, my = msg.y;
			if (inRect(mx, my, input1_x, if_y0, if_w1, if_h)) { focus = 0; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, input2_x, if_y0, if_w2, if_h)) { focus = 1; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, input1_x, if_y1, if_w1, if_h)) { focus = 2; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, input2_x, if_y1, if_w2, if_h)) { focus = 3; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, btn_query_x, btn_query_y, 46, 26))
				{ page = 0; selected = 0; need_redraw = 1; }
			else if (inRect(mx, my, table_x, table_y + header_h, table_w, row_h * rows)) {
				selected = (my - table_y - header_h) / row_h;
				need_redraw = 1;
			}
			else if (inRect(mx, my, edit_x, edit_y, 150, 28))
				do_edit = 1;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB) { focus = (focus + 1) % 4; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_UP && selected > 0) { selected--; need_redraw = 1; }
			if (msg.vkcode == VK_DOWN && selected < rows - 1) { selected++; need_redraw = 1; }
			if (msg.vkcode == VK_LEFT && page > 0) { page--; selected = 0; need_redraw = 1; }
			if (msg.vkcode == VK_RIGHT && page < pages - 1) { page++; selected = 0; need_redraw = 1; }
			if (msg.vkcode == VK_RETURN) do_edit = 1;
			if (msg.vkcode == VK_BACK) {
				if (input_delete_last_char(buf)) { page = 0; selected = 0; need_redraw = 1; }
			}
		}
		else if (msg.message == WM_CHAR) {
			int filters[] = {INPUT_FILTER_ALNUM, INPUT_FILTER_CHINESE, INPUT_FILTER_PHONE, INPUT_FILTER_CHINESE};
			int filter = filters[focus];
			if (input_append_char(buf, max_len, msg.ch, filter)) {
				page = 0; selected = 0; need_redraw = 1;
			}
		}

		if (do_edit) {
			if (total <= 0) {
				MessageBoxA(GetHWnd(), "请先选择可修改订单", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			Order *real = nth_order_by_query(order_id, cust_name, phone, goods_name,
			                                 ORDER_PENDING_REVIEW, page * page_size + selected);
			if (!real) {
				MessageBoxA(GetHWnd(), "请先选择可修改订单", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			modifyOrderEditForm(real);
			need_redraw = 1;
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

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(r->goods_name) ? r->goods_name : r->goods_type);
	x += col_widths[2];

	outtextxy(x + CTRL_PADDING, y_base + 8, r->goods_type);
	x += col_widths[3];

	sprintf(buf, "%d", r->quantity);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[4];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          (char *)operation_type_to_string(r->op_type));
}

static void showInOutList(const InOutRecord *head) {
	const char *headers[] = {"ID", "订单号", "货物名称", "货物类型", "数量", "操作"};
	const int col_widths[] = {45, 155, 85, 75, 55, 60};
	window_show_table("出入库记录", headers, col_widths, 6,
	                  head, offsetof(InOutRecord, next), drawInOutRow, PAGE_SIZE);
}

/* ========== 订单跟踪窗口 ========== */
void trackOrderWin() {
	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		180, 80, 460, 420, WHITE_COLOR, 10, {
			{190, 90, 440, 30, "订单跟踪",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 140, 80, INPUT_H, "订单号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 140, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0, INPUT_FILTER_ALNUM},
			{190, 210, BTN_W, BTN_H, "查询订单详情",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 210, BTN_W, BTN_H, "查看出入库记录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{190, 280, BTN_W, BTN_H, "查看操作日志",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 280, BTN_W, BTN_H, "出入库流水",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{210, 370, BTN_W, BTN_H, "确认完成",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 370, BTN_W, BTN_H, "返回",
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
			        "订单号：%s\n客户：%s\n电话：%s\n客户地址：%s\n发货：%s\n收货：%s\n货物名称：%s\n货物类型：%s\n重量：%s\n数量：%d\n体积：%s\n状态：%s\n预计送达：%s\n驳回原因：%s\n用户ID：%d",
			        o->order_id, o->customer_name, o->customer_phone,
			        strlen(o->customer_addr) ? o->customer_addr : "--",
			        o->from_addr, o->to_addr,
			        strlen(o->goods_name) ? o->goods_name : o->goods_type,
			        o->goods_type,
			        strlen(o->goods_weight) ? o->goods_weight : "--",
			        o->goods_quantity,
			        strlen(o->goods_volume) ? o->goods_volume : "--",
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
			/* 确认完成 */
			char *oid = win.controls[2].text;
			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请先输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			int ret = order_svc_complete(oid);
			if (ret == -1)
				MessageBoxA(GetHWnd(), "订单不存在", "提示", MB_OK | MB_ICONWARNING);
			else if (ret == -2)
				MessageBoxA(GetHWnd(), "仅已送达状态可确认完成", "提示", MB_OK | MB_ICONWARNING);
			else {
				char msg[128];
				sprintf(msg, "订单 %s 已完成", oid);
				MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
			}
		}
		else if (win.current == 8) {
			return;
		}
	}
}
