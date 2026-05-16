#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include "startWin.h"

int startWin(){
	WINDOW_T startWin = {
	    290, 240, 220, 180, WHITE, 3, {
	        {300, 250, 200, 50, "1-登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
	        {300, 305, 200, 50, "2-仓库管理", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	        {300, 365, 200, 50, "3-退出", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
	    }
	};

	window_show(startWin);
	window_run(startWin);

	if (startWin.current == 0)      return 1;   /* 登录 → loginWin */
	else if (startWin.current == 1) return 3;   /* 仓库管理（Day5实现） */
	else if (startWin.current == 2) return -1;  /* 退出 */
	return 0;
}
