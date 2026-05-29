#ifndef WAREHOUSEWIN_H
#define WAREHOUSEWIN_H

#include "../public/common.h"

/* ============================================================
 *  仓储 UI 模块 — 仓库配置 + 仓储管理所有窗口界面
 *
 *  供以下角色复用：
 *  - 管理员    : warehouseConfigWin
 *  - 仓储员    : warehouseMgmtWin
 * ============================================================ */

/* 仓库配置窗口（图8）— 新增仓库 + 列表查看 */
void warehouseConfigWin();

/* 仓储管理主菜单（图10）— 货物入库/货物出库/库存管理/返回 */
void warehouseMgmtWin();

#endif
