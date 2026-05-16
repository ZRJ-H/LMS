#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "common.h"
#include "loginWin.h"
#include "../view/control.h"
int loginWin(){
	WINDOW_T loginWin = {
	    240, 240, 290, 180, WHITE, 6, {
	        {245, 250, 80, 50, "用户名：", WHITE, WHITE, CYAN, LABEL, 0},
	        {245, 305, 80, 50, "密  码：", WHITE, WHITE, CYAN, LABEL, 0},
	        {320, 250, 80, 50, "", CYAN, LIGHTCYAN, WHITE, EDIT, 1},
	        {320, 305, 80, 50, "", CYAN, LIGHTCYAN, WHITE, EDIT_PWD, 0},
	        {245, 360, 130, 50, "登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	        {390, 360, 130, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	    }
	};

	while (1) {
		window_show(loginWin);
		loginWin = window_run(loginWin);

		if (loginWin.current == 4) {          /* 登录按钮 */
			char *username = loginWin.controls[2].text;
			char *password = loginWin.controls[3].text;

			/* 空值检查 */
			if (strlen(username) == 0 || strlen(password) == 0) {
				MessageBoxA(NULL, "账号或密码不能为空！", "提示", MB_OK | MB_ICONWARNING);
				continue;
			}

			/* 密码格式校验 */
			int v = validate_password(password);
			if (v == -1) {
				MessageBoxA(NULL, "密码长度须为 6-10 位", "提示", MB_OK | MB_ICONWARNING);
				loginWin.controls[3].text[0] = '\0';
				continue;
			}
			if (v == -2) {
				MessageBoxA(NULL, "密码只能包含英文字母和数字", "提示", MB_OK | MB_ICONWARNING);
				loginWin.controls[3].text[0] = '\0';
				continue;
			}

			/* 验证凭据 */
			User *u = find_user_by_name(username);
			if (u) {
				/* 检查锁定 */
				if (u->lockout_until > 0 && time(NULL) < u->lockout_until) {
					MessageBoxA(NULL, "账号已被锁定 5 分钟，请稍后再试", "提示", MB_OK | MB_ICONWARNING);
					loginWin.controls[3].text[0] = '\0';
					continue;
				}

				if (strcmp(password, u->password) == 0) {
					/* 登录成功 */
					current_user = u;
					u->failed_attempts = 0;
					u->lockout_until = 0;
					return 2;               /* 跳转到主菜单 */
				}

				/* 登录失败 */
				u->failed_attempts++;
				if (u->failed_attempts >= 3) {
					u->lockout_until = time(NULL) + 300;
					MessageBoxA(NULL, "密码错误 3 次，账号已锁定 5 分钟", "提示", MB_OK | MB_ICONWARNING);
				} else {
					char msg[64];
					sprintf(msg, "账号或密码错误（剩余 %d 次机会）", 3 - u->failed_attempts);
					MessageBoxA(NULL, msg, "提示", MB_OK | MB_ICONWARNING);
				}
			} else {
				MessageBoxA(NULL, "账号或密码错误", "提示", MB_OK | MB_ICONWARNING);
			}

			/* 清空密码框，重新输入 */
			loginWin.controls[3].text[0] = '\0';
			continue;
		}
		else if (loginWin.current == 5) {    /* 返回按钮 */
			return 0;                        /* 回到启动窗口 */
		}
	}
}
