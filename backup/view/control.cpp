#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include "control.h"

/* 全局背景图指针，由 main() 设置 */
static IMAGE *g_bg = NULL;

void set_bg_image(void *img) { g_bg = (IMAGE *)img; }

/* 控件显示函数 */
void control_show(CONTROL_T ctrl){
	int center=0;
	char str[20]={0};
	int i=0;
	COLORREF fill = (ctrl.state == 1) ? ctrl.bgColor1 : ctrl.bgColor2;
	COLORREF txt  = (ctrl.state == 1) ? ctrl.textColor :
	                (ctrl.textColor2 ? ctrl.textColor2 : ctrl.textColor);

	setfillcolor(fill);
	settextcolor(txt);
	settextstyle(16, 10, _T("黑体"));

	/* 有边框的控件类型：先画黑边框，再内缩填充 */
	if (ctrl.type == BUTTON || ctrl.type == EDIT ||
	    ctrl.type == EDIT_PWD || ctrl.type == COMBO) {
		setlinecolor(BLACK);
		rectangle(ctrl.x, ctrl.y, ctrl.x + ctrl.width, ctrl.y + ctrl.height);
		fillrectangle(ctrl.x + 1, ctrl.y + 1,
		              ctrl.x + ctrl.width - 1, ctrl.y + ctrl.height - 1);
	}
	if (ctrl.type == COMBO) {
		char display[100] = {0}, tmp[100] = {0};
		strcpy(tmp, ctrl.text);
		int seg = 0;
		char *token = strtok(tmp, "|");
		while (token) {
			if (seg == ctrl.sel_index) { strcpy(display, token); break; }
			token = strtok(NULL, "|");
			seg++;
		}
		outtextxy(ctrl.x + 5, ctrl.y + 15, display);
		outtextxy(ctrl.x + ctrl.width - 20, ctrl.y + 13, "▼");
	}
	else if (ctrl.type == EDIT_PWD) {
		if (ctrl.visible) {
			outtextxy(ctrl.x + 5, ctrl.y + 15, ctrl.text);
		} else {
			for (i = 0; i < (int)strlen(ctrl.text); i++) str[i] = '*';
			outtextxy(ctrl.x + 5, ctrl.y + 15, str);
		}
	}
	else if (ctrl.type == EDIT || ctrl.type == LABEL) {
		outtextxy(ctrl.x + 5, ctrl.y + 15, ctrl.text);
	}
	else if (ctrl.type == BUTTON) {
		center = ctrl.x + (ctrl.width - (int)strlen(ctrl.text) * 10) / 2;
		outtextxy(center, ctrl.y + 15, ctrl.text);
	}
}

/* 窗口显示 */
WINDOW_T window_show(WINDOW_T win){
	int i=0;
	cleardevice();
		if (g_bg) putimage(0, 0, g_bg);
	setfillcolor(win.bgColor);
	fillrectangle(win.x,win.y,win.x+win.width,win.y+win.height);
	for(i=0;i<win.count;i++){
		control_show(win.controls[i]);//
	}
	return win;
}

