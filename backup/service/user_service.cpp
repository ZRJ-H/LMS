#include "user_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

/* ============================================================
 *  初始化：优先从 data/users.dat 加载
 *  失败 → 调 init_sample_users() → 落盘
 *  加载后清理已过期的 lockout，并恢复 user_id_counter
 * ============================================================ */
int user_svc_init() {
	/* 尝试加载 */
	int count = bin_load_list(USER_DAT_FILE, (void **)&user_list_head, sizeof(User), offsetof(User, next));//介绍这些二进制文件的函数，告诉我它们是干什么的，参数是什么，返回值是什么 --- IGNORE ---
	if (count > 0) {
		/* 清理过期锁定 + 恢复 ID 计数器 */
		User *p = user_list_head;
		int max_id = 0;
		time_t now = time(NULL);
		while (p) {
			if (p->id > max_id) max_id = p->id;
			if (p->lockout_until > 0 && now >= p->lockout_until) {
				p->failed_attempts = 0;
				p->lockout_until = 0;
			}
			p = p->next;
		}
		if (max_id >= user_id_counter) user_id_counter = max_id;
		return count;
	}

	/* 无文件 → 初始化示例用户并保存 */
	init_sample_users();
	user_svc_save();
	return user_id_counter; /* 返回创建的用户数 */
}

/* 持久化 */
int user_svc_save() {
	return bin_save_list(USER_DAT_FILE, user_list_head, sizeof(User), offsetof(User, next));
}

/* 认证 */
User *user_svc_auth(const char *name, const char *pwd, char *err_msg, int err_len) {
	if (!name || !pwd || strlen(name) == 0 || strlen(pwd) == 0) {
		strncpy(err_msg, "账号或密码不能为空", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return NULL;
	}

	if (validate_password(pwd) != 0) {
		strncpy(err_msg, "密码长度须为6-10位，仅字母数字", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return NULL;
	}

	User *u = find_user_by_name(name);
	if (!u) {
		strncpy(err_msg, "账号或密码错误", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return NULL;
	}

	/* 检查锁定 */
	if (u->lockout_until > 0 && time(NULL) < u->lockout_until) {
		strncpy(err_msg, "账号已被锁定5分钟，请稍后再试", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return NULL;
	}

	/* 校验密码哈希 */
	char hash[33];
	md5_hash(pwd, hash);
	if (strcmp(hash, u->password) != 0) {
		u->failed_attempts++;
		if (u->failed_attempts >= 3) {
			u->lockout_until = time(NULL) + 300;
			user_svc_save();
			strncpy(err_msg, "密码错误3次，账号已锁定5分钟", err_len - 1);
		} else {
			sprintf(err_msg, "账号或密码错误（剩余 %d 次机会）", 3 - u->failed_attempts);
		}
		err_msg[err_len - 1] = '\0';
		return NULL;
	}

	/* 登录成功，清除失败记录 */
	u->failed_attempts = 0;
	u->lockout_until = 0;
	return u;
}

/* 创建用户 */
int user_svc_create(const char *name, const char *pwd, UserRole role, char *err_msg, int err_len) {
	int ret = add_user(name, pwd, role);
	if (ret == -1) {
		strncpy(err_msg, "用户名已存在", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return -1;
	}
	if (ret == -2) {
		strncpy(err_msg, "密码不合法（需6-10位字母或数字）", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return -2;
	}
	if (ret == -3) {
		strncpy(err_msg, "内存分配失败", err_len - 1);
		err_msg[err_len - 1] = '\0';
		return -3;
	}

	user_svc_save();
	return 0;
}

/* 重置密码 */
int user_svc_reset_password(const char *name) {
	User *u = find_user_by_name(name);
	if (!u) return -1;
	reset_password(u);
	user_svc_save();
	return 0;
}

/* 查找用户 */
User *user_svc_find_by_name(const char *name) {
	return find_user_by_name(name);
}

/* 获取链表头 */
User *user_svc_list_all() {
	return user_list_head;
}

/* 用户总数 */
int user_svc_count() {
	return list_count(user_list_head, offsetof(User, next));
}
