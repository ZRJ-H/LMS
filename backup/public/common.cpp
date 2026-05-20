#include "common.h"
/* ============================================================
 *           全局链表头指针（定义） 表示各种链表的头指针 
 * ============================================================ */
Goods             *goods_list_head        = NULL;
User              *user_list_head         = NULL;
Order             *order_list_head        = NULL;
OrderDetail       *order_detail_list_head = NULL;
Warehouse         *warehouse_list_head    = NULL;
InOutRecord       *inout_record_list_head = NULL;
Inventory         *inventory_list_head    = NULL;
Route             *route_list_head        = NULL;
Vehicle           *vehicle_list_head      = NULL;
Driver            *driver_list_head       = NULL;
Dispatch          *dispatch_list_head     = NULL;
TransportTracking *tracking_list_head     = NULL;
OperationLog      *log_list_head          = NULL;


/* 全局 ID 计数器 */
//用于管理id 
int goods_id_counter          = 0;
int user_id_counter           = 0;
int order_detail_id_counter   = 0;
int warehouse_id_counter      = 0;
int inout_record_id_counter   = 0;
int inventory_id_counter      = 0;
int route_id_counter          = 0;
int vehicle_id_counter        = 0;
int driver_id_counter         = 0;
int tracking_id_counter       = 0;
int log_id_counter            = 0;
int order_sequence            = 0;


/* 当前登录用户 */
User *current_user = NULL;

/* ============================================================
 *           时间工具函数
 * ============================================================ */

/* 获取当前时间字符串 YYYY-MM-DD HH:MM:SS */
//需要用多大的数组接受呢？至少20字节，格式是19位时间+1结束符，留余量安全起见。
void get_current_time_str(char *buf) {
   time_t now =time(NULL);//获取当前时间戳
   struct tm *t=localtime(&now);//获取当前时间 
   sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",t->tm_year +1900,t->tm_mon +1,t->tm_mday 
   ,t->tm_hour ,t->tm_min ,t->tm_sec ); 
}

/* 获取当前日期字符串 YYYYMMDD */
//需要用多大的数组接受呢？至少16字节，格式是8位日期+1结束符，留余量安全起见。
void get_current_date_str(char *buf) {
    time_t now = time(NULL);
    struct tm *t =localtime(&now);
    sprintf(buf,"%04d%02d%02d",t->tm_year +1900,t->tm_mon +1,t->tm_mday  );
}

/* 计算两个时间戳之间的秒数差 */
long time_diff_seconds(time_t from, time_t to) {
    return (long)(to - from);
    
}

/* ============================================================
 *           密码输入函数
 *           - 支持退格键删除
 *           - Tab 键切换明文/密文显示
 *           - 最大长度 max_len-1（留 \0）
 * ============================================================ */
void input_password(char *buf, int max_len) {
	char ch;
	int i=0;
	int visible=0;//明文/密文显示标识 ,值为1则是明文显示。 
	//循环条件： 用户未完成输入 
	while(1){
		ch=_getch();
		if(ch=='r'||ch=='\n'){//如果输入的是回车或者换行 
			break;
		} 
		else if(ch=='\t'){//如果输入的是tab键 
			visible = !visible;
            /* 退回到输入起始位置，擦除，重绘 */
            int k = 0; 
            for (k = 0; k < i; k++) printf("\b \b");
            for (k = 0; k < i; k++) printf("%c", visible ? buf[k] : '*');
            continue;
		}
		else if(ch=='\b'||ch==127){//如果输入的是退格键或者删除键 
			if(i>0){//
				i--;
				printf("\b \b");
			}
		}
		else if(i<max_len-1&&ch>=32&&ch<127){
			buf[i++]=ch;
			printf("%c",visible?ch:'*');
		}
		}
		buf[i]='\0';
		printf("\n");
	} 
    


/* 明文密码比较 */
int verify_password(const char *input, const char *stored) {
    return strcmp(input, stored) == 0;
}

