#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include "public/common.h"
#include "view/control.h"
#include "startWin.h"
#include "loginWin.h"
#include "app/mainWin.h"
#include "service/user_service.h"
#include "service/order_service.h"

int main(int argc, char** argv) {
	initgraph(800, 600, 1);
	setbkmode(TRANSPARENT);
	SetForegroundWindow(GetHWnd());	/* 强制图形窗口获得焦点，不然键盘输入无效 */

	/* 背景图片（BMP 格式，旧版 EasyX 不支持 PNG） */
	IMAGE img1;
	loadimage(&img1, "image/bg.bmp");
	putimage(0, 0, &img1);

	set_bg_image(&img1);
	init_order_sequence();
	user_svc_init();   /* 从文件加载用户 */
	order_svc_init();  /* 从文件加载订单 */

	/* 函数指针跳转表：索引即窗口ID */
	int (*func[10])() = {startWin, loginWin, mainWin, forgotPasswordWin};
	int win_id = 0;  /* 从 startWin 开始 */
	while (1) {
		win_id = func[win_id]();
		if (win_id < 0) break;          /* 退出系统 */
		if (win_id >= 10) win_id = 0;   /* 安全保障 */
	}

	closegraph();
	return 0;
}
