---
name: lms-ui
description: Generate EasyX UI code that follows LMS project conventions. Use when the user asks to create a new window, add controls, or build an interface screen.
---

# LMS UI Generator

Generate EasyX interface code that reuses the project's existing CONTROL_T / WINDOW_T system.

## Quick Reference

### CONTROL_T Fields (defined in `backup/view/control.h`)
```
{x, y, width, height, "text", bgColor1, bgColor2, textColor, type, state, visible}
```
- `bgColor1` = selected state fill color
- `bgColor2` = unselected state fill color
- `state` = 1 (selected/focused) or 0 (unselected)
- `visible` = 1 (show plaintext for EDIT_PWD) or 0 (show *)

### Control Types
| Constant | Value | Behavior |
|----------|-------|----------|
| BUTTON   | 1     | Clickable, text centered, fillRectangle |
| EDIT     | 2     | Text input, left-aligned at x+5,y+15 |
| EDIT_PWD | 3     | Password input, Tab toggles plaintext/masked |
| LABEL    | 4     | Non-interactive text, skipped by arrow nav |

### Color Convention
| Context | Selected (bgColor1) | Unselected (bgColor2) | Text |
|---------|---------------------|-----------------------|------|
| Normal button | CYAN | LIGHTCYAN | WHITE |
| Danger button (退出/注销) | LIGHTRED | RED | WHITE |
| Label/Edit bg | WHITE | WHITE | BLACK |

### Fixed Constants
- Font: `settextstyle(16, 10, _T("宋体"))`
- Button text position: `x + (width - strlen(text)*10) / 2, y + 15`
- Label/Edit text position: `x + 5, y + 15`

### Navigation Rules (implemented in `window_run`)
- UP/DOWN arrows: skip LABEL type controls, cycle through others
- ENTER: activate focused BUTTON (returns win.current = control index)
- TAB: toggle EDIT_PWD visible/asterisk mode
- BACKSPACE: delete last character in EDIT/EDIT_PWD
- Alphanumeric keys: append to EDIT/EDIT_PWD text

### Event Loop Pattern
```c
while (1) {
    window_show(win);
    win = window_run(win);  // blocks until ENTER on a BUTTON
    switch (win.current) {
    case N: /* button N clicked */; break;
    }
}
```
`win.current` = the index of the BUTTON that was activated (1-indexed in the controls array).

---

## Templates

### Template 1: Button Menu Window

Use for role menus, settings screens, any "pick from a list of buttons" UI.

```c
/* ========== [窗口名] ========== */
if (current_user->role == ROLE_XXX) {
    WINDOW_T win = {
        /* 窗口: x, y, width, height */
        220, 100, 360, [height], WHITE, [control_count], {
            /* 索引0: 标题 LABEL (不可导航) */
            {230, 115, 340, 30, "标题文字", WHITE, WHITE, BLACK, LABEL, 0},
            /* 索引1-N: 功能 BUTTON (第一个 state=1 默认选中) */
            {240, 170, 320, 55, "功能A", CYAN, LIGHTCYAN, WHITE, BUTTON, 1},
            {240, 235, 320, 55, "功能B", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
            {240, 300, 155, 55, "注销登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
            {425, 300, 135, 55, "退出系统", LIGHTRED, RED, WHITE, BUTTON, 0},
        }
    };
    strcpy(win.controls[0].text, title_string);  /* 动态标题 */

    while (1) {
        window_show(win);
        win = window_run(win);
        switch (win.current) {
        case 1: /* 功能A */; break;
        case 2: /* 功能B */; break;
        case 3: current_user = NULL; return 0;   /* 注销 */
        case 4: return -1;                        /* 退出 */
        }
    }
}
```

**Layout math for N buttons:**
- Window height = 80 + N * 65 + 60 (title area + buttons + bottom padding)
- Button y = 150 + (button_index - 1) * 65  (first button at y=150, each next +65)
- "注销" and "退出" are side-by-side: left at x=240/w=155, right at x=425/w=135

### Template 2: Form Input Window

Use for login, user creation, any "fill in fields" UI.

