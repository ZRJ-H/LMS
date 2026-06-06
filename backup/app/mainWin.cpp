#include <graphics.h>
#include <conio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../public/common.h"
#include "../public/ui_config.h"
#include "loginWin.h"
#include "orderWin.h"
#include "../view/control.h"
#include "../service/user_service.h"
#include "../service/order_service.h"
#include "../service/stats_service.h"
#include "../service/backup_service.h"
#include "transportWin.h"
#include "warehouseWin.h"

/* ============================================================
 *  页面标题栏 — 在 window_show() 之后绘制标题 + 用户信息
 * ============================================================ */
static void drawPageHeader(const char *title) {
	char info_left_new[128];
	char info_right_new[128];
	sprintf(info_left_new, "当前用户: %s(%s)",
	        current_user->name, role_to_string(current_user->role));
	sprintf(info_right_new, "登录时间: %s",
	        login_time_str[0] ? login_time_str : "----");
	ui_draw_title(title);
	ui_draw_meta(info_left_new, info_right_new);
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

/* ============================================================
 *  菜单项定义
 * ============================================================ */
typedef struct {
	const char *text;
	int         perm;       /* 所需权限掩码，0 = 所有人 */
	void      (*action)();  /* 回调函数 */
	int         is_exit;    /* 1 = 退出系统 */
} MenuItem;

/* ------- 占位窗口（尚未实现的模块）------- */
static void serviceStubWin() {
	MessageBoxA(GetHWnd(), "客户服务（扩展功能）", "提示", MB_OK);
}
static void roleMgmtStubWin() {
	MessageBoxA(GetHWnd(), "角色管理（扩展功能）", "提示", MB_OK);
}
static void routeConfigStubWin() {
	MessageBoxA(GetHWnd(), "运输路线配置（扩展功能）", "提示", MB_OK);
}
static void dataBackupWin() {
	WINDOW_T win = {
		0, 0, WIN_W, WIN_H, WHITE_COLOR, 4, {
			{MENU_COL_LEFT,  MENU_ROWS_Y[0], MENU_W, MENU_H, "1. 手动备份",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{MENU_COL_RIGHT, MENU_ROWS_Y[0], MENU_W, MENU_H, "2. 数据恢复",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{MENU_COL_LEFT,  MENU_ROWS_Y[1], MENU_W, MENU_H, "3. 查看说明",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{MENU_COL_RIGHT, MENU_ROWS_Y[1], MENU_W, MENU_H, "4. 返回上级",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	window_clear_frame();
	window_set_card(1);
	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统数据备份界面");
		win = window_run(win);

		if (win.current == 0) {
			char dir[MAX_PATH];
			char msg[256];
			int copied = backup_svc_create_manual(dir, sizeof(dir));
			sprintf(msg, "备份完成：%d 个数据文件\n目录：%s", copied, dir);
			MessageBoxA(GetHWnd(), msg, "数据备份", MB_OK | MB_ICONINFORMATION);
		} else if (win.current == 1) {
			int ok = MessageBoxA(GetHWnd(),
				"将从最近一次备份恢复数据文件，当前数据会被覆盖。\n恢复后请重启系统使数据重新加载。\n是否继续？",
				"数据恢复确认", MB_YESNO | MB_ICONWARNING);
			if (ok == IDYES) {
				int restored = backup_svc_restore_latest();
				char msg[256];
				if (restored < 0) {
					MessageBoxA(GetHWnd(), "没有找到可恢复的备份目录", "数据恢复", MB_OK | MB_ICONWARNING);
				} else {
					sprintf(msg, "恢复完成：%d 个数据文件\n请退出并重新启动系统。", restored);
					MessageBoxA(GetHWnd(), msg, "数据恢复", MB_OK | MB_ICONINFORMATION);
				}
			}
		} else if (win.current == 2) {
			MessageBoxA(GetHWnd(),
				"手动备份会复制 users/orders/warehouse/transport 等 TXT 数据文件。\n数据恢复默认使用最近一次备份。",
				"说明", MB_OK | MB_ICONINFORMATION);
		} else if (win.current == 3) {
			return;
		}
	}
}

typedef enum {
	STAT_RANGE_TODAY = 0,
	STAT_RANGE_WEEK,
	STAT_RANGE_MONTH
} StatRange;

static void make_stat_range(StatRange range, char *start_ymd, char *end_ymd) {
	time_t now = time(NULL);
	struct tm t = *localtime(&now);
	sprintf(end_ymd, "%04d%02d%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

	if (range == STAT_RANGE_TODAY) {
		strcpy(start_ymd, end_ymd);
		return;
	}
	if (range == STAT_RANGE_WEEK) {
		int back_days = (t.tm_wday == 0) ? 6 : t.tm_wday - 1;
		now -= (time_t)back_days * 24 * 60 * 60;
		t = *localtime(&now);
	} else {
		t.tm_mday = 1;
	}
	sprintf(start_ymd, "%04d%02d%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
}

static void drawStatButton(int x, int y, int w, int h, const char *text, int active) {
	setlinecolor(INPUT_BORDER);
	setfillcolor(active ? PRIMARY : WHITE_COLOR);
	fillrectangle(x, y, x + w, y + h);
	rectangle(x, y, x + w, y + h);
	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(active ? WHITE_COLOR : TEXT_MAIN);
	outtextxy(x + (w - textwidth(text)) / 2, y + (h - textheight(text)) / 2, (char *)text);
}

static void draw_order_stats_pdf(const OrderStatResult *stat, StatRange range) {
	char line[160];
	int y = UI_PANEL_Y + 175;
	int total = stat->total_orders ? stat->total_orders : 1;

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统订单统计界面", 370);

	drawStatButton(UI_PANEL_X + 25, UI_PANEL_Y + 135, 70, 30, "今日", range == STAT_RANGE_TODAY);
	drawStatButton(UI_PANEL_X + 110, UI_PANEL_Y + 135, 70, 30, "本周", range == STAT_RANGE_WEEK);
	drawStatButton(UI_PANEL_X + 195, UI_PANEL_Y + 135, 70, 30, "本月", range == STAT_RANGE_MONTH);

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 25, y, "================订单统计结果================"); y += 24;
	sprintf(line, "统计时间：%s 至 %s", stat->start_date, stat->end_date);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "总订单数：%d", stat->total_orders);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "已完成订单数：%d", stat->completed_orders);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "订单完成率：%.1f%%", stat->completion_rate);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "驳回订单数：%d", stat->rejected_orders);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "驳回率：%.1f%%", stat->rejection_rate);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "运输中订单数：%d", stat->in_transit_orders);
	outtextxy(UI_PANEL_X + 55, y, line); y += 24;
	sprintf(line, "普通货物:%d单(%.1f%%)", stat->normal_goods_count,
	        stat->normal_goods_count * 100.0f / total);
	outtextxy(UI_PANEL_X + 55, y, line); y += 22;
	sprintf(line, "易碎货物:%d单(%.1f%%)", stat->fragile_count,
	        stat->fragile_count * 100.0f / total);
	outtextxy(UI_PANEL_X + 55, y, line); y += 22;
	sprintf(line, "冷链货物:%d单(%.1f%%)", stat->cold_chain_count,
	        stat->cold_chain_count * 100.0f / total);
	outtextxy(UI_PANEL_X + 55, y, line); y += 22;
	sprintf(line, "危险品:%d单(%.1f%%)", stat->dangerous_count,
	        stat->dangerous_count * 100.0f / total);
	outtextxy(UI_PANEL_X + 55, y, line);

	drawStatButton(UI_PANEL_X + 40, UI_PANEL_Y + 315, 70, 30, "导出", 0);
	drawStatButton(UI_PANEL_X + 165, UI_PANEL_Y + 315, 70, 30, "返回", 0);
}

static void orderStatsWin() {
	StatRange range = STAT_RANGE_WEEK;
	char start_ymd[9], end_ymd[9];
	OrderStatResult stat;
	int need_redraw = 1;

	while (1) {
		if (need_redraw) {
			make_stat_range(range, start_ymd, end_ymd);
			stats_svc_calc_order(start_ymd, end_ymd, &stat);
			draw_order_stats_pdf(&stat, range);
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_MOUSE);
		if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_LEFT && range > STAT_RANGE_TODAY) {
				range = (StatRange)(range - 1);
				need_redraw = 1;
			}
			if (msg.vkcode == VK_RIGHT && range < STAT_RANGE_MONTH) {
				range = (StatRange)(range + 1);
				need_redraw = 1;
			}
		} else if (msg.message == WM_LBUTTONDOWN) {
			if (msg.y >= UI_PANEL_Y + 135 && msg.y <= UI_PANEL_Y + 165) {
				if (msg.x >= UI_PANEL_X + 25 && msg.x <= UI_PANEL_X + 95) {
					range = STAT_RANGE_TODAY; need_redraw = 1;
				} else if (msg.x >= UI_PANEL_X + 110 && msg.x <= UI_PANEL_X + 180) {
					range = STAT_RANGE_WEEK; need_redraw = 1;
				} else if (msg.x >= UI_PANEL_X + 195 && msg.x <= UI_PANEL_X + 265) {
					range = STAT_RANGE_MONTH; need_redraw = 1;
				}
			} else if (msg.y >= UI_PANEL_Y + 315 && msg.y <= UI_PANEL_Y + 345) {
				if (msg.x >= UI_PANEL_X + 40 && msg.x <= UI_PANEL_X + 110) {
					if (stats_svc_export_order_txt("data/order_stats_report.txt", &stat) == 0)
						MessageBoxA(GetHWnd(), "报表已导出到 data/order_stats_report.txt", "提示", MB_OK | MB_ICONINFORMATION);
					else
						MessageBoxA(GetHWnd(), "报表导出失败，请检查 data 目录", "错误", MB_OK | MB_ICONERROR);
				} else if (msg.x >= UI_PANEL_X + 165 && msg.x <= UI_PANEL_X + 235) {
					return;
				}
			}
		}
	}
}

