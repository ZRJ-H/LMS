#ifndef CONTROL_H
#define CONTROL_H

#include <graphics.h>

/* ---- 控件类型 ---- */
#define BUTTON    1
#define EDIT      2
#define EDIT_PWD  3
#define LABEL     4
#define COMBO     5

/* ---- 方向键（保留兼容，推荐用 VK_*）---- */
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_LEFT  75
#define KEY_RIGHT 77

/* ---- 表行绘制回调 ---- */
typedef void (*TableDrawFn)(const void *record, int row_idx, int y_base,
                             int table_x, const int *col_widths, int ncols);

/* ---- 控件结构体 ---- */
typedef struct control_t {
	int x;
	int y;
	int width;
	int height;
	char text[100];
	COLORREF bgColor1;   /* 选中态背景色 */
	COLORREF bgColor2;   /* 未选中态背景色 */
	COLORREF textColor;  /* 选中态文字色 */
	int type;
	int state;           /* 0=未选中, 1=选中 */
	int visible;         /* EDIT_PWD: 0=掩码, 1=明文 */
	int sel_index;       /* COMBO 选中项索引 */
	COLORREF textColor2; /* 未选中态文字色（0=默认用 textColor） */
} CONTROL_T;

/* ---- 窗口结构体 ---- */
typedef struct window_t {
	int x, y;
	int width, height;
	COLORREF bgColor;
	int count;
	CONTROL_T controls[32];
	int current;         /* 当前焦点控件索引 */
} WINDOW_T;

/* ---- 函数声明 ---- */
void      set_bg_image(void *img);
void      redraw_bg();
void      drawWhiteCard();                      /* 绘制白卡片隔离层 */
void      window_set_card(int on);              /* 开启/关闭卡片模式 */
void      window_set_frame(int x, int y, int w, int h);  /* 设置页面边框（浅蓝矩形） */
void      window_clear_frame();
void      ui_draw_panel();
void      ui_draw_title(const char *title);
void      ui_draw_meta(const char *left, const char *right);
void      control_show(CONTROL_T ctrl);
WINDOW_T  window_show(WINDOW_T win);
WINDOW_T  window_run(WINDOW_T win);
int       window_show_table(const char *title,
                            const char **headers,
                            const int *col_widths,
                            int ncols,
                            const void *head,
                            size_t next_offset,
                            TableDrawFn draw_row,
                            int page_size);

#endif /* CONTROL_H */