```c
/* ========== [表单名] ========== */
WINDOW_T form = {
    280, 160, 240, [height], WHITE, [control_count], {
        /* 标签 + 输入框 */
        {290, 170, 100, 30, "用户名:", WHITE, WHITE, BLACK, LABEL, 0},
        {390, 170, 120, 30, "", WHITE, WHITE, BLACK, EDIT, 1},
        {290, 210, 100, 30, "密  码:", WHITE, WHITE, BLACK, LABEL, 0},
        {390, 210, 120, 30, "", WHITE, WHITE, BLACK, EDIT_PWD, 0},
        /* 操作按钮 */
        {290, 260, 90, 50, "登录", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
        {410, 260, 90, 50, "返回", CYAN, LIGHTCYAN, WHITE, BUTTON, 0},
    }
};

while (1) {
    window_show(form);
    form = window_run(form);
    if (form.current == 4) {  /* "登录" button index */
        if (strlen(form.controls[1].text) == 0 ||
            strlen(form.controls[3].text) == 0) {
            MessageBoxA(NULL, "用户名或密码不能为空", "错误", MB_OK);
            continue;
        }
        /* ... auth logic ... */
    }
    break;  /* "返回" */
}
```

### Template 3: Paginated List Window (GUI)

Use for displaying data tables with LEFT/RIGHT page switching.

```c
/* ========== 分页列表 ========== */
static int paginatedListWin(void) {
    const int PAGE_SIZE = 8;
    int *records = NULL;  /* collect data pointers */
    int total = load_records(&records);
    int curPage = 0;
    int totalPages = (total + PAGE_SIZE - 1) / PAGE_SIZE;
    ExMessage msg;

    while (1) {
        cleardevice();

        /* 标题栏 */
        char header[64];
        sprintf(header, "第 %d/%d 页  共 %d 条", curPage + 1, totalPages, total);
        settextstyle(16, 10, _T("宋体"));
        outtextxy(20, 10, header);

        /* 表头 */
        outtextxy(20, 35, "序号  字段1  字段2  状态");

        /* 数据行 */
        int start = curPage * PAGE_SIZE;
        int end = (start + PAGE_SIZE < total) ? start + PAGE_SIZE : total;
        for (int i = start; i < end; i++) {
            int rowY = 60 + (i - start) * 25;
            char buf[256];
            sprintf(buf, "%d    %s    %s    %s", i + 1, ...);
            outtextxy(20, rowY, buf);
        }

        /* 底部提示 */
        outtextxy(20, 560, "← → 翻页  Esc 返回");

        /* 等待输入 */
        msg = getmessage(EX_KEY);
        if (msg.vkcode == VK_ESCAPE) break;
        if (msg.vkcode == VK_LEFT  && curPage > 0) curPage--;
        if (msg.vkcode == VK_RIGHT && curPage < totalPages - 1) curPage++;
    }

    free(records);
    return 0;
}
```

---

## Step-by-Step Guide

When the user asks for a new interface, follow this sequence:

### Step 1: Choose the template
- "选择列表/按钮菜单" → Template 1 (Button Menu)
- "需要输入文字/密码" → Template 2 (Form Input)
- "需要翻页查看数据" → Template 3 (Paginated List)
- Mix of above → combine templates

### Step 2: Calculate layout
- Window width: usually 360 (menu) or 240 (form)
- Window height: `80 + N * 65 + 60` for N buttons
- Button positions: `y = 150 + (i-1) * 65` for button i (in menu template)

### Step 3: Assign control indices
- Index 0 is always the title LABEL
- Index 1..N are BUTTONs (first BUTTON gets state=1)
- "注销" and "退出" are side-by-side at the bottom
- For form template, LABEL+EDIT pairs take adjacent indices

### Step 4: Write the event loop
```c
while (1) {
    window_show(win);
    win = window_run(win);
    switch (win.current) {
    case 1: /* handle button 1 */; break;
    case N: return XX;  /* window ID for nav */
    }
}
```

### Step 5: Wire up window navigation
- Return a window ID (0=startWin, 1=loginWin, 2=mainWin) or sub-window return 0
- Add function pointer to main.cpp's jump table if it's a top-level window
- Call sub-windows from their parent's switch-case

---

## Constraints
- Only use `CONTROL_T` / `WINDOW_T` / `control_show` / `window_show` / `window_run`
- Only use EasyX native functions + standard C
- Integer pixel values — no proportional calculation
- New window functions named `xxxWin()`
- Chinese comments, one-line function description at top
- Do NOT use `getch()` — always use `getmessage()` for keyboard input