static void draw_warehouse_stats_pdf(const WarehouseStatResult *stat, StatRange range) {
	char line[160];
	int y = UI_PANEL_Y + 175;

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统仓储统计界面", 370);

	drawStatButton(UI_PANEL_X + 25, UI_PANEL_Y + 135, 70, 30, "今日", range == STAT_RANGE_TODAY);
	drawStatButton(UI_PANEL_X + 110, UI_PANEL_Y + 135, 70, 30, "本周", range == STAT_RANGE_WEEK);
	drawStatButton(UI_PANEL_X + 195, UI_PANEL_Y + 135, 70, 30, "本月", range == STAT_RANGE_MONTH);

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 25, y, "================仓储统计结果================"); y += 30;
	sprintf(line, "统计时间：%s 至 %s", stat->start_date, stat->end_date);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "入库总数量：%d 件", stat->total_inbound);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "出库总数量：%d 件", stat->total_outbound);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "当前库存总量：%d 件", stat->current_total_inv);
	outtextxy(UI_PANEL_X + 55, y, line);

	drawStatButton(UI_PANEL_X + 40, UI_PANEL_Y + 315, 70, 30, "导出", 0);
	drawStatButton(UI_PANEL_X + 165, UI_PANEL_Y + 315, 70, 30, "返回", 0);
}

