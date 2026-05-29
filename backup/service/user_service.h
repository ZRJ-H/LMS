#ifndef USER_SERVICE_H
#define USER_SERVICE_H
#include "../public/common.h"

#define USER_TXT_FILE "data/users.txt"
#define USER_DAT_FILE "data/users.dat"

/* 初始化：优先从 data/users.txt 加载；旧 data/users.dat 仅用于一次性迁移 */
int  user_svc_init();

/* 持久化当前用户链表到文件 */
int  user_svc_save();

/* 认证：校验凭据 + 失败锁定。成功返回 User*，失败返回 NULL，err_msg 写入原因 */
User *user_svc_auth(const char *name, const char *pwd, char *err_msg, int err_len);

/* 创建用户（含重名校验 + 密码合法性校验），自动保存 */
int  user_svc_create(const char *name, const char *pwd, UserRole role, char *err_msg, int err_len);

/* 按用户名重置密码为 888888，自动保存。返回 0 成功，-1 用户不存在 */
int  user_svc_reset_password(const char *name);

/* 查找用户 */
User *user_svc_find_by_name(const char *name);

/* 获取用户链表头 */
User *user_svc_list_all();

/* 获取用户总数 */
int  user_svc_count();

#endif
