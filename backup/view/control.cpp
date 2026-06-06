#include <graphics.h>
#include <stdio.h>
#include <string.h>
#include "control.h"
#include "../public/common.h"
#include "../public/ui_config.h"

/* ---- 全局背景图 ---- */
static IMAGE *g_bg = NULL;

void set_bg_image(void *img) { g_bg = (IMAGE *)img; }
void redraw_bg() { if (g_bg) putimage(0, 0, g_bg); }

/* ---- 白卡片模式 ---- */
static int g_use_card = 1;  /* 默认子界面使用卡片 */
static int g_frame_x = 0, g_frame_y = 0, g_frame_w = 0, g_frame_h = 0;

void window_set_card(int on) { g_use_card = on; }

void window_set_frame(int x, int y, int w, int h) {
	g_frame_x = x; g_frame_y = y; g_frame_w = w; g_frame_h = h;
}

void window_clear_frame() {
	g_frame_x = 0; g_frame_y = 0; g_frame_w = 0; g_frame_h = 0;
}

void ui_draw_panel() {
	setfillcolor(WHITE_COLOR);
	fillrectangle(UI_PANEL_X, UI_PANEL_Y,
	              UI_PANEL_X + UI_PANEL_W, UI_PANEL_Y + UI_PANEL_H);
	setlinecolor(FRAME_BLUE);
	rectangle(UI_PANEL_X, UI_PANEL_Y,
	          UI_PANEL_X + UI_PANEL_W, UI_PANEL_Y + UI_PANEL_H);
}

void ui_draw_title(const char *title) {
	settextstyle(FONT_HEADER_H + 4, 0, _T("黑体"));
	settextcolor(TEXT_MAIN);
	int tx = UI_PANEL_X + (UI_PANEL_W - textwidth(title)) / 2;
	outtextxy(tx, UI_TITLE_Y, title);
}

void ui_draw_meta(const char *left, const char *right) {
	settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
	settextcolor(TEXT_MAIN);
	if (left) outtextxy(UI_PANEL_X + 35, UI_META_Y, left);
	if (right) {
		int rx = UI_PANEL_X + UI_PANEL_W - 35 - textwidth(right);
		outtextxy(rx, UI_META_Y, right);
	}
}

/* 手动实现圆角矩形填充 — 兼容旧版 EasyX（无 fillroundrect） */
void drawWhiteCard() {
	int x1 = CARD_X1, y1 = CARD_Y1, x2 = CARD_X2, y2 = CARD_Y2, r = CARD_R;
	setfillcolor(CARD_BG);
	/* 中央矩形 */
	fillrectangle(x1 + r, y1,      x2 - r, y2);
	fillrectangle(x1,     y1 + r,  x2,     y2 - r);
	/* 四个圆角：fillellipse(left, top, right, bottom) */
	fillellipse(x1,         y1,         x1 + 2 * r, y1 + 2 * r);  /* 左上 */
	fillellipse(x2 - 2 * r, y1,         x2,         y1 + 2 * r);  /* 右上 */
	fillellipse(x1,         y2 - 2 * r, x1 + 2 * r, y2);          /* 左下 */
	fillellipse(x2 - 2 * r, y2 - 2 * r, x2,         y2);          /* 右下 */
}

/* ---- 通用文字居中公式 ---- */
static int text_center_x(int x1, int x2, const char *text) {
	return x1 + ((x2 - x1) - textwidth(text)) / 2;
}
static int text_center_y(int y1, int y2, const char *text) {
	return y1 + ((y2 - y1) - textheight(text)) / 2;
}

/* ============================================================
 *  控件显示 — 输入框规范：激活白底+PRIMARY边框，未激活浅灰底+灰边框
 * ============================================================ */
