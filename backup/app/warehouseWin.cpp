#include <graphics.h>
#include <conio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../public/common.h"
#include "../public/ui_config.h"
#include "../view/control.h"
#include "../service/warehouse_service.h"
#include "../service/order_service.h"
#include "orderWin.h"

/* ---- Pdf 绘制辅助函数 ---- */
static int inRect(int mx, int my, int x, int y, int w, int h) {
	return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

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

static void drawPageText(int y, int pages, int page) {
	char page_text[96];
	sprintf(page_text, "<-上页     共%d页 当前第%d页     ->下页", pages, page + 1);
	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + (UI_PANEL_W - textwidth(page_text)) / 2, y, page_text);
}

/* ========== 仓库配置窗口 ========== */
static void drawWarehouseRow(const void *record, int row_idx, int y_base,
                             int table_x, const int *col_widths, int ncols) {
	const Warehouse *w = (const Warehouse *)record;
	char buf[64];
	int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));

	sprintf(buf, "%d", w->id);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[0];

	outtextxy(x + CTRL_PADDING, y_base + 8, w->name);
	x += col_widths[1];

	outtextxy(x + CTRL_PADDING, y_base + 8, w->address);
	x += col_widths[2];

	sprintf(buf, "%d", w->manager_id);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
}

static void showWarehouseList(const Warehouse *head) {
	const char *headers[] = {"ID", "仓库名称", "地址", "负责人ID"};
	const int col_widths[] = {50, 120, 140, 80};
	window_show_table("仓库列表", headers, col_widths, 4,
	                  head, offsetof(Warehouse, next), drawWarehouseRow, PAGE_SIZE);
}

void warehouseConfigWin() {
	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		180, 100, 440, 380, WHITE_COLOR, 8, {
			{190, 110, 420, 30, "仓库配置",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 165, 80, INPUT_H, "仓库名称：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{290, 165, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0, 1},
			{190, 210, 80, INPUT_H, "仓库地址：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{290, 210, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0, 1},
			{195, 285, BTN_W, BTN_H, "新增仓库",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 285, BTN_W, BTN_H, "仓库列表",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{310, 345, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 5) {
			char *name = win.controls[2].text;
			char *addr = win.controls[4].text;

			if (strlen(name) == 0 || strlen(addr) == 0) {
				MessageBoxA(GetHWnd(), "仓库名称和地址不能为空", "提示",
				            MB_OK | MB_ICONWARNING);
				continue;
			}

			Warehouse *w = (Warehouse *)malloc(sizeof(Warehouse));
			memset(w, 0, sizeof(Warehouse));
			w->id = ++warehouse_id_counter;
			strncpy_gbk_safe(w->name, name, NAME_LEN);
			strncpy_gbk_safe(w->address, addr, ADDR_LEN);
			w->manager_id = current_user ? current_user->id : 0;
			w->next = warehouse_list_head;
			warehouse_list_head = w;

			warehouse_svc_save_all();

			char msg[256];
			sprintf(msg, "仓库 \"%s\" 添加成功！", name);
			MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);

			memset(win.controls[2].text, 0, INPUT_W);
			memset(win.controls[4].text, 0, INPUT_W);
		}
		else if (win.current == 6) {
			if (!warehouse_list_head)
				MessageBoxA(GetHWnd(), "暂无仓库记录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				showWarehouseList(warehouse_list_head);
		}
		else if (win.current == 7) {
			return;
		}
	}
}

/* ========== 库存表格行绘制回调 ========== */
static void drawInventoryRow(const void *record, int row_idx, int y_base,
                             int table_x, const int *col_widths, int ncols) {
	const Inventory *inv = (const Inventory *)record;
	char buf[64];
	int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));

	sprintf(buf, "%d", inv->id);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[0];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(inv->goods_name) ? inv->goods_name : inv->goods_type);
	x += col_widths[1];

	outtextxy(x + CTRL_PADDING, y_base + 8, inv->goods_type);
	x += col_widths[2];

	sprintf(buf, "%d", inv->quantity);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[3];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(inv->location_id) ? inv->location_id : "--");
	x += col_widths[4];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(inv->in_time) ? inv->in_time : "--");
}

static void showInventoryList(const Inventory *head) {
	const char *headers[] = {"ID", "货物名称", "货物类型", "库存数量", "货位", "最后入库"};
	const int col_widths[] = {45, 90, 80, 80, 70, 150};
	window_show_table("库存列表", headers, col_widths, 6,
	                  head, offsetof(Inventory, next), drawInventoryRow, PAGE_SIZE);
}

static int inventory_matches_query_pdf(const Inventory *inv,
                                       const char *goods_name,
                                       const char *location_id) {
	const char *name = strlen(inv->goods_name) ? inv->goods_name : inv->goods_type;
	if (goods_name[0] && strstr(name, goods_name) == NULL) return 0;
	if (location_id[0] && strstr(inv->location_id, location_id) == NULL) return 0;
	return 1;
}