/* 密码合法性校验：6-10位，仅字母数字 */
int validate_password(const char *pwd) {
    int len = (int)strlen(pwd);
    if (len < 6 || len > 10) return -1;
    for (int i = 0; i < len; i++) {
        char c = pwd[i];
        if (!(c >= '0' && c <= '9') &&
            !(c >= 'a' && c <= 'z') &&
            !(c >= 'A' && c <= 'Z'))
            return -2;
    }
    return 0;
}

/* ========== MD5 哈希实现 (RFC 1321) ========== */
typedef struct {
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
} MD5_CTX;

#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac) { a+=F(b,c,d)+x+ac; a=ROTATE_LEFT(a,s); a+=b; }
#define GG(a,b,c,d,x,s,ac) { a+=G(b,c,d)+x+ac; a=ROTATE_LEFT(a,s); a+=b; }
#define HH(a,b,c,d,x,s,ac) { a+=H(b,c,d)+x+ac; a=ROTATE_LEFT(a,s); a+=b; }
#define II(a,b,c,d,x,s,ac) { a+=I(b,c,d)+x+ac; a=ROTATE_LEFT(a,s); a+=b; }

static void MD5Transform(unsigned int state[4], unsigned char block[64]) {
    unsigned int a=state[0], b=state[1], c=state[2], d=state[3], x[16];
    for (int i=0,j=0; i<16; i++,j+=4)
        x[i]=((unsigned int)block[j])|(((unsigned int)block[j+1])<<8)|
             (((unsigned int)block[j+2])<<16)|(((unsigned int)block[j+3])<<24);
    FF(a,b,c,d,x[ 0], 7,0xd76aa478); FF(d,a,b,c,x[ 1],12,0xe8c7b756);
    FF(c,d,a,b,x[ 2],17,0x242070db); FF(b,c,d,a,x[ 3],22,0xc1bdceee);
    FF(a,b,c,d,x[ 4], 7,0xf57c0faf); FF(d,a,b,c,x[ 5],12,0x4787c62a);
    FF(c,d,a,b,x[ 6],17,0xa8304613); FF(b,c,d,a,x[ 7],22,0xfd469501);
    FF(a,b,c,d,x[ 8], 7,0x698098d8); FF(d,a,b,c,x[ 9],12,0x8b44f7af);
    FF(c,d,a,b,x[10],17,0xffff5bb1); FF(b,c,d,a,x[11],22,0x895cd7be);
    FF(a,b,c,d,x[12], 7,0x6b901122); FF(d,a,b,c,x[13],12,0xfd987193);
    FF(c,d,a,b,x[14],17,0xa679438e); FF(b,c,d,a,x[15],22,0x49b40821);
    GG(a,b,c,d,x[ 1], 5,0xf61e2562); GG(d,a,b,c,x[ 6], 9,0xc040b340);
    GG(c,d,a,b,x[11],14,0x265e5a51); GG(b,c,d,a,x[ 0],20,0xe9b6c7aa);
    GG(a,b,c,d,x[ 5], 5,0xd62f105d); GG(d,a,b,c,x[10], 9,0x02441453);
    GG(c,d,a,b,x[15],14,0xd8a1e681); GG(b,c,d,a,x[ 4],20,0xe7d3fbc8);
    GG(a,b,c,d,x[ 9], 5,0x21e1cde6); GG(d,a,b,c,x[14], 9,0xc33707d6);
    GG(c,d,a,b,x[ 3],14,0xf4d50d87); GG(b,c,d,a,x[ 8],20,0x455a14ed);
    GG(a,b,c,d,x[13], 5,0xa9e3e905); GG(d,a,b,c,x[ 2], 9,0xfcefa3f8);
    GG(c,d,a,b,x[ 7],14,0x676f02d9); GG(b,c,d,a,x[12],20,0x8d2a4c8a);
    HH(a,b,c,d,x[ 5], 4,0xfffa3942); HH(d,a,b,c,x[ 8],11,0x8771f681);
    HH(c,d,a,b,x[11],16,0x6d9d6122); HH(b,c,d,a,x[14],23,0xfde5380c);
    HH(a,b,c,d,x[ 1], 4,0xa4beea44); HH(d,a,b,c,x[ 4],11,0x4bdecfa9);
    HH(c,d,a,b,x[ 7],16,0xf6bb4b60); HH(b,c,d,a,x[10],23,0xbebfbc70);
    HH(a,b,c,d,x[13], 4,0x289b7ec6); HH(d,a,b,c,x[ 0],11,0xeaa127fa);
    HH(c,d,a,b,x[ 3],16,0xd4ef3085); HH(b,c,d,a,x[ 6],23,0x04881d05);
    HH(a,b,c,d,x[ 9], 4,0xd9d4d039); HH(d,a,b,c,x[12],11,0xe6db99e5);
    HH(c,d,a,b,x[15],16,0x1fa27cf8); HH(b,c,d,a,x[ 2],23,0xc4ac5665);
    II(a,b,c,d,x[ 0], 6,0xf4292244); II(d,a,b,c,x[ 7],10,0x432aff97);
    II(c,d,a,b,x[14],15,0xab9423a7); II(b,c,d,a,x[ 5],21,0xfc93a039);
    II(a,b,c,d,x[12], 6,0x655b59c3); II(d,a,b,c,x[ 3],10,0x8f0ccc92);
    II(c,d,a,b,x[10],15,0xffeff47d); II(b,c,d,a,x[ 1],21,0x85845dd1);
    II(a,b,c,d,x[ 8], 6,0x6fa87e4f); II(d,a,b,c,x[15],10,0xfe2ce6e0);
    II(c,d,a,b,x[ 6],15,0xa3014314); II(b,c,d,a,x[13],21,0x4e0811a1);
    II(a,b,c,d,x[ 4], 6,0xf7537e82); II(d,a,b,c,x[11],10,0xbd3af235);
    II(c,d,a,b,x[ 2],15,0x2ad7d2bb); II(b,c,d,a,x[ 9],21,0xeb86d391);
    state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
}

