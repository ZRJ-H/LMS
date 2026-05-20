#ifndef CONTROL_H
#define CONTROL_H
#define BUTTON    1
#define EDIT      2
#define EDIT_PWD  3
#define LABEL     4
#define COMBO     5
/* 表行绘制回调：table_x=表格起始X, col_widths=列宽数组 */
typedef void (*TableDrawFn)(const void *record, int row_idx, int y_base, int table_x, const int *col_widths, int ncols);
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
	int sel_index; /* COMBO 当前选中选项索引（0-based） */
}CONTROL_T;

typedef struct window_t{
	int x;
	int y;
	int width;
	int height;
	COLORREF bgColor;//窗口填充颜色
	int count;//控件数
	CONTROL_T controls[15];
	int current;//当前停留在哪个控件
}WINDOW_T;

/* 分页表格窗口：head=链表头, next_offset=next字段偏移, page_size=每页行数 */
int window_show_table(const char *title,
                       const char **headers,
                       const int *col_widths,
                       int ncols,
                       const void *head,
                       size_t next_offset,
                       TableDrawFn draw_row,
                       int page_size);
//控件显示函数
void control_show(CONTROL_T ctrl);
//设置全局背景图（在 window_show 中每次重绘）
void set_bg_image(void *img);
//窗口显示
WINDOW_T window_show(WINDOW_T win);
//窗口运行驱动
WINDOW_T window_run(WINDOW_T win);
#endif