/* 窗口运行驱动 — 使用 EasyX getmessage() 读取图形窗口键盘事件 */
WINDOW_T window_run(WINDOW_T win){
	ExMessage msg;

	/* 从 win.current 开始，跳过 LABEL */
	int i = win.current;
	while (win.controls[i].type == LABEL) {
		i++;
		if (i >= win.count) i = 0;
	}

	while(1){
		msg = getmessage(EX_KEY | EX_CHAR);

		if(msg.message == WM_KEYDOWN){
			if(msg.vkcode == VK_RETURN){			/* 回车 */
				if(win.controls[i].type == BUTTON){
					win.current = i;
					return win;
				}
				if(win.controls[i].type == COMBO){
					/* 下拉框选择模式 */
					char opts[10][100];
					int opt_cnt=0;
					char tmp[100]={0};
					strcpy(tmp,win.controls[i].text);
					char *token=strtok(tmp,"|");
					while(token&&opt_cnt<10){
						strcpy(opts[opt_cnt++],token);
						token=strtok(NULL,"|");
					}
					int sel=win.controls[i].sel_index;
					int drop_open=1;
					int drop_x=win.controls[i].x;
					int drop_y=win.controls[i].y+win.controls[i].height;
					int drop_w=win.controls[i].width;
					int drop_h=opt_cnt*28;
					/* 如果下方不够空间则向上弹出 */
					if(drop_y+drop_h>600){drop_y=win.controls[i].y-drop_h;}

					while(drop_open){
						/* 绘制下拉列表 */
						setfillcolor(WHITE);
						fillrectangle(drop_x,drop_y,drop_x+drop_w,drop_y+drop_h);
						settextstyle(14,8,_T("黑体"));
						for(int k=0;k<opt_cnt;k++){
							if(k==sel){
								setfillcolor(CYAN);
								fillrectangle(drop_x+2,drop_y+2+k*28,drop_x+drop_w-2,drop_y+k*28+28);
								settextcolor(WHITE);
							}else{
								setfillcolor(WHITE);
								settextcolor(BLACK);
							}
							outtextxy(drop_x+5,drop_y+8+k*28,opts[k]);
						}
						/* 边框 */
						setlinecolor(BLACK);
						rectangle(drop_x,drop_y,drop_x+drop_w,drop_y+drop_h);

						ExMessage dm=getmessage(EX_KEY);
						if(dm.message==WM_KEYDOWN){
							if(dm.vkcode==VK_UP&&sel>0) sel--;
							if(dm.vkcode==VK_DOWN&&sel<opt_cnt-1) sel++;
							if(dm.vkcode==VK_RETURN){
								win.controls[i].sel_index=sel;
								drop_open=0;
							}
							if(dm.vkcode==VK_ESCAPE) drop_open=0;
						}
					}
					/* 重绘整个窗口覆盖下拉痕迹 */
					window_show(win);
				}
			}
			else if(msg.vkcode == VK_TAB){			/* Tab 切换密码明文/密文 */
				if(win.controls[i].type == EDIT_PWD){
					win.controls[i].visible = !win.controls[i].visible;
					control_show(win.controls[i]);
				}
			}
			else if(msg.vkcode == VK_BACK){			/* 退格 */
				if(win.controls[i].type == EDIT || win.controls[i].type == EDIT_PWD){
					int len = (int)strlen(win.controls[i].text);
					if(len > 0){
						win.controls[i].text[len - 1] = '\0';
						control_show(win.controls[i]);
					}
				}
			}
			else if(msg.vkcode == VK_UP){			/* 上键 */
				win.controls[i].state=0;
				control_show(win.controls[i]);
				do{
					i--;
					if(i==-1) i=win.count-1;
				} while(win.controls[i].type==LABEL);
				win.controls[i].state=1;
				control_show(win.controls[i]);
			}
			else if(msg.vkcode == VK_DOWN){			/* 下键 */
				win.controls[i].state=0;
				control_show(win.controls[i]);
				do{
					i++;
					if(i==win.count) i=0;
				} while(win.controls[i].type==LABEL);
				win.controls[i].state=1;
				control_show(win.controls[i]);
			}
		}
		else if(msg.message == WM_CHAR){			/* 字符输入 */
			char ch = (char)msg.ch;
			if((ch>='0'&&ch<='9') || (ch>='a'&&ch<='z') || (ch>='A'&&ch<='Z')){
				if(win.controls[i].type == EDIT || win.controls[i].type == EDIT_PWD){
					int len = (int)strlen(win.controls[i].text);
					win.controls[i].text[len] = ch;
					control_show(win.controls[i]);
				}
			}
		}
	}
}

/* ============================================================
 *      分页表格窗口
 *      title:       标题文本
 *      headers:     列标题数组
 *      col_widths:  每列宽度（像素）
 *      ncols:       列数
 *      head:        链表头指针
 *      next_offset: next 字段偏移量
 *      draw_row:    行绘制回调（参数：record, row_idx, y_base）
 *      page_size:   每页行数
 *      返回：选中行号（1-based），Esc 返回 0
 * ============================================================ */