static void MD5Update(MD5_CTX *ctx, unsigned char *input, unsigned int ilen) {
    unsigned int idx=(ctx->count[0]>>3)&0x3F, i, partLen=64-idx;
    ctx->count[0]+=(ilen<<3);
    if(ctx->count[0]<(ilen<<3)) ctx->count[1]++;
    ctx->count[1]+=(ilen>>29);
    if(ilen>=partLen){
        memcpy(&ctx->buffer[idx], input, partLen);
        MD5Transform(ctx->state, ctx->buffer);
        for(i=partLen; i+63<ilen; i+=64)
            MD5Transform(ctx->state, &input[i]);
        idx=0;
    } else i=0;
    memcpy(&ctx->buffer[idx], &input[i], ilen-i);
}

static void MD5Final(MD5_CTX *ctx, unsigned char digest[16]) {
    unsigned char bits[8];
    int idx, padLen;
    static unsigned int PADDING = 0x80;
    for (int i=0;i<8;i++) bits[i]=(unsigned char)(ctx->count[0]>>(i*8));
    idx=(ctx->count[0]>>3)&0x3F; padLen=idx<56?56-idx:120-idx;
    MD5Update(ctx, (unsigned char*)&PADDING, padLen);
    MD5Update(ctx, bits, 8);
    for (int i=0;i<4;i++) {
        digest[i*4]  =(unsigned char)(ctx->state[i]&0xff);
        digest[i*4+1]=(unsigned char)((ctx->state[i]>>8)&0xff);
        digest[i*4+2]=(unsigned char)((ctx->state[i]>>16)&0xff);
        digest[i*4+3]=(unsigned char)((ctx->state[i]>>24)&0xff);
    }
}

void md5_hash(const char *input, char output[33]) {
    MD5_CTX ctx={{0,0},{0x67452301,0xefcdab89,0x98badcfe,0x10325476},{0}};
    MD5Update(&ctx, (unsigned char *)input, (unsigned int)strlen(input));
    unsigned char digest[16];
    MD5Final(&ctx, digest);
    for (int i=0;i<16;i++) sprintf(output+i*2, "%02x", digest[i]);
    output[32]='\0';
}

