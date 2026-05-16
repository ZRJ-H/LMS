#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "common.h"
#include "mainWin.h"

/* 用户管理子窗口，仅管理员可调用 */
static int userMgmtWin() {
	WINDOW_T win = {
	    200, 120, 400, 360, WHITE, 6, {
	        {210, 135, 380, 30, "用户管理", WHITE, WHITE, BLACK, LABEL, 0},
	        {210, 190, 180, 50, "创建用户", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
	        {400, 190, 180, 50, "查询用户", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	        {210, 260, 180, 50, "重置密码", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	        {400, 260, 180, 50, "用户列表", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	        {210, 400, 370, 50, "返回主菜单", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	    }
	};

	while (1) {
		window_show(win);
		win = window_run(win);

		if (win.current == 0) {     /* 创建用户 */
			char name[64] = {0}, pwd[64] = {0};
			InputBox(name, 32, "请输入新用户名（回车确认）", "创建用户", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			InputBox(pwd, 32, "请输入密码（6-10位字母数字）", "创建用户", NULL, 0, 0, 1);
			if (strlen(pwd) == 0) continue;

			int ret = add_user(name, pwd, ROLE_CUSTOMER);
			if (ret == -1)
				MessageBoxA(NULL, "用户名已存在！", "错误", MB_OK | MB_ICONERROR);
			else if (ret == -2)
				MessageBoxA(NULL, "密码不合法（需6-10位字母或数字）", "错误", MB_OK | MB_ICONERROR);
			else
				MessageBoxA(NULL, "用户创建成功！", "提示", MB_OK | MB_ICONINFORMATION);
		}
		else if (win.current == 1) { /* 查询用户 */
			char name[64] = {0};
			InputBox(name, 32, "请输入要查询的用户名", "查询用户", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			User *u = find_user_by_name(name);
			if (u) {
				char info[256];
				sprintf(info, "ID: %d\n用户名: %s\n角色: %s\n失败次数: %d",
				        u->id, u->name, role_to_string(u->role), u->failed_attempts);
				MessageBoxA(NULL, info, "用户信息", MB_OK | MB_ICONINFORMATION);
			} else {
				MessageBoxA(NULL, "用户不存在", "提示", MB_OK | MB_ICONWARNING);
			}
		}
		else if (win.current == 2) { /* 重置密码 */
			char name[64] = {0};
			InputBox(name, 32, "请输入要重置密码的用户名", "重置密码", NULL, 0, 0, 1);
			if (strlen(name) == 0) continue;
			User *u = find_user_by_name(name);
			if (u) {
				reset_password(u);
				MessageBoxA(NULL, "密码已重置为 888888", "提示", MB_OK | MB_ICONINFORMATION);
			} else {
				MessageBoxA(NULL, "用户不存在", "提示", MB_OK | MB_ICONWARNING);
			}
		}
		else if (win.current == 3) { /* 用户列表 */
			cleardevice();
			User *u = user_list_head;
			int y = 20;
			char buf[256];
			settextstyle(16, 10, _T("宋体"));
			outtextxy(20, y, "ID\t用户名\t角色\t状态");
			y += 24;
			while (u) {
				sprintf(buf, "%d\t%s\t%s\t%s",
				        u->id, u->name, role_to_string(u->role),
				        u->lockout_until > time(NULL) ? "锁定" : "正常");
				outtextxy(20, y, buf);
				y += 22;
				u = u->next;
			}
			outtextxy(20, y + 10, "按任意键返回...");
			getmessage(EX_KEY);
		}
		else if (win.current == 5) { /* 返回 */
			return 0;
		}
	}
}

int mainWin() {
	if (!current_user) return 0;

	char title[128];
	sprintf(title, "欢迎, %s (%s)", current_user->name,
	        role_to_string(current_user->role));

	/* ========== 管理员菜单 ========== */
	if (current_user->role == ROLE_ADMIN) {
		WINDOW_T win = {
		    220, 80, 360, 440, WHITE, 7, {
		        {230, 100, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0},
		        {240, 150, 320, 55, "用户管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
		        {240, 215, 320, 55, "订单管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {240, 280, 320, 55, "订单审核", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {240, 345, 320, 55, "仓储管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {240, 410, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {425, 410, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);

			switch (win.current) {
			case 1: userMgmtWin(); break;                 /* 用户管理 */
			case 2: MessageBoxA(NULL, "订单管理（Day5 实现）", "提示", MB_OK); break;
			case 3: MessageBoxA(NULL, "订单审核（Day6 实现）", "提示", MB_OK); break;
			case 4: MessageBoxA(NULL, "仓储管理（Day8 实现）", "提示", MB_OK); break;
			case 5: current_user = NULL; return 0;         /* 注销 → startWin */
			case 6: return -1;                              /* 退出系统 */
			}
		}
	}

	/* ========== 客服菜单 ========== */
	if (current_user->role == ROLE_SERVICE) {
		WINDOW_T win = {
		    220, 120, 360, 360, WHITE, 5, {
		        {230, 140, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0},
		        {240, 190, 320, 55, "订单受理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
		        {240, 255, 320, 55, "订单查询", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {240, 340, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {425, 340, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);
			switch (win.current) {
			case 1: MessageBoxA(NULL, "订单受理（Day5 实现）", "提示", MB_OK); break;
			case 2: MessageBoxA(NULL, "订单查询（Day6 实现）", "提示", MB_OK); break;
			case 3: current_user = NULL; return 0;
			case 4: return -1;
			}
		}
	}

	/* ========== 仓库管理员菜单 ========== */
	if (current_user->role == ROLE_WAREHOUSE) {
		WINDOW_T win = {
		    220, 140, 360, 320, WHITE, 4, {
		        {230, 160, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0},
		        {240, 210, 320, 55, "出入库管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
		        {240, 290, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {425, 290, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0},
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
		        {230, 160, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0},
		        {240, 210, 320, 55, "调度管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
		        {240, 290, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {425, 290, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0},
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
		        {230, 160, 340, 30, "", WHITE, WHITE, BLACK, LABEL, 0},
		        {240, 210, 320, 55, "查看订单", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
		        {240, 290, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
		        {425, 290, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0},
		    }
		};
		strcpy(win.controls[0].text, title);

		while (1) {
			window_show(win);
			win = window_run(win);
			switch (win.current) {
			case 1: MessageBoxA(NULL, "查看订单（Day6 实现）", "提示", MB_OK); break;
			case 2: current_user = NULL; return 0;
			case 3: return -1;
			}
		}
	}

	return 0;
}
