#include <graphics.h>
#include <conio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../public/common.h"
#include "../public/ui_config.h"
#include "../view/control.h"
#include "../service/order_service.h"
#include "../service/transport_service.h"
#include "../service/warehouse_service.h"

/* ---- Pdf 绘制辅助函数 ---- */
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

/* ---- 工具函数 ---- */
static int inRect(int mx, int my, int x, int y, int w, int h) {
	return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

/* ---- 动态构建下拉文本 ---- */
static void build_vehicle_combo(char *buf, int bufsize) {
	buf[0] = '\0';
	for (Vehicle *v = vehicle_svc_list_all(); v; v = v->next) {
		if (v->status != VEHICLE_IDLE) continue;
		char item[128];
		sprintf(item, "%s %s(%s)%s",
		        v->plate_no,
		        v->type == VEHICLE_COLD_CHAIN ? "冷链车" : "箱式货车",
		        vehicle_status_to_string(v->status),
		        v->next && v->next->status == VEHICLE_IDLE ? "|" : "");
		if ((int)(strlen(buf) + strlen(item)) < bufsize)
			strcat(buf, item);
	}
}

static void build_driver_combo(char *buf, int bufsize) {
	buf[0] = '\0';
	int need_sep = 0;
	for (Driver *d = driver_svc_list_all(); d; d = d->next) {
		if (d->status != DRIVER_ON_DUTY) continue;
		char item[128];
		const char *sep = need_sep ? "|" : "";
		sprintf(item, "%s%d. %s(%s)", sep, d->id, d->name,
		        driver_status_to_string(d->status));
		if ((int)(strlen(buf) + strlen(item)) < bufsize)
			strcat(buf, item);
		need_sep = 1;
	}
}

static void build_route_combo(char *buf, int bufsize) {
	buf[0] = '\0';
	int need_sep = 0;
	for (Route *r = route_svc_list_all(); r; r = r->next) {
		char item[128];
		const char *sep = need_sep ? "|" : "";
		sprintf(item, "%s%d. %s->%s %.0fkm/%s", sep, r->id,
		        r->origin, r->destination, r->distance, r->estimated_time);
		if ((int)(strlen(buf) + strlen(item)) < bufsize)
			strcat(buf, item);
		need_sep = 1;
	}
}

/* ---- 待运输订单表格（Pdf模式）---- */
static int pending_order_matches(const Order *o, const char *order_id) {
	if (order_id[0] && strstr(o->order_id, order_id) == NULL) return 0;
	return 1;
}

static int pending_order_count(const char *order_id) {
	int total = 0;
	for (Order *p = order_list_head; p; p = p->next) {
		if (p->status != ORDER_PENDING_TRANSPORT) continue;
		if (pending_order_matches(p, order_id)) total++;
	}
	return total ? (total + 3) / 4 : 1;
}

static void draw_dispatch_query_pdf(const char *order_id, int page) {
	const int col_w[6] = {32, 145, 65, 70, 70, 65};
	const char *headers[6] = {"序号", "订单号", "货物名称", "发货地址", "收货地址", "货物重量"};
	int header_h = 22, row_h = 22;
	int total_w = 0;
	for (int c = 0; c < 6; c++) total_w += col_w[c];
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = UI_PANEL_Y + 150;
	int start = page * 4;
	int shown = 0, idx = 0, x;
	char buf[64];
	const char *goods_label;

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统运输调度界面", 315);

	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 25, UI_PANEL_Y + 91, "订单号:");
	drawTextBox(UI_PANEL_X + 82, UI_PANEL_Y + 84, 220, 24, order_id, 1);
	drawQueryButton(UI_PANEL_X + 550, UI_PANEL_Y + 84, 70, 30);

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	outtextxy(UI_PANEL_X + 25, UI_PANEL_Y + 125, "待运输订单列表:");

	setfillcolor(BG_TABLE_HDR);
	x = table_x;
	for (int c = 0; c < 6; c++) {
		fillrectangle(x, table_y, x + col_w[c], table_y + header_h);
		setlinecolor(GRAY_LINE);
		rectangle(x, table_y, x + col_w[c], table_y + header_h);
		outtextxy(x + 3, table_y + 5, (char *)headers[c]);
		x += col_w[c];
	}

	goods_label = NULL;
	for (Order *p = order_list_head; p && shown < 4; p = p->next) {
		if (p->status != ORDER_PENDING_TRANSPORT) continue;
		if (!pending_order_matches(p, order_id)) continue;
		if (idx++ < start) continue;

		goods_label = strlen(p->goods_name) ? p->goods_name : p->goods_type;
		int y = table_y + header_h + shown * row_h;
		setfillcolor(WHITE_COLOR);
		fillrectangle(table_x, y, table_x + total_w, y + row_h);
		setlinecolor(GRAY_LINE);
		rectangle(table_x, y, table_x + total_w, y + row_h);
		x = table_x;
		settextcolor(TEXT_MAIN);
		sprintf(buf, "%d", idx);
		outtextxy(x + 4, y + 5, buf); x += col_w[0];
		outtextxy(x + 4, y + 5, p->order_id); x += col_w[1];
		outtextxy(x + 4, y + 5, (char *)goods_label); x += col_w[2];
		outtextxy(x + 4, y + 5, p->from_addr); x += col_w[3];
		outtextxy(x + 4, y + 5, p->to_addr); x += col_w[4];
		outtextxy(x + 4, y + 5, p->goods_weight);
		int vx = table_x;
		for (int c = 0; c < 6; c++) {
			vx += col_w[c];
			line(vx, y, vx, y + row_h);
		}
		shown++;
	}
	if (shown == 0) {
		settextcolor(TEXT_MUTED);
		outtextxy(table_x + 150, table_y + header_h + 32, "暂无待运输订单");
	}

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	drawPageText(UI_PANEL_Y + 288, pending_order_count(order_id), page);
}