/* ============================================================
 *           订单号生成
 *           格式：WL + YYYYMMDD + 6 位序号
 *           序号每日从 1 开始递增
 * ============================================================ */
/* order sequence persistence: read from file, survive restart */
void init_order_sequence() {
    FILE *fp = fopen("data/order_seq.dat", "r");
    if (!fp) { order_sequence = 0; return; }
    char date_buf[16] = {0};
    int seq = 0;
    if (fscanf(fp, "%s %d", date_buf, &seq) == 2) {
        char today[16];
        get_current_date_str(today);
        if (strcmp(date_buf, today) == 0)
            order_sequence = seq;
        else
            order_sequence = 0;
    }
    fclose(fp);
}

static void save_order_sequence() {
    char today[16];
    get_current_date_str(today);
    FILE *fp = fopen("data/order_seq.dat", "w");
    if (fp) {
        fprintf(fp, "%s %d", today, order_sequence);
        fclose(fp);
    }
}

void generate_order_id(char *buf) {
	char date[16];
    //获取当前日期字符串
    get_current_date_str(date);
	//全局订单序号递增
	order_sequence++;
	// 生成订单号 
	sprintf(buf,"WL%s%06d",date,order_sequence);
	save_order_sequence();
}

/* ============================================================
 *           链表通用操作
 * ============================================================ */

/* 计算链表节点数 */
int list_count(const void *head, size_t next_offset) {
	int count =0;
	const char *p=(const char*)head;
	while(p){
		count++;
		p=*(const char**)(p+next_offset);
	}
	return count;
}

/* ============================================================
 *           用户管理
 * ============================================================ */

void init_sample_users() {
    char hash[33];

    User *admin = (User *)malloc(sizeof(User));
    admin->id = ++user_id_counter;
    strcpy(admin->name, "admin");
    md5_hash("admin123", hash);  strcpy(admin->password, hash);
    admin->role = ROLE_ADMIN;
    admin->failed_attempts = 0;  admin->lockout_until = 0;
    admin->next = user_list_head;  user_list_head = admin;

    User *svc = (User *)malloc(sizeof(User));
    svc->id = ++user_id_counter;
    strcpy(svc->name, "service");
    md5_hash("123456", hash);  strcpy(svc->password, hash);
    svc->role = ROLE_SERVICE;
    svc->failed_attempts = 0;  svc->lockout_until = 0;
    svc->next = user_list_head;  user_list_head = svc;

    User *wh = (User *)malloc(sizeof(User));
    wh->id = ++user_id_counter;
    strcpy(wh->name, "warehouse");
    md5_hash("123456", hash);  strcpy(wh->password, hash);
    wh->role = ROLE_WAREHOUSE;
    wh->failed_attempts = 0;  wh->lockout_until = 0;
    wh->next = user_list_head;  user_list_head = wh;

    User *dp = (User *)malloc(sizeof(User));
    dp->id = ++user_id_counter;
    strcpy(dp->name, "dispatcher");
    md5_hash("123456", hash);  strcpy(dp->password, hash);
    dp->role = ROLE_DISPATCHER;
    dp->failed_attempts = 0;  dp->lockout_until = 0;
    dp->next = user_list_head;  user_list_head = dp;

    User *u1 = (User *)malloc(sizeof(User));
    u1->id = ++user_id_counter;
    strcpy(u1->name, "user1");
    md5_hash("123456", hash);  strcpy(u1->password, hash);
    u1->role = ROLE_CUSTOMER;
    u1->failed_attempts = 0;  u1->lockout_until = 0;
    u1->next = user_list_head;  user_list_head = u1;
}

User *find_user_by_name(const char *name) {
    User *p = user_list_head;
    while (p) {
        if (strcmp(p->name, name) == 0) return p;
        p = p->next;
    }
    return NULL;
}

