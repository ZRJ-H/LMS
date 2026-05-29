#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "loginWin.h"
#include "../public/common.h"
#include "../public/ui_config.h"
#include "../service/user_service.h"

static int is_valid_chars(const char *s) {
	int len = (int)strlen(s);
	if (len < 1 || len > 16) return 0;
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
	const int panel_x = 170;
	const int panel_y = 165;
	const int panel_w = 460;
	const int panel_h = 320;
	const int label_x = panel_x + 55;
	const int input_x = panel_x + 125;
	const int input_w = 250;
	const int btn_w = 115;

	window_clear_frame();
	window_set_card(0);
	window_set_frame(panel_x, panel_y, panel_w, panel_h);

	char date_str[32];
	time_t date_now = time(NULL);
	strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&date_now));

	WINDOW_T win = {
		panel_x, panel_y, panel_w, panel_h, WHITE_COLOR, 10, {
			{panel_x + 70, panel_y + 38, 320, 30, "智能物流管理系统登录界面",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{panel_x + 178, panel_y + 86, 140, 24, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MUTED, LABEL, 0, 0, 0, 0},
			{label_x, panel_y + 145, 55, 30, "账号:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{input_x, panel_y + 138, input_w, 30, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT, 1, 0, 0, 0},
			{label_x, panel_y + 195, 55, 30, "密码:",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{input_x, panel_y + 188, input_w, 30, "",
			 WHITE_COLOR, INPUT_BG, BLACK_COLOR, EDIT_PWD, 0, 0, 0, 0},
			{panel_x + 75, panel_y + 255, btn_w, 35, "登  录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{panel_x + 270, panel_y + 255, btn_w, 35, "返  回",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{panel_x + 10, panel_y + 10, 1, 1, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
			{panel_x + 10, panel_y + 10, 1, 1, "",
			 WHITE_COLOR, WHITE_COLOR, TEXT_MAIN, LABEL, 0, 0, 0, 0},
		}
	};
	strcpy(win.controls[1].text, date_str);

	while (1) {
		window_show(win);
		win = window_run(win);
		if (win.current == 6) {
			char *username = win.controls[3].text;
			char *password = win.controls[5].text;

			if (strlen(username) == 0 || strlen(password) == 0) {
				MessageBoxA(GetHWnd(), "账号或密码不能为空", "提示", MB_OK | MB_ICONWARNING);
				win.controls[5].text[0] = '\0';
				win.current = 5;
				continue;
			}
			if (!is_valid_chars(username)) {
				MessageBoxA(GetHWnd(), "账号只能包含字母或数字", "提示", MB_OK | MB_ICONWARNING);
				win.controls[5].text[0] = '\0';
				win.current = 5;
				continue;
			}
			if ((int)strlen(password) < 6 || !is_valid_chars(password)) {
				MessageBoxA(GetHWnd(), "密码需为 6-16 位字母或数字", "提示", MB_OK | MB_ICONWARNING);
				win.controls[5].text[0] = '\0';
				win.current = 5;
				continue;
			}

			char err_msg[128] = {0};
			User *u = user_svc_auth(username, password, err_msg, sizeof(err_msg));
			if (!u) {
				MessageBoxA(GetHWnd(), err_msg, "提示", MB_OK | MB_ICONWARNING);
				win.controls[5].text[0] = '\0';
				win.current = 5;
				continue;
			}

			current_user = u;
			time_t now = time(NULL);
			strftime(login_time_str, sizeof(login_time_str), "%Y-%m-%d %H:%M", localtime(&now));
			return 2;
		}
		if (win.current == 7) return 0;
	}
}