void control_show(CONTROL_T ctrl) {
	if (ctrl.type == LABEL && ctrl.x == UI_PANEL_X &&
	    ctrl.width >= 300 && ctrl.y < UI_PANEL_Y + 55) {
		ui_draw_title(ctrl.text);
		return;
	}

	COLORREF fill = (ctrl.state == 1) ? ctrl.bgColor1 : ctrl.bgColor2;
	COLORREF txt  = (ctrl.state == 1) ? ctrl.textColor :
	                (ctrl.textColor2 ? ctrl.textColor2 : ctrl.textColor);

	setfillcolor(fill);
	settextcolor(txt);
	settextstyle(FONT_BTN_H, FONT_BTN_W, _T("黑体"));

	/* 边框控件：EDIT/EDIT_PWD 焦点态用 PRIMARY 边框，非焦点用灰色 */
	if (ctrl.type == BUTTON || ctrl.type == EDIT ||
	    ctrl.type == EDIT_PWD || ctrl.type == COMBO) {
		if ((ctrl.type == EDIT || ctrl.type == EDIT_PWD) && ctrl.state == 1)
			setlinecolor(PRIMARY);
		else if (ctrl.type == EDIT || ctrl.type == EDIT_PWD)
			setlinecolor(INPUT_BORDER);
		else
			setlinecolor(BLACK_COLOR);

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
		int ty = text_center_y(ctrl.y, ctrl.y + ctrl.height, display);
		outtextxy(ctrl.x + CTRL_PADDING, ty, display);
		outtextxy(ctrl.x + ctrl.width - 20, ty, "▼");
	}
	else if (ctrl.type == EDIT_PWD) {
		int pw_right = ctrl.x + ctrl.width - ctrl.height;
		if (ctrl.visible) {
			int ty = text_center_y(ctrl.y, ctrl.y + ctrl.height, ctrl.text);
			outtextxy(ctrl.x + CTRL_PADDING, ty, ctrl.text);
		} else {
			int len = (int)strlen(ctrl.text);
			char str[64] = {0};
			for (int i = 0; i < len && i < 63; i++) str[i] = '*';
			int ty = text_center_y(ctrl.y, ctrl.y + ctrl.height, str);
			outtextxy(ctrl.x + CTRL_PADDING, ty, str);
		}
		/* 右侧显隐切换方块 */
		int sq_x = pw_right, sq_y = ctrl.y, sq_sz = ctrl.height;
		setlinecolor(BLACK_COLOR);
		setfillcolor(ctrl.visible ? LIGHT_CYAN : WHITE_COLOR);
		fillrectangle(sq_x + 1, sq_y + 1, sq_x + sq_sz - 1, sq_y + sq_sz - 1);
		rectangle(sq_x, sq_y, sq_x + sq_sz, sq_y + sq_sz);
		settextstyle(12, 6, _T("黑体"));
		settextcolor(TEXT_MUTED);
		int sq_cx = text_center_x(sq_x, sq_x + sq_sz, ctrl.visible ? "明" : "*");
		int sq_cy = text_center_y(sq_y, sq_y + sq_sz, "明");
		outtextxy(sq_cx, sq_cy, ctrl.visible ? "明" : "*");
	}
	else if (ctrl.type == EDIT || ctrl.type == LABEL) {
		int ty = text_center_y(ctrl.y, ctrl.y + ctrl.height, ctrl.text);
		outtextxy(ctrl.x + CTRL_PADDING, ty, ctrl.text);
	}
	else if (ctrl.type == BUTTON) {
		int cx = text_center_x(ctrl.x, ctrl.x + ctrl.width, ctrl.text);
		int cy = text_center_y(ctrl.y, ctrl.y + ctrl.height, ctrl.text);
		outtextxy(cx, cy, ctrl.text);
	}
}

/* ============================================================
 *  窗口显示 — g_use_card=1 → 白卡片；g_use_card=0 → 窗口填充
 * ============================================================ */
WINDOW_T window_show(WINDOW_T win) {
	cleardevice();
	redraw_bg();

	if (g_frame_w > 0 && (win.x != g_frame_x || win.y != g_frame_y ||
	    win.width != g_frame_w || win.height != g_frame_h)) {
		window_clear_frame();
	}

	if (g_frame_w > 0 && g_frame_h > 0) {
		/* 白底浅蓝边框 — PDF 原始风格 */
		setfillcolor(WHITE_COLOR);
		fillrectangle(g_frame_x, g_frame_y, g_frame_x + g_frame_w, g_frame_y + g_frame_h);
		setlinecolor(FRAME_BLUE);
		rectangle(g_frame_x, g_frame_y, g_frame_x + g_frame_w, g_frame_y + g_frame_h);
	} else if (g_use_card) {
		ui_draw_panel();
	} else {
		ui_draw_panel();
	}

	for (int i = 0; i < win.count; i++) {
		control_show(win.controls[i]);
	}
	return win;
}

/* ============================================================
 *  窗口运行驱动 — 键盘 + 鼠标事件
 * ============================================================ */
