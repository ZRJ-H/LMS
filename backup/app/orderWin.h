#ifndef ORDERWIN_H
#define ORDERWIN_H

/* ============================================================
 *  订单 UI 模块 — 所有与订单相关的窗口界面
 *
 *  本模块从 mainWin.cpp 解耦独立，供以下角色复用：
 *  - 管理员   : createOrderWin / searchOrderWin / auditOrderWin
 *  - 客服     : createOrderWin / searchOrderWin
 *  - 客户     : searchOrderWin
 *  - 仓储/调度 : 后续扩展
 * ============================================================ */

/* 创建物流订单 — 表单输入客户姓名/电话/地址/货物类型/送达时间
 * 创建后自动调用 order_svc_save() 持久化，状态设为 ORDER_PENDING_REVIEW */
void createOrderWin();

/* 订单查询筛选 — 支持按订单号/客户名/状态(COMBO)组合条件筛选
 * 结果以分页表格展示 */
void searchOrderWin();

/* 订单审核 — 仅管理员调用
 * 分页展示所有待审核订单，输入订单号后：
 *   审核通过 → ORDER_PENDING_OUT
 *   驳回     → ORDER_REJECTED + 填写驳回原因
 * 每次操作后自动 order_svc_save() 落盘 */
void auditOrderWin();

#endif