static void warehouseStatsWin() {
	StatRange range = STAT_RANGE_WEEK;
	char start_ymd[9], end_ymd[9];
	WarehouseStatResult stat;
	int need_redraw = 1;

	while (1) {
		if (need_redraw) {
			make_stat_range(range, start_ymd, end_ymd);
			stats_svc_calc_warehouse(start_ymd, end_ymd, &stat);
			draw_warehouse_stats_pdf(&stat, range);
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_MOUSE);
		if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_LEFT && range > STAT_RANGE_TODAY) {
				range = (StatRange)(range - 1);
				need_redraw = 1;
			}
			if (msg.vkcode == VK_RIGHT && range < STAT_RANGE_MONTH) {
				range = (StatRange)(range + 1);
				need_redraw = 1;
			}
		} else if (msg.message == WM_LBUTTONDOWN) {
			if (msg.y >= UI_PANEL_Y + 135 && msg.y <= UI_PANEL_Y + 165) {
				if (msg.x >= UI_PANEL_X + 25 && msg.x <= UI_PANEL_X + 95) {
					range = STAT_RANGE_TODAY; need_redraw = 1;
				} else if (msg.x >= UI_PANEL_X + 110 && msg.x <= UI_PANEL_X + 180) {
					range = STAT_RANGE_WEEK; need_redraw = 1;
				} else if (msg.x >= UI_PANEL_X + 195 && msg.x <= UI_PANEL_X + 265) {
					range = STAT_RANGE_MONTH; need_redraw = 1;
				}
			} else if (msg.y >= UI_PANEL_Y + 315 && msg.y <= UI_PANEL_Y + 345) {
				if (msg.x >= UI_PANEL_X + 40 && msg.x <= UI_PANEL_X + 110) {
					if (stats_svc_export_warehouse_txt("data/warehouse_stats_report.txt", &stat) == 0)
						MessageBoxA(GetHWnd(), "报表已导出到 data/warehouse_stats_report.txt", "提示", MB_OK | MB_ICONINFORMATION);
					else
						MessageBoxA(GetHWnd(), "报表导出失败，请检查 data 目录", "错误", MB_OK | MB_ICONERROR);
				} else if (msg.x >= UI_PANEL_X + 165 && msg.x <= UI_PANEL_X + 235) {
					return;
				}
			}
		}
	}
}