/* ---- 运输调度主窗口（图12）---- */
static void dispatchWin() {
	char order_id[64] = {0};
	int page = 0, need_redraw = 1;
	const int btn_w = 130, btn_h = 30;
	const int btn_go_x = UI_PANEL_X + 100, btn_track_x = UI_PANEL_X + 260,
	          btn_back_x = UI_PANEL_X + 420;
	const int btn_y = UI_PANEL_Y + UI_PANEL_H - 50;

	while (1) {
		int pages = pending_order_count(order_id);
		if (page >= pages) page = pages - 1;
		if (need_redraw) {
			draw_dispatch_query_pdf(order_id, page);
			settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
			settextcolor(TEXT_MAIN);
			setfillcolor(WHITE_COLOR);
			setlinecolor(GRAY_LINE);

			rectangle(btn_go_x, btn_y, btn_go_x + btn_w, btn_y + btn_h);
			outtextxy(btn_go_x + 25, btn_y + 7, "进入调度");

			rectangle(btn_track_x, btn_y, btn_track_x + btn_w, btn_y + btn_h);
			outtextxy(btn_track_x + 25, btn_y + 7, "运输跟踪");

			rectangle(btn_back_x, btn_y, btn_back_x + btn_w, btn_y + btn_h);
			outtextxy(btn_back_x + 45, btn_y + 7, "返回");
			need_redraw = 0;
		}
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *buf = order_id;
		int max_len = 23;

		if (msg.message == WM_LBUTTONDOWN) {
			if (inRect(msg.x, msg.y, UI_PANEL_X + 82, UI_PANEL_Y + 84, 220, 24)) {
			}
			else if (inRect(msg.x, msg.y, UI_PANEL_X + 550, UI_PANEL_Y + 84, 70, 30)) {
				page = 0;
				need_redraw = 1;
			}
			else if (inRect(msg.x, msg.y, btn_go_x, btn_y, btn_w, btn_h)) {
				if (strlen(order_id) == 0) {
					MessageBoxA(GetHWnd(), "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
					continue;
				}
				Order *o = order_svc_find_by_id(order_id);
				if (!o) {
					MessageBoxA(GetHWnd(), "订单不存在", "提示", MB_OK | MB_ICONWARNING);
					continue;
				}
				if (o->status != ORDER_PENDING_TRANSPORT) {
					MessageBoxA(GetHWnd(), "仅待运输状态的订单可以调度",
					            "提示", MB_OK | MB_ICONWARNING);
					continue;
				}

				/* --- 创建调度单 --- */
				window_clear_frame();
				window_set_card(1);

				char vehicle_combo[2048] = {0};
				char driver_combo[2048] = {0};
				char route_combo[2048] = {0};
				build_vehicle_combo(vehicle_combo, sizeof(vehicle_combo));
				build_driver_combo(driver_combo, sizeof(driver_combo));
				build_route_combo(route_combo, sizeof(route_combo));

				if (vehicle_combo[0] == '\0') strcpy(vehicle_combo, "无可用车辆");
				if (driver_combo[0] == '\0') strcpy(driver_combo, "无可用司机");
				if (route_combo[0] == '\0') strcpy(route_combo, "无可用路线");

				char info_buf[200];
				const char *gname = strlen(o->goods_name) ? o->goods_name : o->goods_type;
				sprintf(info_buf, "订单: %s | 货物: %s | %s->%s",
				        o->order_id, gname, o->from_addr, o->to_addr);

				WINDOW_T form = {
					135, 55, 530, 490, WHITE_COLOR, 14, {
						{285, 70, 245, 26, "智能物流管理系统运输调度界面",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{165, 120, 360, 22, "",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{165, 155, 110, 22, "选择车辆:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{275, 152, 240, 25, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
						{165, 195, 110, 22, "选择司机:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{275, 192, 240, 25, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
						{165, 235, 110, 22, "选择线路:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{275, 232, 240, 25, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
						{165, 275, 110, 22, "计划出发:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{275, 272, 240, 25, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
						{165, 315, 110, 22, "计划到达:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{275, 312, 240, 25, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
						{225, 380, 130, 32, "生成调度单",
						 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
						{385, 380, 130, 32, "返回",
						 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
					}
				};
				strcpy(form.controls[1].text, info_buf);
				strcpy(form.controls[3].text, vehicle_combo);
				strcpy(form.controls[5].text, driver_combo);
				strcpy(form.controls[7].text, route_combo);

				while (1) {
					window_show(form);
					form = window_run(form);

					if (form.current == 12) {
						char *vtext = form.controls[3].text;
						char *dtext = form.controls[5].text;
						char *rtext = form.controls[7].text;
						int v_sel = form.controls[3].sel_index;
						int d_sel = form.controls[5].sel_index;
						int r_sel = form.controls[7].sel_index;

						int vehicle_id = 0, driver_id = 0, route_id = 0;
						if (vtext[0]) {
							int sel = 0;
							for (Vehicle *v = vehicle_svc_list_all(); v; v = v->next) {
								if (v->status != VEHICLE_IDLE) continue;
								if (sel == v_sel) { vehicle_id = v->id; break; }
								sel++;
							}
						}
						if (dtext[0]) {
							int sel = 0;
							for (Driver *d = driver_svc_list_all(); d; d = d->next) {
								if (d->status != DRIVER_ON_DUTY) continue;
								if (sel == d_sel) { driver_id = d->id; break; }
								sel++;
							}
						}
						if (rtext[0]) {
							int sel = 0;
							for (Route *r = route_svc_list_all(); r; r = r->next) {
								if (sel == r_sel) { route_id = r->id; break; }
								sel++;
							}
						}

						if (!vehicle_id || !driver_id || !route_id) {
							MessageBoxA(GetHWnd(), "请选择车辆、司机和线路",
							            "提示", MB_OK | MB_ICONWARNING);
							continue;
						}

						char *depart = form.controls[9].text;
						char *arrive = form.controls[11].text;

						char err[256];
						int ret = dispatch_svc_create(order_id, vehicle_id,
						                              driver_id, route_id,
						                              depart, arrive,
						                              err, sizeof(err));
						if (ret == 0) {
							char msg[512];
							Dispatch *dd = dispatch_svc_find_by_order(order_id);
							sprintf(msg,
							        "调度单已生成！\n\n"
							        "调度单号: %s\n"
							        "订单号: %s\n"
							        "请使用\"运输跟踪\"更新运输状态",
							        dd ? dd->dispatch_id : "--", order_id);
							MessageBoxA(GetHWnd(), msg, "提示",
							            MB_OK | MB_ICONINFORMATION);
							need_redraw = 1;
							goto back_to_list;
						} else {
							MessageBoxA(GetHWnd(), err, "调度失败",
							            MB_OK | MB_ICONERROR);
						}
					}
					else if (form.current == 13) {
						need_redraw = 1;
						goto back_to_list;
					}
					else {
						need_redraw = 1;
						goto back_to_list;
					}
				}
back_to_list:;
			}
			else if (inRect(msg.x, msg.y, btn_track_x, btn_y, btn_w, btn_h)) {
				/* --- 运输跟踪 --- */
				window_clear_frame();
				window_set_card(1);

				WINDOW_T track = {
					180, 100, 440, 400, WHITE_COLOR, 9, {
						{190, 110, 420, 30, "运输状态更新",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{190, 160, 80, INPUT_H, "调度单号:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{290, 160, 270, INPUT_H, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
						{190, 210, 80, INPUT_H, "更新状态:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{290, 210, 270, INPUT_H, "已出发|已到达中转站|已送达|异常",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 0, 0, 0, 0},
						{190, 260, 80, INPUT_H, "异常原因:",
						 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
						{290, 260, 270, INPUT_H, "",
						 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
						{210, 330, BTN_W, BTN_H, "更新状态",
						 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
						{410, 330, BTN_W, BTN_H, "返回",
						 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
					}
				};

				while (1) {
					window_show(track);
					track = window_run(track);

					if (track.current == 7) {
						char *did = track.controls[2].text;
						int  status_idx = track.controls[4].sel_index;

						if (strlen(did) == 0) {
							MessageBoxA(GetHWnd(), "请输入调度单号",
							            "提示", MB_OK | MB_ICONWARNING);
							continue;
						}

						Dispatch *dd = dispatch_svc_find_by_id(did);
						if (!dd) {
							MessageBoxA(GetHWnd(), "调度单不存在",
							            "提示", MB_OK | MB_ICONWARNING);
							continue;
						}

						DispatchStatus new_status;
						switch (status_idx) {
						case 0: new_status = DISPATCH_DEPARTED;  break;
						case 1: new_status = DISPATCH_TRANSIT;   break;
						case 2: new_status = DISPATCH_DELIVERED; break;
						case 3: new_status = DISPATCH_ABNORMAL;  break;
						default: new_status = DISPATCH_DEPARTED; break;
						}

						char *reason = track.controls[6].text;
						if (new_status == DISPATCH_ABNORMAL && strlen(reason) == 0) {
							MessageBoxA(GetHWnd(), "异常状态请填写异常原因",
							            "提示", MB_OK | MB_ICONWARNING);
							continue;
						}

						if (new_status == DISPATCH_DEPARTED) {
								/* 发车前先扣库存、写 OP_OUTBOUND */
								Order *order = order_svc_find_by_id(dd->order_id);
								if (order) {
									char err[256];
									int oret = outbound_svc_execute_by_dispatch(
										did, order->goods_quantity, "--", err, sizeof(err));
									if (oret != 0) {
										MessageBoxA(GetHWnd(), err,
										            "出库失败", MB_OK | MB_ICONERROR);
										continue;
									}
								}
							}

							int ret = tracking_svc_add(did, new_status, reason);
						if (ret == 0) {
							char msg[256];
							sprintf(msg, "调度单 %s 状态更新为: %s",
							        did, dispatch_status_to_string(new_status));
							MessageBoxA(GetHWnd(), msg, "提示",
							            MB_OK | MB_ICONINFORMATION);
							need_redraw = 1;
							goto back_to_list2;
						} else {
							MessageBoxA(GetHWnd(), "更新失败，调度单不存在",
							            "提示", MB_OK | MB_ICONERROR);
						}
					}
					else if (track.current == 8) {
						need_redraw = 1;
						goto back_to_list2;
					}
					else {
						need_redraw = 1;
						goto back_to_list2;
					}
				}
back_to_list2:;
			}
			else if (inRect(msg.x, msg.y, btn_back_x, btn_y, btn_w, btn_h)) {
				return;
			}
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_BACK) {
				int len = (int)strlen(buf);
				if (len > 0) { buf[len - 1] = '\0'; page = 0; need_redraw = 1; }
			}
			if (msg.vkcode == VK_LEFT && page > 0) { page--; need_redraw = 1; }
			if (msg.vkcode == VK_RIGHT && page < pages - 1) { page++; need_redraw = 1; }
		}
		else if (msg.message == WM_CHAR) {
			char ch = (char)msg.ch;
			if (ch >= 32 && ch <= 126) {
				int len = (int)strlen(buf);
				if (len < max_len) {
					buf[len] = ch;
					buf[len + 1] = '\0';
					page = 0;
					need_redraw = 1;
				}
			}
		}
	}
}

/* ---- 桩窗口 ---- */
static void vehicleMgmtStubWin() {
	MessageBoxA(GetHWnd(), "车辆管理（扩展功能）", "提示", MB_OK);
}
static void driverMgmtStubWin() {
	MessageBoxA(GetHWnd(), "司机管理（扩展功能）", "提示", MB_OK);
}

/* ========== 运输管理菜单（图11）========== */
void transportMgmtWin() {
	window_clear_frame();
	window_set_card(0);
	window_set_frame(90, 50, 620, 390);

	WINDOW_T win = {
		90, 50, 620, 390, WHITE_COLOR, 8, {
			{205, 75, 390, 30, "智能物流管理系统运输管理界面",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{125, 130, 240, 24, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{410, 130, 260, 24, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{135, 180, 190, 36, "1. 运输调度和跟踪",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{410, 180, 190, 36, "2. 车辆管理",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{135, 235, 190, 36, "3. 司机管理",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 235, 190, 36, "4. 返回上级",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{135, 325, 470, 24, "操作说明: 通过上下键切换菜单，按Enter键进入",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
		}
	};

	char meta_left[100], meta_right[100];
	sprintf(meta_left, "当前用户: %s(%s)",
	        current_user->name, role_to_string(current_user->role));
	sprintf(meta_right, "登录时间: %s",
	        login_time_str[0] ? login_time_str : "----");
	strcpy(win.controls[1].text, meta_left);
	strcpy(win.controls[2].text, meta_right);

	while (1) {
		window_set_card(0);
		window_set_frame(90, 50, 620, 390);
		window_show(win);
		win = window_run(win);
		switch (win.current) {
		case 3: dispatchWin(); break;
		case 4: vehicleMgmtStubWin(); break;
		case 5: driverMgmtStubWin(); break;
		case 6: return;
		default: return;
		}
	}
}