WINDOW_T window_run(WINDOW_T win) {
	int i = win.current;
	while (win.controls[i].type == LABEL) {
		i++;
		if (i >= win.count) i = 0;
	}

	while (1) {
		ExMessage msg = getmessage(EX_KEY | EX_CHAR | EX_MOUSE);

		/* ---- 鼠标事件 ---- */
		if (msg.message == WM_LBUTTONDOWN) {
			int mx = msg.x, my = msg.y;

			/* 密码显隐方块 */
			for (int c = 0; c < win.count; c++) {
				if (win.controls[c].type == EDIT_PWD) {
					CONTROL_T *pwd = &win.controls[c];
					int sq_sz = pwd->height;
					int sq_x = pwd->x + pwd->width - sq_sz;
					int sq_y = pwd->y;
					if (mx >= sq_x && mx <= sq_x + sq_sz &&
					    my >= sq_y && my <= sq_y + sq_sz) {
						pwd->visible = !pwd->visible;
						window_show(win);
						break;
					}
				}
			}

			/* 按钮点击 → 返回 */
			for (int c = 0; c < win.count; c++) {
				if (win.controls[c].type == BUTTON) {
					if (mx >= win.controls[c].x && mx <= win.controls[c].x + win.controls[c].width &&
					    my >= win.controls[c].y && my <= win.controls[c].y + win.controls[c].height) {
						win.current = c;
						return win;
					}
				}
				/* EDIT/EDIT_PWD/COMBO 点击 → 切换焦点 */
				if (win.controls[c].type == EDIT || win.controls[c].type == EDIT_PWD ||
				    win.controls[c].type == COMBO) {
					if (mx >= win.controls[c].x && mx <= win.controls[c].x + win.controls[c].width &&
					    my >= win.controls[c].y && my <= win.controls[c].y + win.controls[c].height) {
						win.controls[i].state = 0;
						control_show(win.controls[i]);
						i = c;
						win.controls[i].state = 1;
						control_show(win.controls[i]);
						input_reset_pending();
					}
				}
			}
		}

		/* ---- 键盘事件 ---- */
		if (msg.message == WM_KEYDOWN) {
			if (msg.vkcode == VK_ESCAPE) {
				win.current = -1;
				return win;
			}
			if (msg.vkcode == VK_RETURN) {
				if (win.controls[i].type == BUTTON) {
					win.current = i;
					return win;
				}
				if (win.controls[i].type == COMBO) {
					char opts[10][100];
					int opt_cnt = 0;
					char tmp[100] = {0};
					strcpy(tmp, win.controls[i].text);
					char *token = strtok(tmp, "|");
					while (token && opt_cnt < 10) {
						strcpy(opts[opt_cnt++], token);
						token = strtok(NULL, "|");
					}
					int sel = win.controls[i].sel_index;
					int drop_open = 1;
					int drop_x = win.controls[i].x;
					int drop_y = win.controls[i].y + win.controls[i].height;
					int drop_w = win.controls[i].width;
					int drop_h = opt_cnt * 28;
					if (drop_y + drop_h > WIN_H) drop_y = win.controls[i].y - drop_h;

					while (drop_open) {
						setfillcolor(WHITE_COLOR);
						fillrectangle(drop_x, drop_y, drop_x + drop_w, drop_y + drop_h);
						settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));
						for (int k = 0; k < opt_cnt; k++) {
							if (k == sel) {
								setfillcolor(PRIMARY);
								fillrectangle(drop_x + 2, drop_y + 2 + k * 28,
								              drop_x + drop_w - 2, drop_y + k * 28 + 28);
								settextcolor(WHITE_COLOR);
							} else {
								setfillcolor(WHITE_COLOR);
								settextcolor(TEXT_MAIN);
							}
							outtextxy(drop_x + 5, drop_y + 8 + k * 28, opts[k]);
						}
						setlinecolor(BLACK_COLOR);
						rectangle(drop_x, drop_y, drop_x + drop_w, drop_y + drop_h);

						ExMessage dm = getmessage(EX_KEY);
						if (dm.message == WM_KEYDOWN) {
							if (dm.vkcode == VK_UP && sel > 0) sel--;
							if (dm.vkcode == VK_DOWN && sel < opt_cnt - 1) sel++;
							if (dm.vkcode == VK_RETURN) {
								win.controls[i].sel_index = sel;
								drop_open = 0;
							}
							if (dm.vkcode == VK_ESCAPE) drop_open = 0;
						}
					}
					window_show(win);
				}
			}
			else if (msg.vkcode == VK_TAB) {
				if (win.controls[i].type == EDIT_PWD) {
					win.controls[i].visible = !win.controls[i].visible;
					control_show(win.controls[i]);
				}
			}
			else if (msg.vkcode == VK_BACK) {
				if (win.controls[i].type == EDIT || win.controls[i].type == EDIT_PWD) {
					input_delete_last_char(win.controls[i].text);
					control_show(win.controls[i]);
				}
			}
			else if (msg.vkcode == VK_UP) {
				win.controls[i].state = 0;
				control_show(win.controls[i]);
				input_reset_pending();
				do {
					i--;
					if (i == -1) i = win.count - 1;
				} while (win.controls[i].type == LABEL);
				win.controls[i].state = 1;
				control_show(win.controls[i]);
			}
			else if (msg.vkcode == VK_DOWN) {
				win.controls[i].state = 0;
				control_show(win.controls[i]);
				input_reset_pending();
				do {
					i++;
					if (i == win.count) i = 0;
				} while (win.controls[i].type == LABEL);
				win.controls[i].state = 1;
				control_show(win.controls[i]);
			}
		}
		/* 字符输入 */
		else if (msg.message == WM_CHAR) {
			if (win.controls[i].type == EDIT || win.controls[i].type == EDIT_PWD) {
				if (input_append_char(win.controls[i].text, 99, msg.ch,
				    win.controls[i].input_filter)) {
					control_show(win.controls[i]);
				}
			}
		}
	}
}

