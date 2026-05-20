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

/* ------- 角色选择 COMBO 窗口 ------- */
static UserRole selectRoleWin() {
	WINDOW_T win = {
	    230, 180, 340, 220, WHITE, 3, {
	        {240, 190, 320, 30, "请选择用户角色", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {240, 250, 320, 40, "管理员|客服|仓储员|调度员|客户", CYAN, LIGHTCYAN, WHITE, COMBO, 1, 0, 4},
	        {280, 330, 240, 50, "确认创建", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	    }
	};
	UserRole roles[] = {ROLE_ADMIN, ROLE_SERVICE, ROLE_WAREHOUSE, ROLE_DISPATCHER, ROLE_CUSTOMER};

	while (1) {
		window_show(win);
		win = window_run(win);
		if (win.current == 2)
			return roles[win.controls[1].sel_index];
	}
}

/* ------- 用户表格行绘制回调 -------
 * 供 window_show_table 调用，在图形窗口绘制单行用户数据
 * 列顺序：ID | 用户名 | 角色 | 锁定状态 */
static void drawUserRow(const void *record, int row_idx, int y_base,
                        int table_x, const int *col_widths, int ncols) {
	const User *u = (const User *)record;
	char buf[64];
	int x = table_x;
	settextcolor(BLACK);
	settextstyle(14, 8, _T("宋体"));

	sprintf(buf, "%d", u->id);
	outtextxy(x + 5, y_base + 5, buf);
	x += col_widths[0];

	outtextxy(x + 5, y_base + 5, u->name);
	x += col_widths[1];

	outtextxy(x + 5, y_base + 5, (char *)role_to_string(u->role));
	x += col_widths[2];

	outtextxy(x + 5, y_base + 5, u->lockout_until > time(NULL) ? "锁定" : "正常");
}

/* ------- 用户列表分页表格 -------
 * 每页 8 行，复用 window_show_table */
static void showUserList(const User *head) {
	const char *headers[] = {"ID", "用户名", "角色", "状态"};
	const int col_widths[] = {60, 150, 120, 80};
	window_show_table("用户列表", headers, col_widths, 4,
	                  head, offsetof(User, next), drawUserRow, 8);
}

/* ------- 用户条件查询 -------
 * 按用户名关键字筛选，关键字为空时展示全部 */
static void searchUserWin() {
	WINDOW_T win = {
	    220, 160, 360, 280, WHITE, 5, {
	        {230, 170, 340, 30, "用户查询", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {230, 210, 80, 50, "用户名：", WHITE, WHITE, CYAN, LABEL, 0, 0, 0},
	        {330, 210, 240, 50, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0},
	        {230, 310, 160, 50, "查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {415, 310, 160, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
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
				/* 复制匹配记录组成临时链表 */
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

/* ========== 用户管理子窗口 ==========
 * 管理员专属：创建用户 / 查询用户 / 重置密码 / 用户列表 */
static int userMgmtWin() {
	WINDOW_T win = {
	    200, 120, 400, 360, WHITE, 6, {
	        {210, 135, 380, 30, "用户管理", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {210, 190, 180, 50, "创建用户", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0},
	        {400, 190, 180, 50, "查询用户", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {210, 260, 180, 50, "重置密码", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {400, 260, 180, 50, "用户列表", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {210, 400, 370, 50, "返回主菜单", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 0) {
			/* 创建用户 */
			char name[64] = {0}, pwd[64] = {0}, err[128];
			InputBox(name, 32, "请输入新用户名（回车确认）", "创建用户", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			InputBox(pwd, 32, "请输入密码（6-10位字母数字）", "创建用户", NULL, 0, 0, 1);
			if (strlen(pwd) == 0) continue;

			UserRole role = selectRoleWin();

			int ret = user_svc_create(name, pwd, role, err, sizeof(err));
			if (ret == 0)
				MessageBoxA(NULL, "用户创建成功！", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(NULL, err, "错误", MB_OK | MB_ICONERROR);
		}
		else if (win.current == 1) {
			/* 查询用户 */
			searchUserWin();
		}
		else if (win.current == 2) {
			/* 重置密码 — 默认值 888888 */
			char name[64] = {0};
			InputBox(name, 32, "请输入要重置密码的用户名", "重置密码", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			if (user_svc_reset_password(name) == 0)
				MessageBoxA(NULL, "密码已重置为 888888", "提示", MB_OK | MB_ICONINFORMATION);
			else
				MessageBoxA(NULL, "用户不存在", "提示", MB_OK | MB_ICONWARNING);
		}
		else if (win.current == 3) {
			/* 用户列表 */
			showUserList(user_svc_list_all());
		}
		else if (win.current == 5) {
			/* 返回主菜单 */
			return 0;
		}
	}
}

/* ========== 忘记密码窗口 ==========
 * 从启动窗口进入，输入用户名即可重置密码为 888888
 * 无需登录态，任何人均可使用 */
int forgotPasswordWin() {
	WINDOW_T win = {
	    220, 200, 360, 260, WHITE, 5, {
	        {230, 210, 340, 30, "找回密码", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
	        {230, 260, 80, 50, "用户名：", WHITE, WHITE, CYAN, LABEL, 0, 0, 0},
	        {320, 260, 240, 50, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1, 0, 0},
	        {240, 340, 160, 50, "重置密码", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
	        {420, 340, 140, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
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
			return 0;  /* 返回启动窗口 */
		}
	}
}

/* ============================================================
 *  主菜单路由器 — 根据角色渲染不同的功能入口
 *
 *  角色 → 菜单映射：
 *    ADMIN      用户管理 / 创建订单 / 订单查询 / 订单审核 / 仓储管理 / ... / 退出
 *    SERVICE    订单受理 / 订单查询 / 注销 / 退出
 *    WAREHOUSE  出入库管理 / 注销 / 退出
 *    DISPATCHER 调度管理 / 注销 / 退出
 *    CUSTOMER   查看订单 / 注销 / 退出
 *
 *  订单相关操作已解耦到 orderWin.cpp，通过 orderWin.h 调用
 * ============================================================ */
int mainWin() {
	if (!current_user) return 0;

	char title[128];
	sprintf(title, "欢迎, %s (%s)", current_user->name,
	        role_to_string(current_user->role));

	/* ========== 管理员菜单 ========== */
	if (current_user->role == ROLE_ADMIN) {
		WINDOW_T win = {
		    220, 60, 360, 480, WHITE, 8, {
		        {230, 75, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {240, 115, 320, 50, "用户管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0},
		        {240, 172, 320, 50, "创建订单", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {240, 229, 320, 50, "订单查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {240, 286, 320, 50, "订单审核", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {240, 343, 320, 50, "仓储管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {240, 405, 155, 50, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {425, 405, 135, 50, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0, 0, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);

			switch (win.current) {
			case 1: userMgmtWin(); break;
			case 2: createOrderWin(); break;
			case 3: searchOrderWin(); break;
			case 4: auditOrderWin(); break;
			case 5: MessageBoxA(NULL, "仓储管理（Day8 实现）", "提示", MB_OK); break;
			case 6: current_user = NULL; return 0;
			case 7: return -1;
			}
		}
	}

	/* ========== 客服菜单 ========== */
	if (current_user->role == ROLE_SERVICE) {
		WINDOW_T win = {
		    220, 120, 360, 360, WHITE, 5, {
		        {230, 140, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {240, 190, 320, 55, "订单受理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0},
		        {240, 255, 320, 55, "订单查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {240, 340, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {425, 340, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0, 0, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);
			switch (win.current) {
			case 1: createOrderWin(); break;
			case 2: searchOrderWin(); break;
			case 3: current_user = NULL; return 0;
			case 4: return -1;
			}
		}
	}

	/* ========== 仓库管理员菜单 ========== */
	if (current_user->role == ROLE_WAREHOUSE) {
		WINDOW_T win = {
		    220, 140, 360, 320, WHITE, 4, {
		        {230, 160, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {240, 210, 320, 55, "出入库管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0},
		        {240, 290, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {425, 290, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0, 0, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);
			switch (win.current) {
			case 1: MessageBoxA(NULL, "出入库管理（Day8 实现）", "提示", MB_OK); break;
			case 2: current_user = NULL; return 0;
			case 3: return -1;
			}
		}
	}

	/* ========== 调度员菜单 ========== */
	if (current_user->role == ROLE_DISPATCHER) {
		WINDOW_T win = {
		    220, 140, 360, 320, WHITE, 4, {
		        {230, 160, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {240, 210, 320, 55, "调度管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0},
		        {240, 290, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {425, 290, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0, 0, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);
			switch (win.current) {
			case 1: MessageBoxA(NULL, "调度管理（Day10 实现）", "提示", MB_OK); break;
			case 2: current_user = NULL; return 0;
			case 3: return -1;
			}
		}
	}

	/* ========== 客户菜单 ========== */
	if (current_user->role == ROLE_CUSTOMER) {
		WINDOW_T win = {
		    220, 140, 360, 320, WHITE, 4, {
		        {230, 160, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0, 0, 0},
		        {240, 210, 320, 55, "查看订单", CYAN, LIGHTCYAN, WHITE, BUTTON, 1, 0, 0},
		        {240, 290, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0, 0, 0},
		        {425, 290, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0, 0, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);
			switch (win.current) {
			case 1: searchOrderWin(); break;
			case 2: current_user = NULL; return 0;
			case 3: return -1;
			}
		}
	}

	return 0;
}