static int inventory_query_total_pages(const char *goods_name, const char *location_id) {
	int total = 0;
	for (Inventory *p = inventory_list_head; p; p = p->next) {
		if (inventory_matches_query_pdf(p, goods_name, location_id)) total++;
	}
	return total ? (total + 2) / 3 : 1;
}

static void draw_inventory_query_pdf(const char *goods_name,
                                     const char *location_id,
                                     int focus,
                                     int page) {
	const int col_w[6] = {36, 78, 68, 66, 68, 82};
	const char *headers[6] = {"序号", "货物名称", "货物类型", "库存数量", "存储货位", "入库时间"};
	int header_h = 22, row_h = 22;
	int total_w = col_w[0] + col_w[1] + col_w[2] + col_w[3] + col_w[4] + col_w[5];
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = UI_PANEL_Y + 122;
	int start = page * 3;
	int shown = 0, idx = 0, x;
	char buf[64];
	char date_buf[16];

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统库存查询界面", 315);

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 35, UI_PANEL_Y + 91, "货物名称:");
	drawTextBox(UI_PANEL_X + 105, UI_PANEL_Y + 84, 100, 24, goods_name, focus == 0);
	outtextxy(UI_PANEL_X + 230, UI_PANEL_Y + 91, "货位编号:");
	drawTextBox(UI_PANEL_X + 300, UI_PANEL_Y + 84, 80, 24, location_id, focus == 1);
	drawQueryButton(UI_PANEL_X + 395, UI_PANEL_Y + 84, 65, 26);

	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	setfillcolor(BG_TABLE_HDR);
	x = table_x;
	for (int c = 0; c < 6; c++) {
		fillrectangle(x, table_y, x + col_w[c], table_y + header_h);
		setlinecolor(GRAY_LINE);
		rectangle(x, table_y, x + col_w[c], table_y + header_h);
		outtextxy(x + 4, table_y + 5, (char *)headers[c]);
		x += col_w[c];
	}

	for (Inventory *p = inventory_list_head; p && shown < 3; p = p->next) {
		if (!inventory_matches_query_pdf(p, goods_name, location_id)) continue;
		if (idx++ < start) continue;
		int y = table_y + header_h + shown * row_h;
		x = table_x;
		setfillcolor(WHITE_COLOR);
		fillrectangle(table_x, y, table_x + total_w, y + row_h);
		setlinecolor(GRAY_LINE);
		rectangle(table_x, y, table_x + total_w, y + row_h);
		settextcolor(TEXT_MAIN);

		sprintf(buf, "%d", p->id);
		outtextxy(x + 4, y + 5, buf); x += col_w[0];
		outtextxy(x + 4, y + 5, strlen(p->goods_name) ? p->goods_name : p->goods_type); x += col_w[1];
		outtextxy(x + 4, y + 5, p->goods_type); x += col_w[2];
		sprintf(buf, "%d", p->quantity);
		outtextxy(x + 4, y + 5, buf); x += col_w[3];
		outtextxy(x + 4, y + 5, strlen(p->location_id) ? p->location_id : "--"); x += col_w[4];
		if (strlen(p->in_time) >= 10) {
			strncpy(date_buf, p->in_time, 10);
			date_buf[10] = '\0';
			outtextxy(x + 4, y + 5, date_buf);
		} else {
			outtextxy(x + 4, y + 5, strlen(p->in_time) ? p->in_time : "--");
		}

		int vx = table_x;
		for (int c = 0; c < 6; c++) {
			vx += col_w[c];
			line(vx, y, vx, y + row_h);
		}
		shown++;
	}

	if (shown == 0) {
		settextcolor(TEXT_MUTED);
		outtextxy(table_x + 160, table_y + header_h + 30, "暂无匹配库存");
	}

	drawPageText(UI_PANEL_Y + 240, inventory_query_total_pages(goods_name, location_id), page);
}

static void inventoryQueryWinPdf() {
	char goods_name[64] = {0};
	char location_id[64] = {0};
	int focus = 0, page = 0;
	int need_redraw = 1;

	while (1) {
		int pages = inventory_query_total_pages(goods_name, location_id);
		if (page >= pages) page = pages - 1;
		if (need_redraw) {
			draw_inventory_query_pdf(goods_name, location_id, focus, page);
			need_redraw = 0;
		}
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *buf = focus == 0 ? goods_name : location_id;
		int max_len = focus == 0 ? 31 : 15;

		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x >= UI_PANEL_X + 105 && msg.x <= UI_PANEL_X + 205 &&
			    msg.y >= UI_PANEL_Y + 84 && msg.y <= UI_PANEL_Y + 108) {
				if (focus != 0) { focus = 0; input_reset_pending(); need_redraw = 1; }
			}
			else if (msg.x >= UI_PANEL_X + 300 && msg.x <= UI_PANEL_X + 380 &&
			         msg.y >= UI_PANEL_Y + 84 && msg.y <= UI_PANEL_Y + 108) {
				if (focus != 1) { focus = 1; input_reset_pending(); need_redraw = 1; }
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
				if (input_delete_last_char(buf)) { page = 0; need_redraw = 1; }
			}
			if (msg.vkcode == VK_LEFT && page > 0) { page--; need_redraw = 1; }
			if (msg.vkcode == VK_RIGHT && page < pages - 1) { page++; need_redraw = 1; }
		}
		else if (msg.message == WM_CHAR) {
			int filter = (focus == 0) ? INPUT_FILTER_CHINESE : INPUT_FILTER_ALNUM;
			if (input_append_char(buf, max_len, msg.ch, filter)) {
				page = 0;
				need_redraw = 1;
			}
		}
	}
}

