#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include <graphics.h>

/* ============================================================
 *  全局 UI 基础参数 — 全项目唯一颜色/尺寸/坐标常量定义
 *  所有魔术数字均从此文件引用，禁止在其他文件中硬编码
 * ============================================================ */

/* ---- 画布基础尺寸 ---- */
const int WIN_W = 800;
const int WIN_H = 600;

/* ---- 统一色彩搭配 (RGB 格式) ---- */
const COLORREF PRIMARY      = RGB(30, 144, 255);  /* 道奇蓝 — 高亮选中、主按钮、焦点边框 */
const COLORREF TEXT_MAIN    = RGB(44, 62, 80);    /* 深色主文字 — 标题、正文 */
const COLORREF TEXT_MUTED   = RGB(127, 140, 141); /* 灰色副文本 — 提示信息 */
const COLORREF WHITE_COLOR  = RGB(255, 255, 255); /* 纯白 — 卡片背景、输入框背景 */
const COLORREF BG_TABLE_HDR = RGB(230, 235, 240); /* 表头浅灰底色 */
const COLORREF BG_WINDOW    = RGB(255, 255, 255); /* 窗口默认底色 */
const COLORREF BORDER_COLOR = RGB(30, 144, 255);  /* 边框主色（蓝） */
const COLORREF BLACK_COLOR  = RGB(0, 0, 0);       /* 纯黑 — 默认边框 */
const COLORREF RED_BTN      = RGB(220, 53, 69);   /* 危险按钮（驳回/删除）选中 */
const COLORREF LIGHT_RED    = RGB(248, 215, 218); /* 危险按钮未选中底色 */
const COLORREF CYAN_COLOR   = RGB(0, 188, 212);   /* 已弃用 — 不再用于输入框底色 */
const COLORREF LIGHT_CYAN   = RGB(224, 247, 250); /* 表格行高亮 */
const COLORREF INPUT_BG     = RGB(245, 245, 245); /* 输入框未选中底色（极浅灰） */
const COLORREF INPUT_BORDER = RGB(200, 200, 200); /* 输入框未选中边框 */
const COLORREF GRAY_LINE    = RGB(210, 210, 210); /* 表格网格线 */
const COLORREF CARD_BG      = RGB(255, 255, 255); /* 白卡片底色 */

/* ---- 白卡片隔离层 ---- */
const int CARD_X1 = 80;
const int CARD_Y1 = 60;
const int CARD_X2 = 720;
const int CARD_Y2 = 540;
const int CARD_R  = 15;  /* 圆角半径 */

/* ---- 通用组件标准尺寸 ---- */
const int BTN_W = 160;
const int BTN_H = 40;
const int INPUT_W = 300;   /* 输入框统一宽度 */
const int INPUT_H = 35;    /* 输入框统一高度 */
const int INPUT_GAP = 15;  /* 输入框垂直间距 */

/* ---- 欢迎与登录界面 ---- */
const int LOGIN_TITLE_Y    = 80;
const int LOGIN_USER_X     = 280;
const int LOGIN_USER_Y     = 220;
const int LOGIN_PWD_X      = 280;
const int LOGIN_PWD_Y      = 280;
const int LOGIN_BTN_Y      = 380;
const int LOGIN_LABEL_X    = 220;
const int LOGIN_PWD_TOGGLE_X = 565;
const int LOGIN_PWD_TOGGLE_Y = 280;
const int LOGIN_PWD_TOGGLE_SZ = 35;

/* ---- 主菜单网格 ---- */
const int MENU_COL_LEFT  = 200;
const int MENU_COL_RIGHT = 420;
const int MENU_W = 180;
const int MENU_H = 45;
const int MENU_ROWS_Y[4] = {180, 245, 310, 375};

/* ---- 通用数据查询与列表（表格型）---- */
const int TABLE_LEFT   = 80;
const int TABLE_RIGHT  = 720;
const int TABLE_TOP    = 170;
const int TABLE_BOTTOM = 480;
const int ROW_H        = 35;
const int TABLE_HDR_H  = 40;    /* 表头高度 */
const int PAGE_SIZE    = 7;

/* 搜索栏（卡片内坐标） */
const int SEARCH_INPUT_X = 150;
const int SEARCH_INPUT_Y = 100;
const int SEARCH_BTN_X   = 470;
const int SEARCH_BTN_Y   = 100;
const int SEARCH_BTN_W   = 100;
const int SEARCH_INPUT_W = 300;

/* 分页按钮热区 */
const int PAGE_LEFT_X1  = 200;
const int PAGE_LEFT_Y1  = 510;
const int PAGE_LEFT_X2  = 260;
const int PAGE_LEFT_Y2  = 540;
const int PAGE_RIGHT_X1 = 540;
const int PAGE_RIGHT_Y1 = 510;
const int PAGE_RIGHT_X2 = 600;
const int PAGE_RIGHT_Y2 = 540;

/* ---- 字体规格 ---- */
const int FONT_TITLE_H  = 32;
const int FONT_TITLE_W  = 0;
const int FONT_BTN_H    = 16;
const int FONT_BTN_W    = 8;
const int FONT_TABLE_H  = 14;
const int FONT_TABLE_W  = 8;
const int FONT_SMALL_H  = 14;
const int FONT_SMALL_W  = 8;
const int FONT_HEADER_H = 20;
const int FONT_HEADER_W = 12;

/* ---- 通用控件间距 ---- */
const int CTRL_PADDING   = 5;
const int CTRL_GAP       = 20;
const int GRID_ROW_GAP   = 20;

#endif /* UI_CONFIG_H */
