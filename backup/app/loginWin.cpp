#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "public/common.h"
#include "loginWin.h"
#include "../view/control.h"
#include "../service/user_service.h"

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
			char err_msg[128] = {0};

			User *u = user_svc_auth(username, password, err_msg, sizeof(err_msg));
			if (u) {
				current_user = u;
				return 2;                  /* 跳转到主菜单 */
			}

			/* 登录失败 — 弹错误信息 */
			MessageBoxA(NULL, err_msg, "提示", MB_OK | MB_ICONWARNING);

			/* 清空密码框，焦点回到密码输入框 */
			loginWin.controls[3].text[0] = '\0';
			for (int j = 0; j < loginWin.count; j++)
				loginWin.controls[j].state = (j == 3) ? 1 : 0;
			loginWin.current = 3;
			continue;
		}
		else if (loginWin.current == 5) {    /* 返回按钮 */
			return 0;                        /* 回到启动窗口 */
		}
	}
}