int window_show_table(const char *title,
                       const char **headers,
                       const int *col_widths,
                       int ncols,
                       const void *head,
                       size_t next_offset,
                       TableDrawFn draw_row,
                       int page_size) {
	int total = 0;
	const char *p = (const char *)head;
	while (p) { total++; p = *(const char **)(p + next_offset); }

	if (total == 0) {
		settextstyle(18, 12, _T("黑体"));
		outtextxy(100, 280, "暂无数据，按任意键返回...");
		getmessage(EX_KEY);
		return 0;
	}

	/* 收集所有记录指针 */
	const void **records = (const void **)malloc(total * sizeof(void *));
	if (!records) return 0;
	p = (const char *)head;
	for (int idx = 0; idx < total; idx++) {
		records[idx] = p;
		p = *(const char **)(p + next_offset);
	}

	/* 计算总宽度 */
	int total_w = 40;
	for (int c = 0; c < ncols; c++) total_w += col_widths[c];
	int table_x = (800 - total_w) / 2;
	int table_y = 60;
	int row_h = 26;
	int header_h = 30;

	int pages = (total + page_size - 1) / page_size;
	int cur_page = 0;
	int cur_row = 0;  /* 当前页内高亮行（0-based） */

	while (1) {
		cleardevice();

		/* 标题 */
		settextstyle(20, 12, _T("黑体"));
		char title_buf[256];
		sprintf(title_buf, "%s  第 %d/%d 页  共 %d 条", title, cur_page + 1, pages, total);
		outtextxy(table_x, 20, title_buf);

		/* 表头 */
		setfillcolor(CYAN);
		int x = table_x;
		for (int c = 0; c < ncols; c++) {
			fillrectangle(x, table_y, x + col_widths[c], table_y + header_h);
			rectangle(x, table_y, x + col_widths[c], table_y + header_h);
			settextcolor(WHITE);
			settextstyle(14, 8, _T("黑体"));
			outtextxy(x + 5, table_y + 8, (char *)headers[c]);
			x += col_widths[c];
		}

		/* 数据行 */
		int start = cur_page * page_size;
		int end = (start + page_size < total) ? start + page_size : total;
		for (int r = start; r < end; r++) {
			int local_r = r - start;
			int ry = table_y + header_h + local_r * row_h;

			/* 整行高亮 */
			if (local_r == cur_row) {
				setfillcolor(LIGHTCYAN);
				fillrectangle(table_x, ry, table_x + total_w - 40, ry + row_h);
			}

			draw_row(records[r], r, ry, table_x, col_widths, ncols);
		}

		/* 底部提示 */
		settextstyle(14, 8, _T("黑体"));
		settextcolor(BLACK);
		int bottom_y = table_y + header_h + page_size * row_h + 20;
		outtextxy(table_x, bottom_y, "↑↓:选择行  ←→:翻页  Enter:确认  Esc:返回");

		/* 键盘交互 */
		ExMessage tm = getmessage(EX_KEY);
		if (tm.message == WM_KEYDOWN) {
			if (tm.vkcode == VK_ESCAPE) {
				free(records);
				return 0;
			}
			if (tm.vkcode == VK_UP && cur_row > 0) {
				cur_row--;
			}
			if (tm.vkcode == VK_DOWN && cur_row < (end - start) - 1) {
				cur_row++;
			}
			if (tm.vkcode == VK_LEFT && cur_page > 0) {
				cur_page--;
				cur_row = 0;
			}
			if (tm.vkcode == VK_RIGHT && cur_page < pages - 1) {
				cur_page++;
				cur_row = 0;
			}
			if (tm.vkcode == VK_RETURN) {
				int selected = cur_page * page_size + cur_row + 1;  /* 1-based */
				free(records);
				return selected;
			}
		}
	}
}
