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
static void searchUserWin() {
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
	int sub_col_x[2] = {MENU_COL_LEFT - 60, MENU_COL_RIGHT - 60};

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

	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统 — 系统管理");
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

/* ========== 订单管理子窗口 ========== */
static void orderMgmtWin() {
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
	if (!inventory_list_head) {
		MessageBoxA(GetHWnd(), "暂无库存记录", "提示", MB_OK | MB_ICONINFORMATION);
		return;
	}
	showInventoryList(inventory_list_head);
}

/* ========== 库存盘点窗口 ========== */
static void stocktakingWin() {
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

	/* 主菜单不使用白卡片（背景图透出） */
	window_set_card(0);

	while (1) {
		window_show(win);
		drawPageHeader("智能物流管理系统 — 主菜单");
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