static void draw_transport_stats_pdf(const TransportStatResult *stat, StatRange range) {
	char line[160];
	int y = UI_PANEL_Y + 175;

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统运输统计界面", 370);

	drawStatButton(UI_PANEL_X + 25, UI_PANEL_Y + 135, 70, 30, "今日", range == STAT_RANGE_TODAY);
	drawStatButton(UI_PANEL_X + 110, UI_PANEL_Y + 135, 70, 30, "本周", range == STAT_RANGE_WEEK);
	drawStatButton(UI_PANEL_X + 195, UI_PANEL_Y + 135, 70, 30, "本月", range == STAT_RANGE_MONTH);

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 25, y, "================运输统计结果================"); y += 30;
	sprintf(line, "统计时间：%s 至 %s", stat->start_date, stat->end_date);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "调度总单数：%d 单", stat->total_dispatch);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "运输完成单数：%d 单", stat->completed_dispatch);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "运输完成率：%.1f%%", stat->completion_rate);
	outtextxy(UI_PANEL_X + 55, y, line); y += 30;
	sprintf(line, "平均运输时效：%.1f 小时", stat->avg_transport_hours);
	outtextxy(UI_PANEL_X + 55, y, line);

	drawStatButton(UI_PANEL_X + 40, UI_PANEL_Y + 315, 70, 30, "导出", 0);
	drawStatButton(UI_PANEL_X + 165, UI_PANEL_Y + 315, 70, 30, "返回", 0);
}

static void transportStatsWin() {
	StatRange range = STAT_RANGE_WEEK;
	char start_ymd[9], end_ymd[9];
	TransportStatResult stat;
	int need_redraw = 1;

	while (1) {
		if (need_redraw) {
			make_stat_range(range, start_ymd, end_ymd);
			stats_svc_calc_transport(start_ymd, end_ymd, &stat);
			draw_transport_stats_pdf(&stat, range);
			need_redraw = 0;
		}

		ExMessage msg = getmessage(EX_KEY | EX_MOUSE);
		if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_LEFT && range > STAT_RANGE_TODAY) {
				range = (StatRange)(range - 1);
				need_redraw = 1;
			}
			if (msg.vkcode == VK_RIGHT && range < STAT_RANGE_MONTH) {
				range = (StatRange)(range + 1);
				need_redraw = 1;
			}
		} else if (msg.message == WM_LBUTTONDOWN) {
			if (msg.y >= UI_PANEL_Y + 135 && msg.y <= UI_PANEL_Y + 165) {
				if (msg.x >= UI_PANEL_X + 25 && msg.x <= UI_PANEL_X + 95) {
					range = STAT_RANGE_TODAY; need_redraw = 1;
				} else if (msg.x >= UI_PANEL_X + 110 && msg.x <= UI_PANEL_X + 180) {
					range = STAT_RANGE_WEEK; need_redraw = 1;
				} else if (msg.x >= UI_PANEL_X + 195 && msg.x <= UI_PANEL_X + 265) {
					range = STAT_RANGE_MONTH; need_redraw = 1;
				}
			} else if (msg.y >= UI_PANEL_Y + 315 && msg.y <= UI_PANEL_Y + 345) {
				if (msg.x >= UI_PANEL_X + 40 && msg.x <= UI_PANEL_X + 110) {
					if (stats_svc_export_transport_txt("data/transport_stats_report.txt", &stat) == 0)
						MessageBoxA(GetHWnd(), "报表已导出到 data/transport_stats_report.txt", "提示", MB_OK | MB_ICONINFORMATION);
					else
						MessageBoxA(GetHWnd(), "报表导出失败，请检查 data 目录", "错误", MB_OK | MB_ICONERROR);
				} else if (msg.x >= UI_PANEL_X + 165 && msg.x <= UI_PANEL_X + 235) {
					return;
				}
			}
		}
	}
}

