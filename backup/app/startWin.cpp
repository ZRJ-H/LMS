#include <graphics.h>
#include <stdio.h>
#include "startWin.h"
#include "../public/ui_config.h"

int startWin() {
	/* 全屏窗口，容纳 3 个竖排按钮 */
	WINDOW_T win = {
		280, 220, 240, 250, WHITE_COLOR, 3, {
			{320, 250, BTN_W, BTN_H, "登  录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{320, 320, BTN_W, BTN_H, "忘记密码",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{320, 390, BTN_W, BTN_H, "退  出",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	window_show(win);

	/* 大标题 — 在窗口之上单独绘制 */
	settextstyle(FONT_TITLE_H, 0, _T("黑体"));
	settextcolor(TEXT_MAIN);
	const char *title = "智能物流管理系统";
	int tx = (WIN_W - textwidth(title)) / 2;
	outtextxy(tx, LOGIN_TITLE_Y, title);

	window_run(win);

	if (win.current == 0)      return 1;   /* 登录 → loginWin */
	else if (win.current == 1) return 3;   /* 忘记密码 → forgotPasswordWin */
	else                       return -1;  /* 退出 */
}
