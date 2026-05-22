#include <graphics.h>
#include <conio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../public/common.h"
#include "loginWin.h"
#include "orderWin.h"
#include "../view/control.h"
#include "../service/user_service.h"

/* ============================================================
 *  共用页面装饰 — 主菜单和二级子界面复用
 *  在 window_show() 之后、window_run() 之前调用
 *  绘制：蓝色外框 + 标题 + 用户信息栏 + 底部分隔线 + 操作说明
 * ============================================================ */
static void drawPageDecor(const char *title, int btn_bottom) {
	/* 蓝色外框 */
	setlinecolor(BLUE);
	rectangle(60, 20, 740, 580);

	/* 标题 */
	settextstyle(26, 14, _T("黑体"));
	settextcolor(BLACK);
	int tw = (int)strlen(title) * 13;
	outtextxy(60 + (680 - tw) / 2, 50, title);

	/* 用户信息栏 — 左侧 */
	settextstyle(16, 8, _T("黑体"));
	settextcolor(BLACK);
	char info_left[128];
	sprintf(info_left, "当前用户：%s(%s)",
	        current_user->name, role_to_string(current_user->role));
	outtextxy(90, 105, info_left);

	/* 用户信息栏 — 右侧 */
	char info_right[128];
	sprintf(info_right, "登录时间：%s",
	        login_time_str[0] ? login_time_str : "----");
	outtextxy(440, 105, info_right);

	/* 底部分隔线 — 动态位置，在最后一个按钮下方 25px */
	int sep_y  = btn_bottom + 25;
	int foot_y = btn_bottom + 50;
	/* 确保不超出底部边框（580），若溢出则贴底 */
	if (foot_y > 572) { foot_y = 572; sep_y = foot_y - 22; }
	setlinecolor(BLUE);
	line(80, sep_y, 720, sep_y);

	/* 底部操作说明 */
	settextstyle(14, 8, _T("黑体"));
	settextcolor(BLUE);
	const char *footer = "操作说明：通过上下键切换菜单，按Enter键进入";
	int fw = (int)strlen(footer) * 7;
	outtextxy(60 + (680 - fw) / 2, foot_y, footer);
}

/* ============================================================
 *  菜单项定义
 * ============================================================ */
typedef struct {
	const char *text;	//菜单文本
	int         perm;       /* 所需权限位掩码，0=所有人可见 */
	void      (*action)();  /* 回调函数 */
	int         is_exit;    /* 1=退出系统（return -1） */
} MenuItem;

/* ------- 占位窗口（尚未实现的模块）------- */
static void warehouseStubWin() {
	MessageBoxA(NULL, "仓储管理（Day8 实现）", "提示", MB_OK);
}
static void transportStubWin() {
	MessageBoxA(NULL, "运输管理（Day10 实现）", "提示", MB_OK);
}
static void statsStubWin() {
	MessageBoxA(NULL, "统计分析（Day11 实现）", "提示", MB_OK);
}
static void serviceStubWin() {
	MessageBoxA(NULL, "客户服务（Day9 实现）", "提示", MB_OK);
}
static void roleMgmtStubWin() {
	MessageBoxA(NULL, "角色管理（Day11 实现）", "提示", MB_OK);
}
static void warehouseConfigStubWin() {
	MessageBoxA(NULL, "仓库配置（Day8 实现）", "提示", MB_OK);
}
static void routeConfigStubWin() {
	MessageBoxA(NULL, "运输路线配置（Day10 实现）", "提示", MB_OK);
}
static void dataBackupStubWin() {
	MessageBoxA(NULL, "数据备份（Day12 实现）", "提示", MB_OK);
}