static void statsWin() {
	MenuItem sm[5] = {
		{"1. 订单统计", 0, (void(*)())orderStatsWin, 0},
		{"2. 仓储统计", 0, (void(*)())warehouseStatsWin, 0},
		{"3. 运输统计", 0, (void(*)())transportStatsWin, 0},
		{"4. 报表生成", 0, (void(*)())orderStatsWin, 0},
		{"5. 返回上级", 0, NULL, 0},
	};
	WINDOW_T win;
	win.x = 0; win.y = 0; win.width = WIN_W; win.height = WIN_H;
	win.bgColor = WHITE_COLOR;
	win.count = 5;
	for (int i = 0; i < 5; i++) {
		CONTROL_T c;
		c.x = (i % 2 == 0) ? MENU_COL_LEFT : MENU_COL_RIGHT;
		c.y = MENU_ROWS_Y[i / 2];
		c.width = MENU_W; c.height = MENU_H;
		strcpy(c.text, sm[i].text);
		c.bgColor1 = PRIMARY; c.bgColor2 = WHITE_COLOR;
		c.textColor = WHITE_COLOR; c.textColor2 = TEXT_MAIN;
		c.type = BUTTON;
		c.state = (i == 0) ? 1 : 0;
		c.visible = 0; c.sel_index = 0;
		win.controls[i] = c;
	}
	win.current = 0;

	window_clear_frame();
	window_set_card(1);
	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统统计分析界面");
		win = window_run(win);
		if (win.current == 4) return;
		if (sm[win.current].action) sm[win.current].action();
	}
}

/* ------- 角色选择 COMBO 窗口 ------- */
static void selectRoleWin(UserRole *out_role) {
	WINDOW_T win = {
		230, 180, 340, 220, WHITE_COLOR, 3, {
			{240, 190, 320, 30, "请选择用户角色",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{240, 250, 320, 40, "管理员|客服|仓储员|调度员|客户",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, COMBO, 1, 0, 4, 0},
			{280, 330, 240, BTN_H, "确认创建",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};
	UserRole roles[] = {ROLE_ADMIN, ROLE_SERVICE, ROLE_WAREHOUSE, ROLE_DISPATCHER, ROLE_CUSTOMER};

	while (1) {
		window_show(win);
		win = window_run(win);
		if (win.current == 2) {
			*out_role = roles[win.controls[1].sel_index];
			return;
		}
	}
}

/* ------- 用户表格行绘制回调 ------- */
static void drawUserRow(const void *record, int row_idx, int y_base,
                        int table_x, const int *col_widths, int ncols) {
	const User *u = (const User *)record;
	char buf[64];
	int x = table_x;
	settextcolor(TEXT_MAIN);
	settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));

	sprintf(buf, "%d", u->id);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[0];

	outtextxy(x + CTRL_PADDING, y_base + 8, u->name);
	x += col_widths[1];

	outtextxy(x + CTRL_PADDING, y_base + 8, (char *)role_to_string(u->role));
	x += col_widths[2];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          u->lockout_until > time(NULL) ? "锁定" : "正常");
}

/* ------- 用户列表分页表格 ------- */
static void showUserList(const User *head) {
	const char *headers[] = {"ID", "用户名", "角色", "状态"};
	const int col_widths[] = {60, 150, 120, 80};
	window_show_table("用户列表", headers, col_widths, 4,
	                  head, offsetof(User, next), drawUserRow, PAGE_SIZE);
}

/* ------- 用户条件查询 ------- */
static int user_matches_query_pdf(const User *u, const char *keyword) {
	return keyword[0] == '\0' || strstr(u->name, keyword) != NULL;
}

static int user_query_total_pages(const char *keyword) {
	int total = 0;
	for (User *p = user_svc_list_all(); p; p = p->next) {
		if (user_matches_query_pdf(p, keyword)) total++;
	}
	return total ? (total + 3) / 4 : 1;
}

