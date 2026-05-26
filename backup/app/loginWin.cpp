#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "loginWin.h"
#include "../public/common.h"
#include "../public/ui_config.h"
#include "../service/user_service.h"

/* ---- 校验：长度 6-10 位且全部为数字/字母 ---- */
static int is_valid_input(const char *s) {
	int len = (int)strlen(s);
	if (len < 4 || len > 16) return 0;
	for (int i = 0; i < len; i++) {
		char ch = s[i];
		if (!((ch >= '0' && ch <= '9') ||
		      (ch >= 'a' && ch <= 'z') ||
		      (ch >= 'A' && ch <= 'Z')))
			return 0;
	}
	return 1;
}

int loginWin() {
	WINDOW_T win = {
		190, 190, 440, 260, WHITE_COLOR, 6, {
			/* 0: 账号标签 */
			{(int)LOGIN_LABEL_X, 225, 60, INPUT_H, "账号:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			/* 1: 账号输入框（初始焦点） */
			{(int)LOGIN_USER_X, (int)LOGIN_USER_Y, (int)INPUT_W, (int)INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			/* 2: 密码标签 */
			{(int)LOGIN_LABEL_X, 285, 60, INPUT_H, "密码:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			/* 3: 密码输入框（掩码，右侧有显隐切换方块） */
			{(int)LOGIN_PWD_X, (int)LOGIN_PWD_Y, (int)INPUT_W, (int)INPUT_H, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT_PWD, 0, 0, 0, 0},
			/* 4: 登录按钮 */
			{280, (int)LOGIN_BTN_Y, (int)BTN_W, (int)BTN_H, "登  录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			/* 5: 返回按钮 */
			{460, (int)LOGIN_BTN_Y, (int)BTN_W, (int)BTN_H, "返  回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);

		/* 大标题 — 绘于窗口之上 */
		settextstyle(FONT_TITLE_H, 0, _T("黑体"));
		settextcolor(TEXT_MAIN);
		const char *title = "智能物流管理系统";
		int tx = (WIN_W - textwidth(title)) / 2;
		outtextxy(tx, LOGIN_TITLE_Y, title);

		win = window_run(win);

		if (win.current == 4) {
			/* 登录按钮 */
			char *username = win.controls[1].text;
			char *password = win.controls[3].text;

			/* 空值检查 */
			if (strlen(username) == 0 || strlen(password) == 0) {
				MessageBoxA(GetHWnd(),
				            "账号或密码不能为空",
				            "提示", MB_OK | MB_ICONWARNING);
				win.controls[3].text[0] = '\0';
				for (int j = 0; j < win.count; j++)
					win.controls[j].state = (j == 3) ? 1 : 0;
				win.current = 3;
				continue;
			}

			/* 格式检查：6-10 位字母数字 */
			if (!is_valid_input(username) || !is_valid_input(password)) {
				MessageBoxA(GetHWnd(),
				            "账号或密码格式不正确（需4-16位字母或数字）",
				            "提示", MB_OK | MB_ICONWARNING);
				win.controls[3].text[0] = '\0';
				for (int j = 0; j < win.count; j++)
					win.controls[j].state = (j == 3) ? 1 : 0;
				win.current = 3;
				continue;
			}

			/* 调用服务层认证 */
			char err_msg[128] = {0};
			User *u = user_svc_auth(username, password, err_msg, sizeof(err_msg));
			if (!u) {
				MessageBoxA(GetHWnd(), err_msg, "提示", MB_OK | MB_ICONWARNING);
				win.controls[3].text[0] = '\0';
				for (int j = 0; j < win.count; j++)
					win.controls[j].state = (j == 3) ? 1 : 0;
				win.current = 3;
				continue;
			}

			/* 登录成功 */
			current_user = u;
			time_t now = time(NULL);
			strftime(login_time_str, sizeof(login_time_str),
			         "%Y-%m-%d %H:%M", localtime(&now));
			return 2;  /* → mainWin */
		}
		else if (win.current == 5) {
			return 0;  /* 返回 → startWin */
		}
	}
}
