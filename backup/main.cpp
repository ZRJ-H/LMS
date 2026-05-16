#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include "common.h"
#include "control.h"
#include "startWin.h"
#include "loginWin.h"
#include "mainWin.h"

int main(int argc, char** argv) {
	initgraph(800, 600, 1);
	setbkmode(TRANSPARENT);
	SetForegroundWindow(GetHWnd());	/* 强制图形窗口获得焦点，不然键盘输入无效 */

	/* 显示背景图片 */
	IMAGE img1;
	loadimage(&img1, "image/bg.png", 800, 600);
	putimage(0, 0, &img1);

	/* 启动时初始化示例用户 */
	init_sample_users();

	/* 函数指针跳转表：索引即窗口ID */
	int (*func[10])() = {startWin, loginWin, mainWin};
	int win_id = 0;
	while (1) {
		win_id = func[win_id]();
		if (win_id < 0) break;          /* 退出系统 */
		if (win_id >= 10) win_id = 0;   /* 安全保障 */
	}

	closegraph();
	return 0;
}