static void draw_user_query_pdf(const char *keyword, int page) {
	const int col_w[4] = {80, 100, 100, 80};
	const char *headers[4] = {"用户id", "用户名", "角色", "状态"};
	int header_h = 22, row_h = 22;
	int total_w = col_w[0] + col_w[1] + col_w[2] + col_w[3];
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = UI_PANEL_Y + 122;
	int start = page * 4;
	int shown = 0, idx = 0, x;
	char id_buf[32];

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统用户查询界面", 285);

	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 60, UI_PANEL_Y + 91, "用户名搜索:");
	drawTextBox(UI_PANEL_X + 145, UI_PANEL_Y + 84, 150, 24, keyword, 1);
	drawQueryButton(UI_PANEL_X + 312, UI_PANEL_Y + 84, 65, 26);

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

	for (User *p = user_svc_list_all(); p && shown < 4; p = p->next) {
		if (!user_matches_query_pdf(p, keyword)) continue;
		if (idx++ < start) continue;
		int y = table_y + header_h + shown * row_h;
		setfillcolor(WHITE_COLOR);
		fillrectangle(table_x, y, table_x + total_w, y + row_h);
		setlinecolor(GRAY_LINE);
		rectangle(table_x, y, table_x + total_w, y + row_h);
		x = table_x;
		settextcolor(TEXT_MAIN);
		sprintf(id_buf, "%d", p->id);
		outtextxy(x + 5, y + 5, id_buf); x += col_w[0];
		outtextxy(x + 5, y + 5, p->name); x += col_w[1];
		outtextxy(x + 5, y + 5, (char *)role_to_string(p->role)); x += col_w[2];
		outtextxy(x + 5, y + 5, p->lockout_until > time(NULL) ? "锁定" : "可用");
		int vx = table_x;
		for (int c = 0; c < 4; c++) {
			vx += col_w[c];
			line(vx, y, vx, y + row_h);
		}
		shown++;
	}
	if (shown == 0) {
		settextcolor(TEXT_MUTED);
		outtextxy(table_x + 130, table_y + header_h + 42, "暂无匹配用户");
	}

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	drawPageText(UI_PANEL_Y + 240, user_query_total_pages(keyword), page);
}

static void searchUserWinPdf() {
	char keyword[64] = {0};
	int page = 0;
	int need_redraw = 1;

	while (1) {
		int pages = user_query_total_pages(keyword);
		if (page >= pages) page = pages - 1;
		if (need_redraw) {
			draw_user_query_pdf(keyword, page);
			need_redraw = 0;
		}
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);

		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x >= UI_PANEL_X + UI_PANEL_W - 70 &&
			    msg.x <= UI_PANEL_X + UI_PANEL_W &&
			    msg.y >= UI_PANEL_Y + 250) return;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_BACK) {
				if (input_delete_last_char(keyword)) { page = 0; need_redraw = 1; }
			}
			if (msg.vkcode == VK_LEFT && page > 0) { page--; need_redraw = 1; }
			if (msg.vkcode == VK_RIGHT && page < pages - 1) { page++; need_redraw = 1; }
		}
		else if (msg.message == WM_CHAR) {
			if (input_append_char(keyword, 30, msg.ch, INPUT_FILTER_CHINESE)) {
				page = 0;
				need_redraw = 1;
			}
		}
	}
}

static void searchUserWin() {
	searchUserWinPdf();
}

