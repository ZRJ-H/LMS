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
#include "../service/warehouse_service.h"

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
	return;
	/* 标题 */
	settextstyle(FONT_TITLE_H - 4, 0, _T("黑体"));
	settextcolor(TEXT_MAIN);
	int tx = (WIN_W - textwidth(title)) / 2;
	outtextxy(tx, 40, title);

	/* 用户信息栏 — 左侧 */
	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	char info_left[128];
	sprintf(info_left, "当前用户：%s(%s)",
	        current_user->name, role_to_string(current_user->role));
	outtextxy(80, 105, info_left);

	/* 用户信息栏 — 右侧（登录时间） */
	char info_right[128];
	sprintf(info_right, "登录时间：%s",
	        login_time_str[0] ? login_time_str : "----");
	outtextxy(480, 105, info_right);
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
static void transportStubWin() {
	MessageBoxA(GetHWnd(), "运输管理（扩展功能）", "提示", MB_OK);
}
static void statsStubWin() {
	MessageBoxA(GetHWnd(), "统计分析（扩展功能）", "提示", MB_OK);
}
static void serviceStubWin() {
	MessageBoxA(GetHWnd(), "客户服务（扩展功能）", "提示", MB_OK);
}
static void roleMgmtStubWin() {
	MessageBoxA(GetHWnd(), "角色管理（扩展功能）", "提示", MB_OK);
}
static void warehouseConfigStubWin() {
	MessageBoxA(GetHWnd(), "仓库配置（扩展功能）", "提示", MB_OK);
}
static void routeConfigStubWin() {
	MessageBoxA(GetHWnd(), "运输路线配置（扩展功能）", "提示", MB_OK);
}
static void dataBackupStubWin() {
	MessageBoxA(GetHWnd(), "数据备份（扩展功能）", "提示", MB_OK);
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

	while (1) {
		int pages = user_query_total_pages(keyword);
		if (page >= pages) page = pages - 1;
		draw_user_query_pdf(keyword, page);
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);

		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x >= UI_PANEL_X + UI_PANEL_W - 70 &&
			    msg.x <= UI_PANEL_X + UI_PANEL_W &&
			    msg.y >= UI_PANEL_Y + 250) return;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_BACK) {
				int len = (int)strlen(keyword);
				if (len > 0) { keyword[len - 1] = '\0'; page = 0; }
			}
			if (msg.vkcode == VK_LEFT && page > 0) page--;
			if (msg.vkcode == VK_RIGHT && page < pages - 1) page++;
		}
		else if (msg.message == WM_CHAR) {
			char ch = (char)msg.ch;
			if (ch >= 32 && ch <= 126) {
				int len = (int)strlen(keyword);
				if (len < 31) {
					keyword[len] = ch;
					keyword[len + 1] = '\0';
					page = 0;
				}
			}
		}
	}
}

