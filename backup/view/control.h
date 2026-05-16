#ifndef CONTROL_H
#define CONTROL_H
#define BUTTON    1
#define EDIT      2
#define EDIT_PWD  3
#define LABEL     4
//定义方向键
#define KEY_UP		72
#define KEY_DOWN	80
#define KEY_LEFT	75
#define KEY_RIGHT	77
//定义控件结构体 

typedef struct control_t{
	int x;
	int y;
	int width;
	int height;
	char text[100];
	COLORREF bgColor1;
	COLORREF bgColor2;
	COLORREF textColor;
	int type;
	int state;
	int visible;   /* EDIT_PWD 是否明文显示，0=掩码 */
}CONTROL_T;

typedef struct window_t{
	int x;
	int y;
	int width;
	int height;
	COLORREF bgColor;//窗口填充颜色
	int count;//控件数
	CONTROL_T controls[10];
	int current;//当前停留在哪个控件 
}WINDOW_T;
//控件显示函数 
void control_show(CONTROL_T ctrl);
//窗口显示 
WINDOW_T window_show(WINDOW_T win);
//窗口运行驱动
WINDOW_T window_run(WINDOW_T win); 
#endif
