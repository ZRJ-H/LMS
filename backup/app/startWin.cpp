#include <graphics.h>
#include <stdio.h>
#include <time.h>
#include "startWin.h"
#include "../public/ui_config.h"

int startWin() {
	/* 图1 欢迎界面 — 布局 B（按钮选择页） */
	window_set_card(0);
	const int frame_x = 100;
	const int frame_y = 50;
	const int frame_w = 600;
	const int frame_h = 450;
	const int btn_x = frame_x + 80;
	const int btn_w = 120;
	const int btn_h = 30;
	window_set_frame(frame_x, frame_y, frame_w, frame_h);

	WINDOW_T win = {
		frame_x, frame_y, frame_w, frame_h, WHITE_COLOR, 3, {
			/* 0: 登录 */
			{btn_x, 255, btn_w, btn_h, "登  录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			/* 1: 忘记密码 */
			{btn_x, 295, btn_w, btn_h, "忘记密码",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			/* 2: 退出系统 */
			{btn_x, 335, btn_w, btn_h, "退出系统",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);

		/* 页面大标题 — 窗口顶部居中 */
		settextstyle(22, 0, _T("黑体"));
		settextcolor(TEXT_MAIN);
		const char *title = "智能物流管理系统";
		int tx = frame_x + (frame_w - textwidth(title)) / 2;
		outtextxy(tx, 180, title);

		/* 日期 — 框内居中 */
		settextstyle(16, 0, _T("黑体"));
		settextcolor(TEXT_MUTED);
		time_t now = time(NULL);
		char date_str[32];
		strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&now));
		int dx = frame_x + (frame_w - textwidth(date_str)) / 2;
		outtextxy(dx, 220, date_str);

		/* 说明文字 */
		settextcolor(TEXT_MUTED);
		const char *hint = "请选择操作";
		int hx = frame_x + (frame_w - textwidth(hint)) / 2;
		outtextxy(hx, 380, hint);

		win = window_run(win);

		if (win.current == 0)      return 1;   /* 登录 → loginWin */
		else if (win.current == 1) return 3;   /* 忘记密码 → forgotPasswordWin */
		else                       return -1;  /* 退出 */
	}
}
