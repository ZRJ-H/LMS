#include "backup_service.h"
#include "user_service.h"
#include "order_service.h"
#include "warehouse_service.h"
#include "transport_service.h"
#include "../public/common.h"
#include <stdio.h>
#include <string.h>

static const char *BACKUP_FILES[] = {
	USER_TXT_FILE,
	ORDER_TXT_FILE,
	WAREHOUSE_TXT_FILE,
	INOUT_RECORD_TXT_FILE,
	INVENTORY_TXT_FILE,
	LOG_TXT_FILE,
	VEHICLE_TXT_FILE,
	DRIVER_TXT_FILE,
	ROUTE_TXT_FILE,
	DISPATCH_TXT_FILE,
	TRACKING_TXT_FILE,
	"data/order_seq.txt",
	"data/dispatch_seq.txt",
};

static int backup_file_count() {
	return (int)(sizeof(BACKUP_FILES) / sizeof(BACKUP_FILES[0]));
}

static void make_backup_dir(char *dir, int len) {
	char now[20];
	get_current_time_str(now);
	sprintf(dir, "data\\backup\\%.4s%.2s%.2s_%.2s%.2s%.2s",
	        now, now + 5, now + 8, now + 11, now + 14, now + 17);
	dir[len - 1] = '\0';
}

static const char *basename_from_path(const char *path) {
	const char *name = strrchr(path, '/');
	if (!name) name = strrchr(path, '\\');
	return name ? name + 1 : path;
}

int backup_svc_create_manual(char *out_dir, int out_len) {
	int copied = 0;
	char dst[MAX_PATH];

	user_svc_save();
	order_svc_save();
	warehouse_svc_save_all();
	transport_svc_save_all();

	CreateDirectoryA("data", NULL);
	CreateDirectoryA("data\\backup", NULL);
	make_backup_dir(out_dir, out_len);
	CreateDirectoryA(out_dir, NULL);

	for (int i = 0; i < backup_file_count(); i++) {
		const char *src = BACKUP_FILES[i];
		sprintf(dst, "%s\\%s", out_dir, basename_from_path(src));
		if (GetFileAttributesA(src) != INVALID_FILE_ATTRIBUTES &&
		    CopyFileA(src, dst, FALSE)) {
			copied++;
		}
	}
	return copied;
}

static int find_latest_backup_dir(char *out_dir, int len) {
	WIN32_FIND_DATAA fd;
	HANDLE h = FindFirstFileA("data\\backup\\*", &fd);
	char latest[MAX_PATH] = {0};

	if (h == INVALID_HANDLE_VALUE) return 0;
	do {
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
		    strcmp(fd.cFileName, ".") != 0 &&
		    strcmp(fd.cFileName, "..") != 0) {
			if (strcmp(fd.cFileName, latest) > 0)
				strcpy(latest, fd.cFileName);
		}
	} while (FindNextFileA(h, &fd));
	FindClose(h);

	if (latest[0] == '\0') return 0;
	sprintf(out_dir, "data\\backup\\%s", latest);
	out_dir[len - 1] = '\0';
	return 1;
}

int backup_svc_restore_latest() {
	char latest_dir[MAX_PATH];
	char src[MAX_PATH];
	int restored = 0;

	if (!find_latest_backup_dir(latest_dir, sizeof(latest_dir))) return -1;

	for (int i = 0; i < backup_file_count(); i++) {
		const char *dst = BACKUP_FILES[i];
		sprintf(src, "%s\\%s", latest_dir, basename_from_path(dst));
		if (GetFileAttributesA(src) != INVALID_FILE_ATTRIBUTES &&
		    CopyFileA(src, dst, FALSE)) {
			restored++;
		}
	}
	return restored;
}