/* ========== 系统管理子窗口 — 2×4 网格 ========== */
static void sysAdminWin() {
	MenuItem sm[8] = {
		{"1. 创建用户",    0, NULL,                      0},
		{"2. 用户查询",    0, NULL,                      0},
		{"3. 密码重置",    0, NULL,                      0},
		{"4. 角色管理",    0, (void(*)())roleMgmtStubWin,     0},
		{"5. 仓库配置",    0, (void(*)())warehouseConfigWin, 0},
		{"6. 运输路线配置",0, (void(*)())routeConfigStubWin,   0},
		{"7. 数据备份",    0, (void(*)())dataBackupWin,        0},
		{"8. 返回上级",    0, NULL,                           0},
	};

	/* 复用主菜单网格 Y 坐标，左列 X 偏移适配子窗口 */
	int sub_col_x[2] = {MENU_COL_LEFT, MENU_COL_RIGHT};

	WINDOW_T win;
	win.x = 60;  win.y = 20;  win.width = 680;  win.height = 560;
	win.bgColor = WHITE_COLOR;
	win.count = 8;

	for (int i = 0; i < 8; i++) {
		CONTROL_T c;
		c.x = sub_col_x[i % 2];
		c.y = MENU_ROWS_Y[i / 2];
		c.width = MENU_W;  c.height = MENU_H;
		strcpy(c.text, sm[i].text);
		c.type = BUTTON;
		c.bgColor1 = PRIMARY;  c.bgColor2 = WHITE_COLOR;
		c.textColor = WHITE_COLOR;  c.textColor2 = TEXT_MAIN;
		c.state = (i == 0) ? 1 : 0;
		c.visible = 0;  c.sel_index = 0;
		win.controls[i] = c;
	}
	win.current = 0;

	window_clear_frame();
	window_set_card(1);
	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统系统管理界面");
		win = window_run(win);

		switch (win.current) {
		case 0: {
			char name[64] = {0}, pwd[64] = {0}, err[128];
			InputBox(name, 32, "请输入新用户名（回车确认）", "创建用户", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			InputBox(pwd, 32, "请输入密码（6-10位字母数字）", "创建用户", NULL, 0, 0, 1);
			if (strlen(pwd) == 0) continue;
			UserRole role;
			selectRoleWin(&role);
			int ret = user_svc_create(name, pwd, role, err, sizeof(err));
			if (ret == 0)
				MessageBoxA(GetHWnd(), "用户创建成功！", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(GetHWnd(), err, "错误", MB_OK | MB_ICONERROR);
			break;
		}
		case 1: searchUserWin(); break;
		case 2: {
			char name[64] = {0};
			InputBox(name, 32, "请输入要重置密码的用户名", "重置密码", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			if (user_svc_reset_password(name) == 0)
				MessageBoxA(GetHWnd(), "密码已重置为 888888", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(GetHWnd(), "用户不存在", "提示", MB_OK | MB_ICONWARNING);
			break;
		}
		case 7: return;
		default: if (sm[win.current].action) sm[win.current].action(); break;
		}
	}
}


static void orderMgmtWinPdf() {
	MenuItem om[6] = {
		{"1. 创建订单", 0, (void(*)())createOrderWin, 0},
		{"2. 订单审核", 0, (void(*)())auditOrderWin, 0},
		{"3. 订单查询", 0, (void(*)())searchOrderWin, 0},
		{"4. 订单修改", 0, (void(*)())modifyOrderWin, 0},
		{"5. 订单跟踪", 0, (void(*)())trackOrderWin, 0},
		{"6. 返回上级", 0, NULL, 0},
	};
	WINDOW_T win;
	win.x = 0; win.y = 0; win.width = WIN_W; win.height = WIN_H;
	win.bgColor = WHITE_COLOR;
	win.count = 6;
	for (int i = 0; i < 6; i++) {
		CONTROL_T c;
		c.x = (i % 2 == 0) ? MENU_COL_LEFT : MENU_COL_RIGHT;
		c.y = MENU_ROWS_Y[i / 2];
		c.width = MENU_W; c.height = MENU_H;
		strcpy(c.text, om[i].text);
		c.bgColor1 = PRIMARY; c.bgColor2 = WHITE_COLOR;
		c.textColor = WHITE_COLOR; c.textColor2 = TEXT_MAIN;
		c.type = BUTTON;
		c.state = (i == 0) ? 1 : 0;
		c.visible = 0; c.sel_index = 0;
		win.controls[i] = c;
	}
	win.current = 0;

	window_clear_frame();
	window_set_card(1);
	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统订单管理界面");
		win = window_run(win);
		if (win.current == 5) return;
		if (om[win.current].action) om[win.current].action();
	}
}

static void orderMgmtWin() {
	orderMgmtWinPdf();
}

/* ========== 密码修改窗口 ========== */
static void changePasswordWin() {
	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		220, 160, 360, 310, WHITE_COLOR, 7, {
			{230, 170, 340, 30, "修改密码",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{230, 215, 80, INPUT_H, "旧密码：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{310, 215, 250, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT_PWD, 1, 0, 0, 0},
			{230, 265, 80, INPUT_H, "新密码：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{310, 265, 250, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT_PWD, 0, 0, 0, 0},
			{230, 340, BTN_W, BTN_H, "确认修改",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{420, 340, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 5) {
			char *old_pwd = win.controls[2].text;
			char *new_pwd = win.controls[4].text;

			if (strlen(old_pwd) == 0 || strlen(new_pwd) == 0) {
				MessageBoxA(GetHWnd(), "密码不能为空", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			char hash[33];
			md5_hash(old_pwd, hash);
			if (strcmp(hash, current_user->password) != 0) {
				MessageBoxA(GetHWnd(), "旧密码错误", "提示", MB_OK | MB_ICONWARNING);
				win.controls[2].text[0] = '\0';
				continue;
			}

			if (validate_password(new_pwd) != 0) {
				MessageBoxA(GetHWnd(), "新密码须为4-16位字母或数字", "提示", MB_OK | MB_ICONWARNING);
				win.controls[4].text[0] = '\0';
				continue;
			}

			md5_hash(new_pwd, hash);
			strcpy(current_user->password, hash);
			user_svc_save();
			MessageBoxA(GetHWnd(), "密码修改成功！", "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}
		else if (win.current == 6) {
			return;
		}
	}
}

/* ========== 忘记密码窗口 ========== */
int forgotPasswordWin() {
	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		220, 200, 360, 260, WHITE_COLOR, 5, {
			{230, 210, 340, 30, "找回密码",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{230, 260, 80, BTN_H, "用户名：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{320, 260, 240, BTN_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{240, 340, BTN_W, BTN_H, "重置密码",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{420, 340, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 3) {
			char *name = win.controls[2].text;
			if (strlen(name) == 0) {
				MessageBoxA(GetHWnd(), "请输入用户名", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (user_svc_reset_password(name) == 0)
				MessageBoxA(GetHWnd(), "密码已重置为 888888，请返回登录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(GetHWnd(), "用户名不存在", "提示", MB_OK | MB_ICONWARNING);
		}
		else if (win.current == 4) {
			return 0;
		}
	}
}

/* ============================================================
 *  主菜单 — 标准 2×4 网格，8 按钮常显，权限过滤在 Enter 时校验
 * ============================================================ */
int mainWin() {
	if (!current_user) return 0;

	int p = role_get_permissions(current_user->role);

	/* 8 个菜单项（2×4 网格固定布局） */
	MenuItem menu[8] = {
		{"1. 系统管理", PERM_USER_MANAGE,  (void(*)())sysAdminWin,      0},
		{"2. 订单管理", PERM_ORDER_MANAGE, (void(*)())orderMgmtWin,     0},
		{"3. 仓储管理", PERM_WAREHOUSE,    (void(*)())warehouseMgmtWin, 0},
		{"4. 运输管理", PERM_TRANSPORT,    (void(*)())transportMgmtWin, 0},
		{"5. 统计分析", PERM_STATISTICS,   (void(*)())statsWin,         0},
		{"6. 客户服务", PERM_ORDER_MANAGE, (void(*)())serviceStubWin,   0},
		{"7. 密码修改", 0,                 (void(*)())changePasswordWin,0},
		{"8. 退出系统", 0,                 NULL,                        1},
	};

	WINDOW_T win;
	win.x = 0;  win.y = 0;  win.width = WIN_W;  win.height = WIN_H;
	win.bgColor = WHITE_COLOR;
	win.count = 8;

	for (int i = 0; i < 8; i++) {
		CONTROL_T c;
		c.x = (i % 2 == 0) ? MENU_COL_LEFT : MENU_COL_RIGHT;
		c.y = MENU_ROWS_Y[i / 2];
		c.width = MENU_W;  c.height = MENU_H;
		strcpy(c.text, menu[i].text);
		c.type = BUTTON;
		c.bgColor1 = PRIMARY;  c.bgColor2 = WHITE_COLOR;
		c.textColor = WHITE_COLOR;  c.textColor2 = TEXT_MAIN;
		c.state = (i == 0) ? 1 : 0;
		c.visible = 0;  c.sel_index = 0;
		win.controls[i] = c;
	}
	win.current = 0;

	window_clear_frame();
	/* 主菜单不使用白卡片（背景图透出） */
	window_set_card(0);

	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统主菜单界面");
		win = window_run(win);
		int idx = win.current;

		/* 权限校验 — 角色特定提示 */
		if (menu[idx].perm != 0 && !(p & menu[idx].perm)) {
			char perm_err[128];
			sprintf(perm_err, "对不起，%s角色无权访问该模块！",
			        role_to_string(current_user->role));
			MessageBoxA(GetHWnd(), perm_err, "权限拦截", MB_OK | MB_ICONERROR);
			continue;
		}

		if (menu[idx].is_exit) return -1;
		if (menu[idx].action) menu[idx].action();
	}
}