/* ============================================================
 *  分页表格 — 完整二维网格 + 40px 表头 + 列间竖线 + 行间横线
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
		cleardevice();
		redraw_bg();
		ui_draw_panel();
		settextstyle(FONT_HEADER_H, FONT_HEADER_W, _T("黑体"));
		settextcolor(TEXT_MUTED);
		int cx = text_center_x(0, WIN_W, "暂无数据，按任意键返回...");
		outtextxy(cx, 280, "暂无数据，按任意键返回...");
		getmessage(EX_KEY);
		return 0;
	}

	const void **records = (const void **)malloc(total * sizeof(void *));
	if (!records) return 0;
	p = (const char *)head;
	for (int idx = 0; idx < total; idx++) {
		records[idx] = p;
		p = *(const char **)(p + next_offset);
	}

	/* 表格几何 — 内容在卡片内 */
	int local_widths[10];
	int total_w = 0;
	for (int c = 0; c < ncols; c++) {
		local_widths[c] = col_widths[c];
		total_w += local_widths[c];
	}
	if (total_w > UI_PANEL_W - 36 && ncols > 0) {
		int target_w = UI_PANEL_W - 36;
		int scaled_w = 0;
		for (int c = 0; c < ncols; c++) {
			local_widths[c] = local_widths[c] * target_w / total_w;
			if (local_widths[c] < 45) local_widths[c] = 45;
			scaled_w += local_widths[c];
		}
		while (scaled_w > target_w) {
			for (int c = ncols - 1; c >= 0 && scaled_w > target_w; c--) {
				if (local_widths[c] > 45) {
					local_widths[c]--;
					scaled_w--;
				}
			}
		}
		total_w = scaled_w;
	}
	int table_x = UI_PANEL_X + (UI_PANEL_W - total_w) / 2;
	int table_y = TABLE_TOP;
	int header_h = TABLE_HDR_H;
	int row_h = ROW_H;

	int pages = (total + page_size - 1) / page_size;
	int cur_page = 0;
	int cur_row = 0;

	while (1) {
		cleardevice();
		redraw_bg();
		ui_draw_panel();

		/* 标题栏 */
		settextstyle(FONT_HEADER_H, FONT_HEADER_W, _T("黑体"));
		char title_buf[256];
		sprintf(title_buf, "%s  第 %d/%d 页  共 %d 条", title, cur_page + 1, pages, total);
		settextcolor(TEXT_MAIN);
		int title_cx = text_center_x(UI_PANEL_X, UI_PANEL_X + UI_PANEL_W, title_buf);
		outtextxy(title_cx, table_y - header_h, title_buf);

		/* ---- 表头：40px 高，BG_TABLE_HDR 底色 ---- */
		setfillcolor(BG_TABLE_HDR);
		int x = table_x;
		for (int c = 0; c < ncols; c++) {
			fillrectangle(x, table_y, x + local_widths[c], table_y + header_h);
			setlinecolor(GRAY_LINE);
			rectangle(x, table_y, x + local_widths[c], table_y + header_h);
			settextcolor(TEXT_MAIN);
			settextstyle(FONT_TABLE_H, FONT_TABLE_W, _T("黑体"));
			int cx = text_center_x(x, x + local_widths[c], headers[c]);
			int cy = text_center_y(table_y, table_y + header_h, headers[c]);
			outtextxy(cx, cy, (char *)headers[c]);
			x += local_widths[c];
		}

		/* ---- 数据行：35px，网格线完整 ---- */
		int start = cur_page * page_size;
		int end = (start + page_size < total) ? start + page_size : total;
		for (int r = start; r < end; r++) {
			int local_r = r - start;
			int ry = table_y + header_h + local_r * row_h;

			/* 行高亮 */
			if (local_r == cur_row) {
				setfillcolor(LIGHT_CYAN);
				fillrectangle(table_x, ry, table_x + total_w, ry + row_h);
			}

			/* 绘制行数据 */
			draw_row(records[r], r, ry, table_x, local_widths, ncols);

			/* 列间竖线 */
			setlinecolor(GRAY_LINE);
			int vx = table_x;
			for (int c = 0; c < ncols; c++) {
				vx += local_widths[c];
				line(vx, ry, vx, ry + row_h);
			}
			/* 行底横线 */
			line(table_x, ry + row_h, table_x + total_w, ry + row_h);
		}
		/* 表格外框 */
		setlinecolor(GRAY_LINE);
		rectangle(table_x, table_y, table_x + total_w,
		          table_y + header_h + (end - start) * row_h);

		/* ---- 底栏翻页 ---- */
		int bottom_y = table_y + header_h + page_size * row_h + 30;
		settextstyle(FONT_SMALL_H, FONT_SMALL_W, _T("黑体"));

		if (cur_page > 0) {
			setfillcolor(PRIMARY);
			fillrectangle(PAGE_LEFT_X1, PAGE_LEFT_Y1, PAGE_LEFT_X2, PAGE_LEFT_Y2);
			settextcolor(WHITE_COLOR);
			outtextxy(PAGE_LEFT_X1 + 12, PAGE_LEFT_Y1 + 10, "<-上页");
		} else {
			settextcolor(TEXT_MUTED);
			outtextxy(PAGE_LEFT_X1, PAGE_LEFT_Y1 + 10, "<-上页");
		}
		if (cur_page < pages - 1) {
			setfillcolor(PRIMARY);
			fillrectangle(PAGE_RIGHT_X1, PAGE_RIGHT_Y1, PAGE_RIGHT_X2, PAGE_RIGHT_Y2);
			settextcolor(WHITE_COLOR);
			outtextxy(PAGE_RIGHT_X1 + 12, PAGE_RIGHT_Y1 + 10, "下页->");
		} else {
			settextcolor(TEXT_MUTED);
			outtextxy(PAGE_RIGHT_X1, PAGE_RIGHT_Y1 + 10, "下页->");
		}

		settextcolor(TEXT_MUTED);
		int hint_cx = text_center_x(0, WIN_W, "↑↓:选择行  ←→:翻页  Enter:确认  Esc:返回");
		outtextxy(hint_cx, bottom_y, "↑↓:选择行  ←→:翻页  Enter:确认  Esc:返回");

		/* ---- 交互 ---- */
		ExMessage tm = getmessage(EX_KEY | EX_MOUSE);
		if (tm.message == WM_KEYDOWN) {
			if (tm.vkcode == VK_ESCAPE) {
				free(records);
				return 0;
			}
			if (tm.vkcode == VK_UP && cur_row > 0) cur_row--;
			if (tm.vkcode == VK_DOWN && cur_row < (end - start) - 1) cur_row++;
			if (tm.vkcode == VK_LEFT && cur_page > 0) { cur_page--; cur_row = 0; }
			if (tm.vkcode == VK_RIGHT && cur_page < pages - 1) { cur_page++; cur_row = 0; }
			if (tm.vkcode == VK_RETURN) {
				int selected = cur_page * page_size + cur_row + 1;
				free(records);
				return selected;
			}
		}
		if (tm.message == WM_LBUTTONDOWN) {
			int mx = tm.x, my = tm.y;
			if (cur_page > 0 &&
			    mx >= PAGE_LEFT_X1 && mx <= PAGE_LEFT_X2 &&
			    my >= PAGE_LEFT_Y1 && my <= PAGE_LEFT_Y2) {
				cur_page--; cur_row = 0;
			}
			if (cur_page < pages - 1 &&
			    mx >= PAGE_RIGHT_X1 && mx <= PAGE_RIGHT_X2 &&
			    my >= PAGE_RIGHT_Y1 && my <= PAGE_RIGHT_Y2) {
				cur_page++; cur_row = 0;
			}
			for (int r = start; r < end; r++) {
				int ry = table_y + header_h + (r - start) * row_h;
				if (mx >= table_x && mx <= table_x + total_w &&
				    my >= ry && my <= ry + row_h) {
					cur_row = r - start;
				}
			}
		}
	}
}
