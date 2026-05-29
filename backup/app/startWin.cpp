#include <graphics.h>
#include <stdio.h>
#include <time.h>
#include "startWin.h"
#include "../public/ui_config.h"

int startWin() {
	const int panel_w = 320;
	const int panel_h = 250;
	const int panel_x = (800 - panel_w) / 2;
	const int panel_y = 160;
	const int btn_w = 130;
	const int btn_h = 32;
	const int btn_x = panel_x + (panel_w - btn_w) / 2;
	const int btn_y = panel_y + 118;

	window_clear_frame();
	window_set_card(0);
	window_set_frame(panel_x, panel_y, panel_w, panel_h);

	WINDOW_T win = {
		panel_x, panel_y, panel_w, panel_h, WHITE_COLOR, 3, {
			{btn_x, btn_y, btn_w, btn_h, "登  录",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 1, 0, 0, TEXT_MAIN},
			{btn_x, btn_y + 38, btn_w, btn_h, "忘记密码",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
			{btn_x, btn_y + 76, btn_w, btn_h, "退出系统",
			 PRIMARY, WHITE_COLOR, WHITE_COLOR, BUTTON, 0, 0, 0, TEXT_MAIN},
		}
	};

	while (1) {
		window_show(win);
		settextstyle(20, 0, _T("黑体"));
		settextcolor(TEXT_MAIN);
		const char *title = "智能物流管理系统";
		outtextxy(panel_x + (panel_w - textwidth(title)) / 2, panel_y + 42, title);

		settextstyle(16, 0, _T("黑体"));
		settextcolor(TEXT_MUTED);
		time_t now = time(NULL);
		char date_str[32];
		strftime(date_str, sizeof(date_str), "%Y-%m-%d", localtime(&now));
		int dx = panel_x + (panel_w - textwidth(date_str)) / 2;
		outtextxy(dx, panel_y + 78, date_str);

		const char *hint = "请选择操作";
		int hx = panel_x + (panel_w - textwidth(hint)) / 2;
		outtextxy(hx, panel_y + 225, hint);

		win = window_run(win);
		if (win.current == 0) return 1;
		if (win.current == 1) return 3;
		return -1;
	}
}
