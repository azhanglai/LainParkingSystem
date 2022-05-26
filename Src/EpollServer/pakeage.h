#ifndef __DEFINE_FILE_H
#define __DEFINE_FILE_H

#define MAX_SIZE        1024    // 最大长度

// 数据包类型的宏定义
#define ERROR_NUMBER    -1      // 错误信息
#define REGIST_TYPE     1       // 用户注册包 
#define LOGIN_TYPE      2       // 用户登录包
#define PHOTO_TYPE      3       // 图片包
#define CAR_GETIN       4       // 汽车入场包
#define CAR_GETOUT      5       // 汽车出场包
#define HEART_TYPE      6       // 连接心跳包
#define CAR_MSG_TYPE    7       // 汽车信息包
#define VIDEO_TYPE      8       // 视频记录包
#define VIDEO_OPEN      9       // 视频打开
#define VIDEO_CLOSE     10      // 视频关闭
#define OFFLINE_TYPE    11      // 用户下线包

/* 消息队列结构体 */
typedef struct {
    long mtype;
    char mtext[12];
} MYBUF;

/* 心跳包结构体 */
typedef struct {
    int fd;				
    int time;
} PACK_HEART;

/* 数据包头 */
typedef struct {
    int bodyType;		// 数据包类型
    int bodySize;		// 数据包大小
    int crcCode;		// 校验码
    char seqNum[36];	// 流水号
} PACK_HEAD;

/* 登录注册包体 */
typedef struct {
    char name[24];		// 用户账号
    char pwd[16];		// 用户密码
} PACK_REGIST_LOGIN;

/* 登录注册反馈包体 */
typedef struct {
    int result;			// 0：成功，1：用户名错误，2：密码错误
} PACK_RL_BACK;

/* 汽车入场包 */
typedef struct {
    int number;				// 序号
    char car_num[18];		// 车牌号
    char now_time[36];		// 当前时间
    char location[12];		// 位置
    char photo_path[100];	// 图片路径
} PACK_ENTER;

/* 图片包 */
typedef struct {
    char filename[40];		// 文件名
    int realSize;			// 实际大小
    int num;				// 包的序号
    int sum;				// 包的总数
    char context[MAX_SIZE];	// 存储数据的数组
} PACK_PHOTO;

/* 图片反馈 */
typedef struct {
    int result;				// 1：收到了，继续发 0：没收到
} PACK_PHOTO_BACK;

/* 汽车出场包 */
typedef struct {
    char car_num[18];		// 车牌号
    char out_time[36];		// 出场时间
} PACK_EXIT;

/* 汽车出场反馈 */
typedef struct {
    char in_time[36];		// 入场时间
    int vip;				// 是否VIP
    char total_time[64];	// 停车时长
    int money;				// 停车金额
} PACK_EXIT_BACK;

/* 汽车信息包 */
typedef struct mycarmsg {
    char car_num[18];		// 车牌号
    char in_time[36];		// 入场时间
    char out_time[36];		// 出场时间
    int page;				// 第几页
} PACK_CARMSG;

/* 汽车信息反馈 */
typedef struct {
    char car_num[18];		// 车牌号
    char in_time[36];		// 入场时间
    char out_time[36];		// 出场时间
    int money;				// 停车金额
} PACK_CARMSG_BACK;

/* 全部汽车信息反馈 */
typedef struct {
    int realCount;			// 实际的数据条数
    int totalCount;			// 总条数
    PACK_CARMSG_BACK arr[4];// 汽车信息反馈数组
} PACK_ALLCARMSG_BACK;

/* 视频包 */
typedef struct {
    char video_name[24];	// 视频名称 
    int frameCount;			// 记录帧数
    int totalFrameCount;	// 总帧数
} PACK_VIDEO;

/* 视频反馈 */
typedef struct {
    int frameCount;			// 记录帧数
} PACK_VIDEO_BACK;

#endif /* __DEFINE_FILE_H */