/* ========== 出入库记录表格 ========== */
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

/* ========== 仓储管理子菜单 ========== */
/* ---- 仓库窗口前向声明 ---- */
static void inboundWin();
static void outboundWin();
static void inventoryWin();
static void stocktakingWin();

void warehouseMgmtWin() {
	char meta_left[100], meta_right[100];

	window_clear_frame();
	window_set_card(0);
	window_set_frame(90, 50, 620, 440);

	sprintf(meta_left, "当前用户: %s(%s)",
	        current_user->name, role_to_string(current_user->role));
	sprintf(meta_right, "登录时间: %s",
	        login_time_str[0] ? login_time_str : "----");

	WINDOW_T win = {
		90, 50, 620, 440, WHITE_COLOR, 9, {
			{205, 75, 390, 30, "智能物流管理系统仓储管理界面",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{125, 130, 240, 24, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{410, 130, 260, 24, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{135, 180, 190, 36, "1. 货物入库",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{410, 180, 190, 36, "2. 货物出库",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{135, 235, 190, 36, "3. 库存查询",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 235, 190, 36, "4. 库存盘点",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{270, 290, 190, 36, "5. 返回上级",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{135, 365, 470, 24, "操作说明: 通过上下键切换菜单，按Enter键进入",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
		}
	};
	strcpy(win.controls[1].text, meta_left);
	strcpy(win.controls[2].text, meta_right);

	while (1) {
		window_set_card(0);
		window_set_frame(90, 50, 620, 440);
		window_show(win);
		win = window_run(win);
		switch (win.current) {
		case 3: inboundWin(); break;
		case 4: outboundWin(); break;
		case 5: inventoryWin(); break;
		case 6: stocktakingWin(); break;
		case 7: return;
		default: return;
		}
	}
}

/* ---- 仓库订单匹配（含货物类型）---- */
static int wh_order_matches(const Order *o, const char *order_id,
                             const char *cust_name, const char *phone,
                             const char *goods_name, const char *goods_type) {
	if (order_id[0] && strstr(o->order_id, order_id) == NULL) return 0;
	if (cust_name[0] && strstr(o->customer_name, cust_name) == NULL) return 0;
	if (phone[0] && strstr(o->customer_phone, phone) == NULL) return 0;
	if (goods_name[0]) {
		const char *gname = strlen(o->goods_name) ? o->goods_name : o->goods_type;
		if (strstr(gname, goods_name) == NULL) return 0;
	}
	if (goods_type[0] && strstr(o->goods_type, goods_type) == NULL) return 0;
	return 1;
}

/* ---- 仓库筛选订单链表 ---- */
static Order *wh_build_filtered_list(const char *order_id,
                                      const char *cust_name,
                                      const char *phone,
                                      const char *goods_name,
                                      const char *goods_type,
                                      OrderStatus required_status) {
	Order *head = NULL, *tail = NULL;
	for (Order *p = order_list_head; p; p = p->next) {
		if (p->status != required_status) continue;
		if (!wh_order_matches(p, order_id, cust_name, phone, goods_name, goods_type)) continue;
		Order *copy = (Order *)malloc(sizeof(Order));
		memcpy(copy, p, sizeof(Order));
		copy->next = NULL;
		if (!head) head = copy;
		else tail->next = copy;
		tail = copy;
	}
	return head;
}

static void wh_free_filtered(Order *head) {
	while (head) { Order *t = head; head = head->next; free(t); }
}

/* ---- 6列入库表格（序号/订单号/客户/货物名称/货物类型/数量）---- */
static void drawOrderRowWh(const void *record, int row_idx, int y_base,
                            int table_x, const int *col_widths, int ncols) {
	const Order *o = (const Order *)record;
	char buf[64]; int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	sprintf(buf, "%d", row_idx + 1);
	outtextxy(x + 3, y_base + 8, buf); x += col_widths[0];
	outtextxy(x + 3, y_base + 8, o->order_id); x += col_widths[1];
	outtextxy(x + 3, y_base + 8, o->customer_name); x += col_widths[2];
	const char *gname = strlen(o->goods_name) ? o->goods_name : o->goods_type;
	outtextxy(x + 3, y_base + 8, (char *)gname); x += col_widths[3];
	outtextxy(x + 3, y_base + 8, o->goods_type); x += col_widths[4];
	sprintf(buf, "%d", o->goods_quantity);
	outtextxy(x + 3, y_base + 8, buf);
}

/* ---- 出库表格（含库存列）---- */
static void drawOrderRowWhInv(const void *record, int row_idx, int y_base,
                               int table_x, const int *col_widths, int ncols) {
	const Order *o = (const Order *)record;
	const char *gname = strlen(o->goods_name) ? o->goods_name : o->goods_type;
	const char *gtype = strlen(o->goods_type) ? o->goods_type : gname;
	char buf[64]; int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	sprintf(buf, "%d", row_idx + 1);
	outtextxy(x + 3, y_base + 8, buf); x += col_widths[0];
	outtextxy(x + 3, y_base + 8, o->order_id); x += col_widths[1];
	outtextxy(x + 3, y_base + 8, o->customer_name); x += col_widths[2];
	outtextxy(x + 3, y_base + 8, (char *)gname); x += col_widths[3];
	outtextxy(x + 3, y_base + 8, o->goods_type); x += col_widths[4];
	sprintf(buf, "%d", o->goods_quantity); outtextxy(x + 3, y_base + 8, buf); x += col_widths[5];
	/* 查库存 */
	Warehouse *wh = warehouse_svc_get_default();
	Inventory *inv = wh ? inventory_svc_find(gname, gtype, wh->id) : NULL;
	sprintf(buf, "%d", inv ? inv->quantity : 0);
	outtextxy(x + 3, y_base + 8, buf);
}

/* ========== 入库窗口 ========== */
static void inboundWin() {
	char order_id[64] = {0}, cust_name[64] = {0}, phone[64] = {0};
	char goods_name[64] = {0}, goods_type[64] = {0};
	int focus = 0, need_redraw = 1;
	const int if_x = UI_PANEL_X + 68, if_y0 = UI_PANEL_Y + 86;
	const int if_w = 100, if_h = 22, if_y1 = if_y0 + 28;
	const int btn_qx = UI_PANEL_X + 372, btn_qy = if_y0;

	while (1) {
		const int btn_y = UI_PANEL_Y + 195;
		if (need_redraw) {
			cleardevice();
			redraw_bg();
			window_clear_frame();
			window_set_card(1);
			drawQueryFrame("智能物流管理系统入库管理界面", 240);

			settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
			settextcolor(TEXT_MAIN);
			outtextxy(UI_PANEL_X + 12, if_y0 + 2, "订单号:");
			drawTextBox(if_x, if_y0, if_w, if_h, order_id, focus == 0);
			outtextxy(UI_PANEL_X + 178, if_y0 + 2, "客户:");
			drawTextBox(if_x + 120, if_y0, if_w, if_h, cust_name, focus == 1);
			outtextxy(UI_PANEL_X + 12, if_y1 + 2, "电话:");
			drawTextBox(if_x, if_y1, if_w, if_h, phone, focus == 2);
			outtextxy(UI_PANEL_X + 178, if_y1 + 2, "货物:");
			drawTextBox(if_x + 120, if_y1, if_w, if_h, goods_name, focus == 3);
			outtextxy(UI_PANEL_X + 12, if_y1 + 32, "类型:");
			drawTextBox(if_x, if_y1 + 28, if_w, if_h, goods_type, focus == 4);
			drawQueryButton(btn_qx, btn_qy, 56, 26);

			settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
			setlinecolor(BLACK_COLOR);
			setfillcolor(PRIMARY); settextcolor(WHITE_COLOR);
			rectangle(UI_PANEL_X + 110, btn_y, UI_PANEL_X + 270, btn_y + 26);
			outtextxy(UI_PANEL_X + 155, btn_y + 5, "查询可入库订单");
			outtextxy(UI_PANEL_X + 310, btn_y + 5, "返回(Esc)");
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *fields[5] = {order_id, cust_name, phone, goods_name, goods_type};
		int max_lens[5] = {23, 31, 15, 31, 15};
		char *buf = fields[focus];
		int max_len = max_lens[focus];
		int clicked_query = 0;

		if (msg.message == WM_LBUTTONDOWN) {
			int mx = msg.x, my = msg.y;
			if (inRect(mx, my, if_x, if_y0, if_w, if_h)) { focus = 0; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x + 120, if_y0, if_w, if_h)) { focus = 1; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x, if_y1, if_w, if_h)) { focus = 2; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x + 120, if_y1, if_w, if_h)) { focus = 3; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x, if_y1 + 28, if_w, if_h)) { focus = 4; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, btn_qx, btn_qy, 56, 26)) clicked_query = 1;
			else if (inRect(mx, my, UI_PANEL_X + 110, btn_y, 160, 26)) clicked_query = 1;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB) { focus = (focus + 1) % 5; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_UP) { focus = (focus + 2) % 5; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_DOWN) { focus = (focus + 3) % 5; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_RETURN) clicked_query = 1;
			if (msg.vkcode == VK_BACK) {
				if (input_delete_last_char(buf)) { need_redraw = 1; }
			}
		}
		else if (msg.message == WM_CHAR) {
			int filters[] = {INPUT_FILTER_ALNUM, INPUT_FILTER_CHINESE, INPUT_FILTER_PHONE, INPUT_FILTER_CHINESE, INPUT_FILTER_CHINESE};
			int filter = filters[focus];
			if (input_append_char(buf, max_len, msg.ch, filter)) {
				need_redraw = 1;
			}
		}

		if (!clicked_query) continue;

		Order *filtered = wh_build_filtered_list(order_id, cust_name, phone,
		                     goods_name, goods_type, ORDER_PENDING_OUT);
		if (!filtered) {
			MessageBoxA(GetHWnd(), "暂无可入库订单", "提示", MB_OK | MB_ICONINFORMATION);
			continue;
		}

		const char *headers[] = {"序号", "订单号", "客户", "货物名称", "货物类型", "数量"};
		const int col_w[] = {32, 130, 68, 72, 60, 46};
		int sel = window_show_table("待入库订单列表", headers, col_w, 6,
		                            filtered, offsetof(Order, next), drawOrderRowWh, 5);
		if (sel <= 0) { wh_free_filtered(filtered); continue; }

		Order *selected = filtered;
		for (int i = 1; i < sel && selected; i++) selected = selected->next;
		if (!selected) { wh_free_filtered(filtered); continue; }

		/* 入库确认表单 */
		window_clear_frame();
		window_set_card(1);
		const char *gname = strlen(selected->goods_name) ? selected->goods_name : selected->goods_type;
		char title[128];
		sprintf(title, "入库操作 — %s(%s) x%d", gname, selected->goods_type, selected->goods_quantity);

		WINDOW_T form = {
			180, 100, 440, 340, WHITE_COLOR, 8, {
				{190, 110, 420, 26, "",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 155, 90, INPUT_H, "订单号:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 155, 220, INPUT_H, "",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 195, 90, INPUT_H, "入库数量:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 195, 220, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
				{190, 235, 90, INPUT_H, "货位编号:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 235, 220, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
				{210, 295, 100, 30, "确认入库",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
				{350, 295, 100, 30, "返回",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			}
		};
		strcpy(form.controls[0].text, title);
		strcpy(form.controls[2].text, selected->order_id);
		sprintf(form.controls[4].text, "%d", selected->goods_quantity);

		int done = 0;
		while (!done) {
			window_show(form);
			form = window_run(form);
			if (form.current == 7) {
				char *qty_str = form.controls[4].text;
				char *loc = form.controls[6].text;
				int qty = atoi(qty_str);
				if (qty <= 0) {
					MessageBoxA(GetHWnd(), "入库数量必须大于0", "提示", MB_OK | MB_ICONWARNING);
					continue;
				}
				char err[256];
				int ret = inbound_svc_execute(selected->order_id, qty,
				                              strlen(loc) ? loc : "--", err, sizeof(err));
				if (ret == 0) {
					char msg[256];
					const char *nm = strlen(selected->goods_name) ? selected->goods_name : selected->goods_type;
					sprintf(msg, "入库成功！\n订单号: %s\n货物: %s x%d", selected->order_id, nm, qty);
					MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
					done = 1;
				} else {
					MessageBoxA(GetHWnd(), err, "入库失败", MB_OK | MB_ICONERROR);
				}
			} else { done = 1; }
		}
		wh_free_filtered(filtered);
		need_redraw = 1;
	}
}

/* ========== 出库窗口 ========== */
static void outboundWin() {
	char order_id[64] = {0}, cust_name[64] = {0};
	char goods_name[64] = {0}, goods_type[64] = {0};
	int focus = 0, need_redraw = 1;
	const int if_x = UI_PANEL_X + 68, if_y0 = UI_PANEL_Y + 86;
	const int if_w = 100, if_h = 22, if_y1 = if_y0 + 28;
	const int btn_qx = UI_PANEL_X + 372, btn_qy = if_y0;

	while (1) {
		const int btn_y = UI_PANEL_Y + 195;
		if (need_redraw) {
			cleardevice();
			redraw_bg();
			window_clear_frame();
			window_set_card(1);
			drawQueryFrame("智能物流管理系统出库管理界面", 240);

			settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
			settextcolor(TEXT_MAIN);
			outtextxy(UI_PANEL_X + 12, if_y0 + 2, "订单号:");
			drawTextBox(if_x, if_y0, if_w, if_h, order_id, focus == 0);
			outtextxy(UI_PANEL_X + 178, if_y0 + 2, "客户:");
			drawTextBox(if_x + 120, if_y0, if_w, if_h, cust_name, focus == 1);
			outtextxy(UI_PANEL_X + 12, if_y1 + 2, "货物:");
			drawTextBox(if_x, if_y1, if_w, if_h, goods_name, focus == 2);
			outtextxy(UI_PANEL_X + 178, if_y1 + 2, "类型:");
			drawTextBox(if_x + 120, if_y1, if_w, if_h, goods_type, focus == 3);
			drawQueryButton(btn_qx, btn_qy, 56, 26);

			settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
			setlinecolor(BLACK_COLOR);
			setfillcolor(PRIMARY); settextcolor(WHITE_COLOR);
			rectangle(UI_PANEL_X + 110, btn_y, UI_PANEL_X + 270, btn_y + 26);
			outtextxy(UI_PANEL_X + 155, btn_y + 5, "查询可出库订单");
			outtextxy(UI_PANEL_X + 310, btn_y + 5, "返回(Esc)");
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *fields[4] = {order_id, cust_name, goods_name, goods_type};
		int max_lens[4] = {23, 31, 31, 15};
		char *buf = fields[focus];
		int max_len = max_lens[focus];
		int clicked_query = 0;

		if (msg.message == WM_LBUTTONDOWN) {
			int mx = msg.x, my = msg.y;
			if (inRect(mx, my, if_x, if_y0, if_w, if_h)) { focus = 0; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x + 120, if_y0, if_w, if_h)) { focus = 1; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x, if_y1, if_w, if_h)) { focus = 2; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x + 120, if_y1, if_w, if_h)) { focus = 3; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, btn_qx, btn_qy, 56, 26)) clicked_query = 1;
			else if (inRect(mx, my, UI_PANEL_X + 110, btn_y, 160, 26)) clicked_query = 1;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB) { focus = (focus + 1) % 4; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_UP || msg.vkcode == VK_DOWN) { focus = (focus + 2) % 4; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_RETURN) clicked_query = 1;
			if (msg.vkcode == VK_BACK) {
				if (input_delete_last_char(buf)) { need_redraw = 1; }
			}
		}
		else if (msg.message == WM_CHAR) {
			int filters[] = {INPUT_FILTER_ALNUM, INPUT_FILTER_CHINESE, INPUT_FILTER_CHINESE, INPUT_FILTER_CHINESE};
			int filter = filters[focus];
			if (input_append_char(buf, max_len, msg.ch, filter)) {
				need_redraw = 1;
			}
		}

		if (!clicked_query) continue;

		Order *filtered = wh_build_filtered_list(order_id, cust_name, "",
		                     goods_name, goods_type, ORDER_PENDING_OUT);
		if (!filtered) {
			MessageBoxA(GetHWnd(), "暂无可出库订单", "提示", MB_OK | MB_ICONINFORMATION);
			continue;
		}

		const char *headers[] = {"序号", "订单号", "客户", "货物名称", "货物类型", "数量", "库存"};
		const int col_w[] = {30, 128, 62, 68, 56, 44, 44};
		int sel = window_show_table("待出库订单列表", headers, col_w, 7,
		                            filtered, offsetof(Order, next), drawOrderRowWhInv, 5);
		if (sel <= 0) { wh_free_filtered(filtered); continue; }

		Order *selected = filtered;
		for (int i = 1; i < sel && selected; i++) selected = selected->next;
		if (!selected) { wh_free_filtered(filtered); continue; }

		/* 出库确认表单 */
		window_clear_frame();
		window_set_card(1);
		const char *gname = strlen(selected->goods_name) ? selected->goods_name : selected->goods_type;
		const char *gtype = strlen(selected->goods_type) ? selected->goods_type : gname;
		Warehouse *wh = warehouse_svc_get_default();
		Inventory *inv = wh ? inventory_svc_find(gname, gtype, wh->id) : NULL;
		int stock = inv ? inv->quantity : 0;
		char title[128];
		sprintf(title, "出库操作 — %s(%s) 库存:%d", gname, gtype, stock);

		WINDOW_T form = {
			180, 100, 440, 340, WHITE_COLOR, 8, {
				{190, 110, 420, 26, "",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 155, 90, INPUT_H, "订单号:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 155, 220, INPUT_H, "",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 195, 90, INPUT_H, "出库数量:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 195, 220, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
				{190, 235, 90, INPUT_H, "货位编号:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 235, 220, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
				{210, 295, 100, 30, "确认出库",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
				{350, 295, 100, 30, "返回",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			}
		};
		strcpy(form.controls[0].text, title);
		strcpy(form.controls[2].text, selected->order_id);
		sprintf(form.controls[4].text, "%d", selected->goods_quantity);

		int done = 0;
		while (!done) {
			window_show(form);
			form = window_run(form);
			if (form.current == 7) {
				char *qty_str = form.controls[4].text;
				char *loc = form.controls[6].text;
				int qty = atoi(qty_str);
				if (qty <= 0) {
					MessageBoxA(GetHWnd(), "出库数量必须大于0", "提示", MB_OK | MB_ICONWARNING);
					continue;
				}
				/* 重新检查库存 */
				inv = wh ? inventory_svc_find(gname, gtype, wh->id) : NULL;
				stock = inv ? inv->quantity : 0;
				if (qty > stock) {
					char err[128];
					sprintf(err, "库存不足：当前库存 %d，需要 %d", stock, qty);
					MessageBoxA(GetHWnd(), err, "库存不足", MB_OK | MB_ICONWARNING);
					continue;
				}
				char err[256];
				int ret = outbound_svc_execute(selected->order_id, qty,
				                               strlen(loc) ? loc : "--", err, sizeof(err));
				if (ret == 0) {
					char msg[256];
					sprintf(msg, "出库成功！订单 %s 已转为\"运输中\"", selected->order_id);
					MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
					done = 1;
				} else {
					MessageBoxA(GetHWnd(), err, "出库失败", MB_OK | MB_ICONERROR);
				}
			} else { done = 1; }
		}
		wh_free_filtered(filtered);
		need_redraw = 1;
	}
}

/* ========== 库存查询窗口 ========== */
static void inventoryWin() {
	inventoryQueryWinPdf();
}

/* ========== 库存盘点窗口 ========== */
static int inv_matches_query(const Inventory *inv, const char *inv_id,
                              const char *goods_name, const char *goods_type,
                              const char *location) {
	char id_buf[16];
	sprintf(id_buf, "%d", inv->id);
	if (inv_id[0] && strstr(id_buf, inv_id) == NULL) return 0;
	if (goods_name[0]) {
		const char *nm = strlen(inv->goods_name) ? inv->goods_name : inv->goods_type;
		if (strstr(nm, goods_name) == NULL) return 0;
	}
	if (goods_type[0] && strstr(inv->goods_type, goods_type) == NULL) return 0;
	if (location[0] && inv->location_id[0] && strstr(inv->location_id, location) == NULL) return 0;
	return 1;
}

static Inventory *inv_build_filtered(const char *inv_id, const char *goods_name,
                                      const char *goods_type, const char *location) {
	Inventory *head = NULL, *tail = NULL;
	for (Inventory *p = inventory_list_head; p; p = p->next) {
		if (!inv_matches_query(p, inv_id, goods_name, goods_type, location)) continue;
		Inventory *copy = (Inventory *)malloc(sizeof(Inventory));
		memcpy(copy, p, sizeof(Inventory));
		copy->next = NULL;
		if (!head) head = copy;
		else tail->next = copy;
		tail = copy;
	}
	return head;
}

static void inv_free_filtered(Inventory *head) {
	while (head) { Inventory *t = head; head = head->next; free(t); }
}

static void drawStockRow(const void *record, int row_idx, int y_base,
                          int table_x, const int *col_widths, int ncols) {
	const Inventory *inv = (const Inventory *)record;
	char buf[64]; int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
	sprintf(buf, "%d", inv->id);
	outtextxy(x + 3, y_base + 8, buf); x += col_widths[0];
	const char *nm = strlen(inv->goods_name) ? inv->goods_name : inv->goods_type;
	outtextxy(x + 3, y_base + 8, (char *)nm); x += col_widths[1];
	outtextxy(x + 3, y_base + 8, inv->goods_type); x += col_widths[2];
	sprintf(buf, "%d", inv->quantity); outtextxy(x + 3, y_base + 8, buf); x += col_widths[3];
	outtextxy(x + 3, y_base + 8, strlen(inv->location_id) ? inv->location_id : "--"); x += col_widths[4];
	if (strlen(inv->in_time) >= 10) { strncpy(buf, inv->in_time, 10); buf[10] = '\0'; }
	else { strcpy(buf, strlen(inv->in_time) ? inv->in_time : "--"); }
	outtextxy(x + 3, y_base + 8, buf);
}

static void stocktakingWin() {
	char inv_id[16] = {0}, goods_name[64] = {0};
	char goods_type[64] = {0}, location[64] = {0};
	int focus = 0, need_redraw = 1;
	const int if_x = UI_PANEL_X + 68, if_y0 = UI_PANEL_Y + 86;
	const int if_w = 100, if_h = 22, if_y1 = if_y0 + 28;
	const int btn_qx = UI_PANEL_X + 372, btn_qy = if_y0;

	while (1) {
		const int btn_y = UI_PANEL_Y + 195;
		if (need_redraw) {
			cleardevice();
			redraw_bg();
			window_clear_frame();
			window_set_card(1);
			drawQueryFrame("智能物流管理系统库存盘点界面", 240);

			settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
			settextcolor(TEXT_MAIN);
			outtextxy(UI_PANEL_X + 12, if_y0 + 2, "库存ID:");
			drawTextBox(if_x, if_y0, if_w, if_h, inv_id, focus == 0);
			outtextxy(UI_PANEL_X + 178, if_y0 + 2, "货物:");
			drawTextBox(if_x + 120, if_y0, if_w, if_h, goods_name, focus == 1);
			outtextxy(UI_PANEL_X + 12, if_y1 + 2, "类型:");
			drawTextBox(if_x, if_y1, if_w, if_h, goods_type, focus == 2);
			outtextxy(UI_PANEL_X + 178, if_y1 + 2, "货位:");
			drawTextBox(if_x + 120, if_y1, if_w, if_h, location, focus == 3);
			drawQueryButton(btn_qx, btn_qy, 56, 26);

			settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
			setlinecolor(BLACK_COLOR);
			setfillcolor(PRIMARY); settextcolor(WHITE_COLOR);
			rectangle(UI_PANEL_X + 130, btn_y, UI_PANEL_X + 250, btn_y + 26);
			outtextxy(UI_PANEL_X + 170, btn_y + 5, "查询库存");
			outtextxy(UI_PANEL_X + 310, btn_y + 5, "返回(Esc)");
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *fields[4] = {inv_id, goods_name, goods_type, location};
		int max_lens[4] = {15, 31, 15, 15};
		char *buf = fields[focus];
		int max_len = max_lens[focus];
		int clicked_query = 0;

		if (msg.message == WM_LBUTTONDOWN) {
			int mx = msg.x, my = msg.y;
			if (inRect(mx, my, if_x, if_y0, if_w, if_h)) { focus = 0; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x + 120, if_y0, if_w, if_h)) { focus = 1; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x, if_y1, if_w, if_h)) { focus = 2; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, if_x + 120, if_y1, if_w, if_h)) { focus = 3; input_reset_pending(); need_redraw = 1; }
			else if (inRect(mx, my, btn_qx, btn_qy, 56, 26)) clicked_query = 1;
			else if (inRect(mx, my, UI_PANEL_X + 130, btn_y, 120, 26)) clicked_query = 1;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB) { focus = (focus + 1) % 4; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_UP || msg.vkcode == VK_DOWN) { focus = (focus + 2) % 4; input_reset_pending(); need_redraw = 1; }
			if (msg.vkcode == VK_RETURN) clicked_query = 1;
			if (msg.vkcode == VK_BACK) {
				if (input_delete_last_char(buf)) { need_redraw = 1; }
			}
		}
		else if (msg.message == WM_CHAR) {
			int filters[] = {INPUT_FILTER_ALNUM, INPUT_FILTER_CHINESE, INPUT_FILTER_CHINESE, INPUT_FILTER_ALNUM};
			int filter = filters[focus];
			if (input_append_char(buf, max_len, msg.ch, filter)) {
				need_redraw = 1;
			}
		}

		if (!clicked_query) continue;

		Inventory *filtered = inv_build_filtered(inv_id, goods_name, goods_type, location);
		if (!filtered) {
			MessageBoxA(GetHWnd(), "暂无匹配库存", "提示", MB_OK | MB_ICONINFORMATION);
			continue;
		}

		const char *headers[] = {"库存ID", "货物名称", "货物类型", "库存数量", "货位", "入库时间"};
		const int col_w[] = {50, 90, 72, 70, 65, 100};
		int sel = window_show_table("库存盘点列表", headers, col_w, 6,
		                            filtered, offsetof(Inventory, next), drawStockRow, 5);
		if (sel <= 0) { inv_free_filtered(filtered); continue; }

		Inventory *selected = filtered;
		for (int i = 1; i < sel && selected; i++) selected = selected->next;
		if (!selected) { inv_free_filtered(filtered); continue; }

		/* 盘点确认 */
		window_clear_frame();
		window_set_card(1);
		const char *nm = strlen(selected->goods_name) ? selected->goods_name : selected->goods_type;
		char info[256];
		sprintf(info, "库存ID:%d  货物:%s(%s)  系统数量:%d  货位:%s",
		        selected->id, nm, selected->goods_type, selected->quantity,
		        strlen(selected->location_id) ? selected->location_id : "--");

		WINDOW_T form = {
			180, 100, 440, 340, WHITE_COLOR, 7, {
				{190, 110, 420, 26, "库存盘点",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 148, 420, 24, "",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 190, 90, INPUT_H, "库存ID:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 190, 220, INPUT_H, "",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{190, 230, 90, INPUT_H, "实际数量:",
				 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
				{285, 230, 220, INPUT_H, "",
				 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
				{225, 290, 100, 30, "确认盘点",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
				{365, 290, 100, 30, "返回",
				 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			}
		};
		strcpy(form.controls[1].text, info);
		sprintf(form.controls[3].text, "%d", selected->id);

		int done = 0;
		while (!done) {
			window_show(form);
			form = window_run(form);
			if (form.current == 6) {
				char *qty_str = form.controls[5].text;
				if (strlen(qty_str) == 0) {
					MessageBoxA(GetHWnd(), "请输入实际数量", "提示", MB_OK | MB_ICONWARNING);
					continue;
				}
				int actual_qty = atoi(qty_str);
				if (actual_qty < 0) {
					MessageBoxA(GetHWnd(), "实际数量不能为负数", "提示", MB_OK | MB_ICONWARNING);
					continue;
				}
				int sys_qty = selected->quantity;
				int diff = actual_qty - sys_qty;
				char result[384];
				sprintf(result,
				        "===== 盘点结果 =====\n\n"
				        "货物名称: %s\n"
				        "货物类型: %s\n"
				        "系统数量: %d\n"
				        "实际数量: %d\n"
				        "差    异: %+d\n\n"
				        "%s",
				        nm, selected->goods_type, sys_qty, actual_qty, diff,
				        diff == 0 ? "盘点一致" : "! 存在差异，请核查！");
				int icon = diff == 0 ? MB_ICONINFORMATION : MB_ICONWARNING;
				MessageBoxA(GetHWnd(), result, "盘点结果", MB_OK | icon);
				done = 1;
			} else { done = 1; }
		}
		inv_free_filtered(filtered);
		need_redraw = 1;
	}
}
