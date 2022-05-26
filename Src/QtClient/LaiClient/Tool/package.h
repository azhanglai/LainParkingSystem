#ifndef PACKAGE_H
#define PACKAGE_H

#define ERROR_ NUMBER -1    // 错误信息
#define REGISTER 1          // 注册包
#define LOGIN 2             // 登录包
#define PHOTO_TYPE 3        // 图片包
#define CAR_GETIN 4         // 入场包
#define CAR_GETOUT 5        // 出场包
#define HEART_TYPE 6        // 心跳包
#define CAR_MSG_TYPE 7      // 车辆信息包
#define VIDEO_TYPE 8        // 视频记录包
#define MAX_SIZE 1024

// 包头
typedef struct{
    int bodyType;       // 包体类型
    int bodySize;       // 包体大小
    int crc_code;       // crc校验码
    char serialNum[36]; // 流水号
} PACK_HEAD;

// 登录结构体
typedef struct {
    char name[24];
    char pwd[16];
} PACK_REGIST_LOGIN;

// 返回结果结构体
typedef struct {
    int result;     //0成功 1用户名错误 2密码错误
} REGIST_LOGIN_RESULT;

// 图片包
typedef struct {
    char filename[40];  // 文件名
    int realSize;       // 真实的字符串大小
    int num;            // 表示是第几个包
    int sum;            // 总共有几个包
    char context[MAX_SIZE]; // 用来存储数据的数组
} PACK_PHOTO;

// 图片结果包
typedef struct {
    int result;     // 1表示收到了 继续发，0表示没收到
} PACK_PHOTO_BACK;

// 入场包
typedef struct {
    int number;             // 序号
    char car_num[18];       // 车牌
    char now_time[36];      // 时间
    char location[12];      // 位置
    char photo_path[100];   // 图片路径
} PACK_ENTER;

// 出场包
typedef struct {
    char car_number[18];    // 车牌
    char out_time[36];      // 出场时间
} PACK_EXIT;

// 出场返回包
typedef struct {
    char in_time[36];       // 入场时间
    int vip;                // 是否为VIP
    char total_time[64];    // 停车时长
    int money;              // 金额
} PACK_EXIT_BACK;

// 车辆信息包
typedef struct {
    char car_number[18];    // 车牌
    char in_time[36];       // 入场时间
    char out_time[36];      // 出场时间
    int page;               // 第几页
} PACK_CARMSG;

// 车辆信息返回包
typedef struct {
    char car_number[18];    // 车牌
    char in_time[36];       // 入场时间
    char out_time[36];      // 出场时间
    int money;              // 停车金额
} PACK_CARMSG_BACK;

// 车辆信息返回包
typedef struct {
    int realCount;          // 真实几条数据
    int totalCount;         // 总条数
    PACK_CARMSG_BACK arr[4];// 四个数组结构体
} PACK_ALLCARMSG_BACK;


// 视频包
typedef struct {
    char video_name[24];    // 视频名称
    int frameCount;         // 记录帧数
    int totalFrameCount;    // 总帧数
} PACK_VIDEO;

// 视频返回包
typedef struct {
    int frameCount;         // 记录帧数
} PACK_VIDEO_BACK;


#endif // PACKAGE_H