int add_user(const char *name, const char *pwd, UserRole role) {
    if (find_user_by_name(name)) return -1;          /* 用户名已存在 */
    if (validate_password(pwd) != 0) return -2;       /* 密码不合法 */
    User *u = (User *)malloc(sizeof(User));
    if (!u) return -3;
    u->id = ++user_id_counter;
    strncpy(u->name, name, NAME_LEN - 1);
    u->name[NAME_LEN - 1] = '\0';
    char hash[33];
    md5_hash(pwd, hash);
    strncpy(u->password, hash, PASSWORD_LEN - 1);
    u->password[PASSWORD_LEN - 1] = '\0';
    u->role = role;
    u->failed_attempts = 0;  u->lockout_until = 0;
    u->next = user_list_head;
    user_list_head = u;
    return 0;
}

void reset_password(User *u) {
    if (u) {
        char hash[33];
        md5_hash("888888", hash);
        strcpy(u->password, hash);
        u->failed_attempts = 0;
        u->lockout_until = 0;
    }
}

/* ============================================================
 *           分页显示（控制台）
 *           head:       链表头指针
 *           next_offset: next 字段在结构体中的偏移量
 *           print_row:  回调函数，打印单条记录
 *           page_size:  每页显示的记录数
 *           返回：总记录数
 * ============================================================ */
int show_paginated_list(const void *head,
                        size_t next_offset,
                        PrintRowFn print_row,
                        int page_size) {
    int total = list_count(head, next_offset);
    if (total == 0) {
        printf("暂无数据，按任意键返回...\n");
        _getch();
        return 0;
    }

    /* 收集所有记录指针，支持随机翻页 */
    const void **records = (const void **)malloc(total * sizeof(void *));
    if (!records) return 0;
    const char *p = (const char *)head;
    for (int i = 0; i < total; i++) {
        records[i] = p;
        p = *(const char **)(p + next_offset);
    }

    int cur = 0;
    int pages = (total + page_size - 1) / page_size;
    while (1) {
        system("cls");
        int start = cur * page_size;
        int end = (start + page_size < total) ? start + page_size : total;

        printf("===== 记录列表  第 %d/%d 页  共 %d 条 =====\n", cur + 1, pages, total);
        for (int i = start; i < end; i++)
            print_row(records[i], i + 1);
        printf("===== 按上下方向键翻页，Esc 退出 =====\n");

        int ch = _getch();
        if (ch == 27) break;
        if (ch == -32) {
            ch = _getch();
            if (ch == 72 && cur > 0) cur--;           /* 上键 */
            if (ch == 80 && cur < pages - 1) cur++;   /* 下键 */
        }
    }

    free(records);
    return total;
}

/* ============================================================
 *      通用二进制文件读写（所有模块复用）
 *		每个bat文件的前四个字节存储记录数，后续是连续的记录数据
 *      bat文件格式：
 *        [int  count]      记录总数
 *        [byte record_1]   记录 1（record_size 字节）
 *        [byte record_2]   ...
 *        ...
 *
 *      约定：每个结构体第一个字段为 struct Xxx *next，
 *            调用时传入 offsetof(StructType, next) 作为 next_offset。
 *            由于 next 永远在偏移 0，也可直接传 0。
 * ============================================================ */

/* 将链表保存到二进制文件，返回写入的记录数，失败返回 -1 */
int bin_save_list(const char *filename,
                  const void *head,
                  size_t record_size,
                  size_t next_offset) {
    FILE *fp=fopen(filename,"wb");//二进制写入 
    if(!fp) return -1;
    
    /*第一遍：计数*/
	int count =0;
	const char *p=(const char *)head;//从头开始
	while(p){
		count ++;
		p=*(const char **)(p+next_offset);//通过便宜获取next指针
	}

	/*写入记录数*/
	fwrite(&count,sizeof(int),1,fp);
	/*第二遍：逐条写入*/
	p=(const char *)head;
	while(p){
		fwrite(p,record_size,1,fp);
		p=*(const char **)(p+next_offset);//通过便宜获取next指针
	} 
	fclose(fp);
	return count;
}

