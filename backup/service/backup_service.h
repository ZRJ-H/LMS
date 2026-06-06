#ifndef BACKUP_SERVICE_H
#define BACKUP_SERVICE_H

#include <windows.h>

/* 创建时间戳备份目录并复制当前数据，返回复制文件数 */
int backup_svc_create_manual(char *out_dir, int out_len);

/* 从最近一次备份恢复数据文件，返回恢复文件数；-1 表示无备份 */
int backup_svc_restore_latest();

#endif