static void searchUserWin() {
	searchUserWinPdf();
	return;
	WINDOW_T win = {
		220, 160, 360, 260, WHITE_COLOR, 5, {
			{230, 170, 340, 30, "用户查询",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{230, 210, 80, BTN_H, "用户名：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{320, 210, 240, BTN_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{230, 300, BTN_W, BTN_H, "查询",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{415, 300, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 3) {
			char *keyword = win.controls[2].text;
			if (strlen(keyword) == 0) {
				showUserList(user_svc_list_all());
			} else {
				User *filtered = NULL, *tail = NULL;
				User *p = user_svc_list_all();
				while (p) {
					if (strstr(p->name, keyword)) {
						User *copy = (User *)malloc(sizeof(User));
						memcpy(copy, p, sizeof(User));
						copy->next = NULL;
						if (!filtered) filtered = copy;
						else tail->next = copy;
						tail = copy;
					}
					p = p->next;
				}
				showUserList(filtered);
				while (filtered) {
					User *tmp = filtered;
					filtered = filtered->next;
					free(tmp);
				}
			}
		}
		else if (win.current == 4) {
			return;
		}
	}
}

/* ========== 系统管理子窗口 — 2×4 网格 ========== */
static void sysAdminWin() {
	MenuItem sm[8] = {
		{"1. 创建用户",    0, NULL,                      0},
		{"2. 用户查询",    0, NULL,                      0},
		{"3. 密码重置",    0, NULL,                      0},
		{"4. 角色管理",    0, (void(*)())roleMgmtStubWin,     0},
		{"5. 仓库配置",    0, (void(*)())warehouseConfigStubWin, 0},
		{"6. 运输路线配置",0, (void(*)())routeConfigStubWin,   0},
		{"7. 数据备份",    0, (void(*)())dataBackupStubWin,    0},
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
	return;
	WINDOW_T win = {
		220, 120, 360, 360, WHITE_COLOR, 6, {
			{230, 135, 340, 30, "订单管理",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{230, 185, 320, BTN_H, "创建订单",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{230, 245, 320, BTN_H, "订单查询与审核",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{230, 305, 320, BTN_H, "订单跟踪",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{230, 385, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);
		switch (win.current) {
		case 1: createOrderWin(); break;
		case 2: searchOrderWin(); break;
		case 3: trackOrderWin(); break;
		case 4: return;
		}
	}
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

	outtextxy(x + CTRL_PADDING, y_base + 8, inv->goods_type);
	x += col_widths[1];

	sprintf(buf, "%d", inv->quantity);
	outtextxy(x + CTRL_PADDING, y_base + 8, buf);
	x += col_widths[2];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(inv->location_id) ? inv->location_id : "--");
	x += col_widths[3];

	outtextxy(x + CTRL_PADDING, y_base + 8,
	          strlen(inv->in_time) ? inv->in_time : "--");
}

static void showInventoryList(const Inventory *head) {
	const char *headers[] = {"ID", "货物类型", "库存数量", "货位", "最后入库"};
	const int col_widths[] = {50, 100, 100, 80, 160};
	window_show_table("库存列表", headers, col_widths, 5,
	                  head, offsetof(Inventory, next), drawInventoryRow, PAGE_SIZE);
}

static int inventory_matches_query_pdf(const Inventory *inv,
                                       const char *goods_name,
                                       const char *location_id) {
	if (goods_name[0] && strstr(inv->goods_type, goods_name) == NULL) return 0;
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
	const int col_w[6] = {45, 85, 75, 70, 70, 90};
	const char *headers[6] = {"序号", "货物名称", "货物类型", "库存数量", "存储货位", "入库时间"};
	int header_h = 22, row_h = 22;
	int total_w = col_w[0] + col_w[1] + col_w[2] + col_w[3] + col_w[4] + col_w[5];
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = UI_PANEL_Y + 122;
	int start = page * 3;
	int shown = 0, idx = 0, x;
	char buf[64];

	cleardevice();
	redraw_bg();
	window_clear_frame();
	window_set_card(1);
	drawQueryFrame("智能物流管理系统库存查询界面", 315);

	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	outtextxy(UI_PANEL_X + 35, UI_PANEL_Y + 91, "货位名称:");
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
		outtextxy(x + 4, y + 5, p->goods_type); x += col_w[1];
		outtextxy(x + 4, y + 5, p->goods_type); x += col_w[2];
		sprintf(buf, "%d", p->quantity);
		outtextxy(x + 4, y + 5, buf); x += col_w[3];
		outtextxy(x + 4, y + 5, strlen(p->location_id) ? p->location_id : "--"); x += col_w[4];
		outtextxy(x + 4, y + 5, strlen(p->in_time) ? p->in_time : "--");

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

	while (1) {
		int pages = inventory_query_total_pages(goods_name, location_id);
		if (page >= pages) page = pages - 1;
		draw_inventory_query_pdf(goods_name, location_id, focus, page);
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);
		char *buf = focus == 0 ? goods_name : location_id;
		int max_len = focus == 0 ? 31 : 15;

		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x >= UI_PANEL_X + 105 && msg.x <= UI_PANEL_X + 205 &&
			    msg.y >= UI_PANEL_Y + 84 && msg.y <= UI_PANEL_Y + 108) focus = 0;
			else if (msg.x >= UI_PANEL_X + 300 && msg.x <= UI_PANEL_X + 380 &&
			         msg.y >= UI_PANEL_Y + 84 && msg.y <= UI_PANEL_Y + 108) focus = 1;
		}
		else if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) return;
			if (msg.vkcode == VK_TAB) focus = 1 - focus;
			if (msg.vkcode == VK_BACK) {
				int len = (int)strlen(buf);
				if (len > 0) { buf[len - 1] = '\0'; page = 0; }
			}
			if (msg.vkcode == VK_LEFT && page > 0) page--;
			if (msg.vkcode == VK_RIGHT && page < pages - 1) page++;
		}
		else if (msg.message == WM_CHAR) {
			char ch = (char)msg.ch;
			if (ch >= 32 && ch <= 126) {
				int len = (int)strlen(buf);
				if (len < max_len) {
					buf[len] = ch;
					buf[len + 1] = '\0';
					page = 0;
				}
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

/* ========== 仓储管理子菜单 ========== */
/* ---- 仓库窗口前向声明 ---- */
static void inboundWin();
static void outboundWin();
static void inventoryWin();
static void stocktakingWin();

static void warehouseMgmtWin() {
	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		220, 120, 360, 410, WHITE_COLOR, 6, {
			{230, 135, 340, 30, "仓储管理",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{230, 180, 320, BTN_H, "入库管理",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{230, 235, 320, BTN_H, "出库管理",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{230, 290, 320, BTN_H, "库存查询",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{230, 345, 320, BTN_H, "库存盘点",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{230, 420, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);
		switch (win.current) {
		case 1: inboundWin(); break;
		case 2: outboundWin(); break;
		case 3: inventoryWin(); break;
		case 4: stocktakingWin(); break;
		case 5: return;
		}
	}
}

/* ========== 入库窗口 ========== */
static void inboundWin() {
	Order *pending = order_svc_list_by_status(ORDER_PENDING_OUT);
	if (!pending) {
		MessageBoxA(GetHWnd(), "暂无待出库订单", "提示", MB_OK | MB_ICONINFORMATION);
	} else {
		showOrderList(pending);
		while (pending) {
			Order *tmp = pending;
			pending = pending->next;
			free(tmp);
		}
	}

	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		180, 60, 460, 440, WHITE_COLOR, 11, {
			{190, 70, 440, 30, "入库管理",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 115, 80, INPUT_H, "订单号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 115, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{190, 160, 80, INPUT_H, "入库数量：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 160, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 205, 80, INPUT_H, "货位编号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 205, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 270, BTN_W, BTN_H, "查看待出库订单",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 270, BTN_W, BTN_H, "查看库存",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{210, 370, BTN_W, BTN_H, "确认入库",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 370, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 7) {
			Order *p = order_svc_list_by_status(ORDER_PENDING_OUT);
			if (!p) {
				MessageBoxA(GetHWnd(), "暂无待出库订单", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				showOrderList(p);
				while (p) { Order *tmp = p; p = p->next; free(tmp); }
			}
		}
		else if (win.current == 8) {
			if (!inventory_list_head)
				MessageBoxA(GetHWnd(), "暂无库存记录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				showInventoryList(inventory_list_head);
		}
		else if (win.current == 9) {
			char *oid = win.controls[2].text;
			char *qty_str = win.controls[4].text;
			char *loc = win.controls[6].text;

			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			int qty = atoi(qty_str);
			if (qty <= 0) {
				MessageBoxA(GetHWnd(), "请输入有效的入库数量", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			char err[256];
			int ret = inbound_svc_execute(oid, qty, strlen(loc) ? loc : "--",
			                              err, sizeof(err));
			if (ret == 0) {
				char msg[256];
				sprintf(msg, "入库成功！\n订单号: %s\n货物: %s x%d",
				        oid,
				        order_svc_find_by_id(oid)->goods_type, qty);
				MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
				return;
			} else {
				MessageBoxA(GetHWnd(), err, "入库失败", MB_OK | MB_ICONERROR);
			}
		}
		else if (win.current == 10) {
			return;
		}
	}
}

/* ========== 出库窗口 ========== */
static void outboundWin() {
	Order *pending = order_svc_list_by_status(ORDER_PENDING_OUT);
	if (!pending) {
		MessageBoxA(GetHWnd(), "暂无待出库订单", "提示", MB_OK | MB_ICONINFORMATION);
	} else {
		showOrderList(pending);
		while (pending) {
			Order *tmp = pending;
			pending = pending->next;
			free(tmp);
		}
	}

	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		180, 60, 460, 440, WHITE_COLOR, 11, {
			{190, 70, 440, 30, "出库管理",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{190, 115, 80, INPUT_H, "订单号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 115, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{190, 160, 80, INPUT_H, "出库数量：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 160, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 205, 80, INPUT_H, "货位编号：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{270, 205, INPUT_W, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{190, 270, BTN_W, BTN_H, "查看待出库订单",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{380, 270, BTN_W, BTN_H, "查看库存",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{210, 370, BTN_W, BTN_H, "确认出库",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 370, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 7) {
			Order *p = order_svc_list_by_status(ORDER_PENDING_OUT);
			if (!p) {
				MessageBoxA(GetHWnd(), "暂无待出库订单", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				showOrderList(p);
				while (p) { Order *tmp = p; p = p->next; free(tmp); }
			}
		}
		else if (win.current == 8) {
			if (!inventory_list_head)
				MessageBoxA(GetHWnd(), "暂无库存记录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				showInventoryList(inventory_list_head);
		}
		else if (win.current == 9) {
			char *oid = win.controls[2].text;
			char *qty_str = win.controls[4].text;
			char *loc = win.controls[6].text;

			if (strlen(oid) == 0) {
				MessageBoxA(GetHWnd(), "请输入订单号", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			int qty = atoi(qty_str);
			if (qty <= 0) {
				MessageBoxA(GetHWnd(), "请输入有效的出库数量", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			char err[256];
			int ret = outbound_svc_execute(oid, qty, strlen(loc) ? loc : "--",
			                               err, sizeof(err));
			if (ret == 0) {
				char msg[256];
				sprintf(msg, "出库成功！订单 %s 已转为\"运输中\"", oid);
				MessageBoxA(GetHWnd(), msg, "提示", MB_OK | MB_ICONINFORMATION);
				return;
			} else {
				MessageBoxA(GetHWnd(), err, "出库失败", MB_OK | MB_ICONERROR);
			}
		}
		else if (win.current == 10) {
			return;
		}
	}
}

/* ========== 库存查询窗口 ========== */
static void inventoryWin() {
	inventoryQueryWinPdf();
}

/* ========== 库存盘点窗口 ========== */
static void stocktakingWin() {
	window_clear_frame();
	window_set_card(1);

	WINDOW_T win = {
		200, 120, 400, 380, WHITE_COLOR, 9, {
			{210, 130, 380, 30, "库存盘点",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{210, 175, 80, INPUT_H, "库存ID：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{290, 175, 280, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{210, 220, 80, INPUT_H, "实际数量：",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{290, 220, 280, INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 0, 0, 0, 0},
			{210, 275, 380, 25, "提示：先查看库存列表获取ID",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MUTED, LABEL, 0, 0, 0, 0},
			{210, 315, BTN_W, BTN_H, "查看库存列表",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{410, 315, BTN_W, BTN_H, "确认盘点",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{310, 375, BTN_W, BTN_H, "返回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 6) {
			if (!inventory_list_head)
				MessageBoxA(GetHWnd(), "暂无库存记录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				showInventoryList(inventory_list_head);
		}
		else if (win.current == 7) {
			char *id_str = win.controls[2].text;
			char *qty_str = win.controls[4].text;

			if (strlen(id_str) == 0 || strlen(qty_str) == 0) {
				MessageBoxA(GetHWnd(), "请输入库存ID和实际数量", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			int inv_id = atoi(id_str);
			int actual_qty = atoi(qty_str);

			Inventory *inv = inventory_list_head;
			while (inv && inv->id != inv_id) inv = inv->next;

			if (!inv) {
				MessageBoxA(GetHWnd(), "库存ID不存在", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			int sys_qty = inv->quantity;
			int diff = actual_qty - sys_qty;

			char result[384];
			sprintf(result,
			        "===== 盘点结果 =====\n\n"
			        "货物类型: %s\n"
			        "系统数量: %d\n"
			        "实际数量: %d\n"
			        "差    异: %+d\n\n"
			        "%s",
			        inv->goods_type, sys_qty, actual_qty, diff,
			        diff == 0 ? "盘点一致" : "! 存在差异，请核查！");

			int icon = diff == 0 ? MB_ICONINFORMATION : MB_ICONWARNING;
			MessageBoxA(GetHWnd(), result, "盘点结果",
			            MB_OK | icon);
		}
		else if (win.current == 8) {
			return;
		}
	}
}

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
		{"4. 运输管理", PERM_TRANSPORT,    (void(*)())transportStubWin, 0},
		{"5. 统计分析", PERM_STATISTICS,   (void(*)())statsStubWin,     0},
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