/* 从二进制文件加载链表，返回读取的记录数，失败返回 -1
 * head:        输出参数，指向新链表的头指针
 * record_size: 每条记录的字节数
 * next_offset: next 指针字段在结构体中的偏移量（0）  */
int bin_load_list(const char *filename,
                  void **head,
                  size_t record_size,
                  size_t next_offset) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;

    /* 读取记录数 */
    int count = 0;
    if (fread(&count, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    *head = NULL;
    void *tail = NULL;

    for (int i = 0; i < count; i++) {
        void *node = malloc(record_size);
        if (!node) {
            fclose(fp);
            return i;
        }
        if (fread(node, record_size, 1, fp) != 1) {
            free(node);
            fclose(fp);
            return i;
        }

        /* 覆盖从文件中读到的垃圾指针，重建链表 */
        *(void **)((char *)node + next_offset) = NULL;

        if (!*head) {
            *head = node;
        } else {
            *(void **)((char *)tail + next_offset) = node;
        }
        tail = node;
    }

    fclose(fp);
    return count;
}

/* ============================================================
 *           角色与状态字符串转换
 *           实现各类枚举到字符的转换 
 * ============================================================ */

const char *role_to_string(UserRole role) {
    switch (role) {
        case ROLE_ADMIN:      return "管理员";
        case ROLE_SERVICE:    return "客服";
        case ROLE_WAREHOUSE:  return "仓储员";
        case ROLE_DISPATCHER: return "调度员";
        case ROLE_CUSTOMER:   return "客户";
        default:              return "未知";
    }
   
}

const char *order_status_to_string(OrderStatus status) {
    switch (status) {
        case ORDER_PENDING_REVIEW: return "待审核";
        case ORDER_REJECTED:       return "已驳回";
        case ORDER_PENDING_OUT:    return "待出库";
        case ORDER_IN_TRANSIT:     return "运输中";
        case ORDER_DELIVERED:      return "已送达";
        case ORDER_COMPLETED:      return "已完成";
        default:                   return "未知";
    }
}

const char *vehicle_status_to_string(VehicleStatus status) {
    switch (status) {
        case VEHICLE_IDLE:        return "空闲";
        case VEHICLE_IN_TRANSIT:  return "运输中";
        case VEHICLE_MAINTENANCE: return "维修中";
        default:                  return "未知";
    }
}

const char *driver_status_to_string(DriverStatus status) {
    switch (status) {
        case DRIVER_ON_DUTY:  return "在岗";
        case DRIVER_ON_LEAVE: return "休假";
        default:              return "未知";
    }
}

const char *dispatch_status_to_string(DispatchStatus status) {
    switch (status) {
        case DISPATCH_PENDING:   return "未执行";
        case DISPATCH_DEPARTED:  return "已出发";
        case DISPATCH_TRANSIT:   return "中转";
        case DISPATCH_DELIVERED: return "已送达";
        case DISPATCH_ABNORMAL:  return "异常";
        default:                 return "未知";
    }
}

/* 返回角色对应的权限位掩码 */
const char *goods_type_to_string(int type) {
    switch (type) {
        case GOODS_NORMAL:     return "普通";
        case GOODS_FRAGILE:    return "易碎";
        case GOODS_COLD_CHAIN: return "冷链";
        case GOODS_DANGEROUS:  return "危险品";
        default:               return "未知";
    }
}


int role_get_permissions(UserRole role) {
    switch (role) {
        case ROLE_ADMIN:      return ROLE_PERM_ADMIN;
        case ROLE_SERVICE:    return ROLE_PERM_SERVICE;
        case ROLE_WAREHOUSE:  return ROLE_PERM_WAREHOUSE;
        case ROLE_DISPATCHER: return ROLE_PERM_DISPATCHER;
        case ROLE_CUSTOMER:   return ROLE_PERM_CUSTOMER;
        default:              return 0;
    }
}