/* ------- 角色选择 COMBO 窗口 ------- */
static void selectRoleWin(UserRole *out_role) {
	WINDOW_T win = {
	    230, 180, 340, 220, WHITE, 3, {
	        {240, 190, 320, 30, "请选择用户角色", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {240, 250, 320, 40, "管理员|客服|仓储员|调度员|客户", CYAN, LIGHTCYAN, WHITE, COMBO, 1, 0, 4, BLACK},
	        {280, 330, 240, 50, "确认创建", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	    }
	};
	UserRole roles[] = {ROLE_ADMIN, ROLE_SERVICE, ROLE_WAREHOUSE, ROLE_DISPATCHER, ROLE_CUSTOMER};

	while (1) {
		window_show(win);
		win = window_run(win);
		if (win.current == 2) {//确认创建按钮被按下
			*out_role = roles[win.controls[1].sel_index];//根据COMBO框的选中索引获取对应的角色，并通过输出参数返回
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
	settextcolor(BLACK);
	settextstyle(14, 8, _T("黑体"));

	sprintf(buf, "%d", u->id);
	outtextxy(x + 5, y_base + 5, buf);
	x += col_widths[0];

	outtextxy(x + 5, y_base + 5, u->name);
	x += col_widths[1];

	outtextxy(x + 5, y_base + 5, (char *)role_to_string(u->role));
	x += col_widths[2];

	outtextxy(x + 5, y_base + 5, u->lockout_until > time(NULL) ? "锁定" : "正常");
}

/* ------- 用户列表分页表格 ------- */
static void showUserList(const User *head) {
	const char *headers[] = {"ID", "用户名", "角色", "状态"};
	const int col_widths[] = {60, 150, 120, 80};
	window_show_table("用户列表", headers, col_widths, 4,
	                  head, offsetof(User, next), drawUserRow, 5);
}

/* ------- 用户条件查询 ------- */
static void searchUserWin() {
	WINDOW_T win = {
	    220, 160, 360, 280, WHITE, 5, {
	        {230, 170, 340, 30, "用户查询", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {230, 210, 80, 50, "用户名：", WHITE, WHITE, CYAN, LABEL, 0, 0, 0, BLACK},
	        {330, 210, 240, 50, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0, BLACK},
	        {230, 310, 160, 50, "查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	        {415, 310, 160, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
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
	/* 菜单项定义 */
	MenuItem sm[8] = {
	    {"1. 创建用户",    0, (void(*)())NULL,               0},
	    {"2. 用户查询",    0, (void(*)())NULL,               0},
	    {"3. 密码重置",    0, (void(*)())NULL,               0},
	    {"4. 角色管理",    0, (void(*)())roleMgmtStubWin,    0},
	    {"5. 仓库配置",    0, (void(*)())warehouseConfigStubWin, 0},
	    {"6. 运输路线配置",0, (void(*)())routeConfigStubWin,  0},
	    {"7. 数据备份",    0, (void(*)())dataBackupStubWin,   0},
	    {"8. 返回上级",    0, NULL,                           0},
	};

	/* 网格坐标（与主菜单完全一致） */
	int col_x[2] = {100, 420};
	int row_y[4] = {165, 230, 295, 360};

	WINDOW_T win;
	win.x = 60;  win.y = 20;  win.width = 680;  win.height = 560;
	win.bgColor = WHITE;
	win.count = 8;

	for (int i = 0; i < 8; i++) {
		CONTROL_T c;
		c.x = col_x[i % 2];  c.y = row_y[i / 2];
		c.width = 280;  c.height = 50;
		strcpy(c.text, sm[i].text);
		c.type = BUTTON;
		c.bgColor1 = BLUE;  c.bgColor2 = WHITE;
		c.textColor = WHITE;  c.textColor2 = 0x000001;
		c.state = (i == 0) ? 1 : 0;
		c.visible = 0;  c.sel_index = 0;
		win.controls[i] = c;
	}
	win.current = 0;

	while (1) {
		window_show(win);
		drawPageDecor("智能物流管理系统系统管理界面", 410);  /* 最后按钮 bottom = 360+50 */
		win = window_run(win);

		switch (win.current) {
		case 0: {
			/* 创建用户 */
			char name[64] = {0}, pwd[64] = {0}, err[128];
			InputBox(name, 32, "请输入新用户名（回车确认）", "创建用户", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			InputBox(pwd, 32, "请输入密码（6-10位字母数字）", "创建用户", NULL, 0, 0, 1);
			if (strlen(pwd) == 0) continue;
			UserRole role;
			selectRoleWin(&role);
			int ret = user_svc_create(name, pwd, role, err, sizeof(err));
			if (ret == 0)
				MessageBoxA(NULL, "用户创建成功！", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(NULL, err, "错误", MB_OK | MB_ICONERROR);
			break;
		}
		case 1: searchUserWin(); break;
		case 2: {
			/* 密码重置 */
			char name[64] = {0};
			InputBox(name, 32, "请输入要重置密码的用户名", "重置密码", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			if (user_svc_reset_password(name) == 0)
				MessageBoxA(NULL, "密码已重置为 888888", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(NULL, "用户不存在", "提示", MB_OK | MB_ICONWARNING);
			break;
		}
		case 7: return;  /* 返回上级 */
		default: if (sm[win.current].action) sm[win.current].action(); break;
		}
	}
}

/* ========== 订单管理子窗口 ========== */
static void orderMgmtWin() {
	WINDOW_T win = {
	    220, 160, 360, 320, WHITE, 5, {
	        {230, 175, 340, 30, "订单管理", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {240, 220, 320, 50, "创建订单", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0, BLACK},
	        {240, 280, 320, 50, "订单查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	        {240, 345, 250, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);
		switch (win.current) {
		case 1: createOrderWin(); break;
		case 2: searchOrderWin(); break;
		case 3: return;
		}
	}
}

/* ========== 密码修改窗口 ========== */
static void changePasswordWin() {
	WINDOW_T win = {
	    220, 160, 360, 310, WHITE, 7, {
	        {230, 170, 340, 30, "修改密码", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {230, 215, 80, 35, "旧密码：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {310, 215, 250, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT_PWD, 1, 0, 0, BLACK},
	        {230, 265, 80, 35, "新密码：", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {310, 265, 250, 35, "", CYAN, LIGHTCYAN, WHITE, EDIT_PWD, 0, 0, 0, BLACK},
	        {230, 340, 160, 50, "确认修改", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	        {420, 340, 140, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 5) {
			char *old_pwd = win.controls[2].text;
			char *new_pwd = win.controls[4].text;

			if (strlen(old_pwd) == 0 || strlen(new_pwd) == 0) {
				MessageBoxA(NULL, "密码不能为空", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			/* 校验旧密码 */
			char hash[33];
			md5_hash(old_pwd, hash);
			if (strcmp(hash, current_user->password) != 0) {
				MessageBoxA(NULL, "旧密码错误", "提示", MB_OK | MB_ICONWARNING);
				win.controls[2].text[0] = '\0';
				continue;
			}

			/* 校验新密码合法性 */
			if (validate_password(new_pwd) != 0) {
				MessageBoxA(NULL, "新密码须为6-10位字母或数字", "提示", MB_OK | MB_ICONWARNING);
				win.controls[4].text[0] = '\0';
				continue;
			}

			/* 更新密码 */
			md5_hash(new_pwd, hash);
			strcpy(current_user->password, hash);
			user_svc_save();
			MessageBoxA(NULL, "密码修改成功！", "提示", MB_OK | MB_ICONINFORMATION);
			return;
		}
		else if (win.current == 6) {
			return;
		}
	}
}

/* ========== 忘记密码窗口 ========== */
int forgotPasswordWin() {
	WINDOW_T win = {
	    220, 200, 360, 260, WHITE, 5, {
	        {230, 210, 340, 30, "找回密码", WHITE, WHITE, BLACK, LABEL, 0, 0, 0, BLACK},
	        {230, 260, 80, 50, "用户名：", WHITE, WHITE, CYAN, LABEL, 0, 0, 0, BLACK},
	        {320, 260, 240, 50, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0, BLACK},
	        {240, 340, 160, 50, "重置密码", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	        {420, 340, 140, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0, BLACK},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 3) {
			char *name = win.controls[2].text;
			if (strlen(name) == 0) {
				MessageBoxA(NULL, "请输入用户名", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}
			if (user_svc_reset_password(name) == 0)
				MessageBoxA(NULL, "密码已重置为 888888，请返回登录", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(NULL, "用户名不存在", "提示", MB_OK | MB_ICONWARNING);
		}
		else if (win.current == 4) {
			return 0;
		}
	}
}

/* ============================================================
 *  主菜单路由器 — 动态紧凑网格 + 权限过滤
 *
 *  根据角色权限筛选可见菜单项，按 2 列网格紧凑排列、
 *  垂直居中，不留空白空洞。配色：白底 + 蓝边框 + 蓝选中/黑未选中
 * ============================================================ */
int mainWin() {
	if (!current_user) return 0;

	int p = role_get_permissions(current_user->role);

	/* 菜单定义 */
	MenuItem menu[8] = {
	    {"1. 系统管理", PERM_USER_MANAGE,  (void(*)())sysAdminWin,      0},
	    {"2. 订单管理", PERM_ORDER_MANAGE, (void(*)())orderMgmtWin,     0},
	    {"3. 仓储管理", PERM_WAREHOUSE,    (void(*)())warehouseStubWin, 0},
	    {"4. 运输管理", PERM_TRANSPORT,    (void(*)())transportStubWin, 0},
	    {"5. 统计分析", PERM_STATISTICS,   (void(*)())statsStubWin,     0},
	    {"6. 客户服务", PERM_ORDER_MANAGE, (void(*)())serviceStubWin,   0},
	    {"7. 密码修改", 0,                 (void(*)())changePasswordWin,0},
	    {"8. 退出系统", 0,                 NULL,                        1},
	};

	/* 第一步：筛出可见项，记录原索引 */
	MenuItem *vis[8];
	int vis_idx[8];
	int n = 0;
	for (int i = 0; i < 8; i++) {
		if (menu[i].perm == 0 || (p & menu[i].perm)) {
			vis[n] = &menu[i];
			vis_idx[n] = i;
			n++;
		}
	}

	/* 第二步：按可见数量计算垂直居中 */
	int rows = (n + 1) / 2;
	int grid_h = rows * 50 + (rows - 1) * 20;       /* 按钮高50 + 行距20 */
	int start_y = 180 + (400 - grid_h) / 2;          /* 在 180~580 垂直居中 */
	int col_x[2] = {100, 420};

	/* 第三步：只创建可见按钮 */
	WINDOW_T win;
	win.x = 60;  win.y = 20;  win.width = 680;  win.height = 560;
	win.bgColor = WHITE;
	win.count = n;

	for (int i = 0; i < n; i++) {
		CONTROL_T c;
		c.x = col_x[i % 2];
		c.y = start_y + (i / 2) * 70;
		c.width = 280;  c.height = 50;
		strcpy(c.text, vis[i]->text);
		c.type = BUTTON;
		c.bgColor1 = BLUE;    c.bgColor2 = WHITE;
		c.textColor = WHITE;  c.textColor2 = 0x000001;
		c.state = (i == 0) ? 1 : 0;
		c.visible = 0;  c.sel_index = 0;
		win.controls[i] = c;
	}
	win.current = 0;

	while (1) {
		window_show(win);
		drawPageDecor("智能物流管理系统主菜单界面", start_y + (rows - 1) * 70 + 50);
		win = window_run(win);
		int idx = vis_idx[win.current];  /* 控件索引 → 原菜单索引 */

		if (menu[idx].is_exit) return -1;
		if (menu[idx].action) menu[idx].action();
	}
}
