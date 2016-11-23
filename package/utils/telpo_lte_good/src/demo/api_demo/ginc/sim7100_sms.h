#ifndef SIM7100_SMS
#define SIM7100_SMS

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define SMS_NUM_LEN_MAX     16      //电话号码最大长度16位  
#define DISABLE 0 
#define ENABLE 1
#define FALSE 0
#define TRUE	1

//短信发送日期,使用字符格式  
typedef struct  
{  
    u8  Year;       //年20xx年  
    u8  Month;      //月  
    u8  Day;        //日  
    u8  Hour;       //小时  
    u8  Minute;     //分钟  
    u8  Second;     //秒  
    u8  Reserve1;   //保留  
    u8  Reserve2;   //保留  
}SMS_TIMER ;  

// 短消息参数结构，编码/解码共用 
// 其中，字符串以0结尾 
typedef struct { 
    char SCA[16];       // 短消息服务中心号码(SMSC地址) 
    char TPA[16];       // 目标号码或回复号码(TP-DA或TP-RA) 
    char TP_PID;        // 用户信息协议标识(TP-PID)  例如:点对点
    char TP_DCS;        // 用户信息编码方式(TP-DCS) 	00 04 08
    char TP_SCTS[16];   // 服务时间戳字符串(TP_SCTS), 接收时用到 
    char TP_UD[161];    // 原始用户信息(编码前或解码后的TP-UD) 
    char index;         // 短消息序号，在读取时用到 
    char message_type;  //消息类型，收的、发的 等
} SM_PARAM; 

typedef struct  
{  
    SMS_TIMER    Timer;                      //短信发送的时间     
    char  NumBuff[SMS_NUM_LEN_MAX];   //电话号码缓冲区,使用的是字符模式                
    u8     NumLen;                     //电话号码长度  
    u8     SMS_Size;                   //短信有效内容长度,最大140B  
    u8     TEXT_MODE;                  // 1:短信为TEXT模式;0:短信为PDU模式  
    u8     PDU;                        //PDU数据,用于区分是否有短信头部信息  
    u8     DSC;                        //DSC数据,用于区分是否为字符模式(0),PDU模式(0X08)  
    u8     AllNum;                     //当前短信总分割数  
    u8     PreNum;                     //当前位置  
    u8     IndexNum;                 //当前索引位置1-50  
    u8	tp_ud[161];
    u8	message_type;  			//消息类型，收的、发的 等	
}sms_info ;  

#define GtoU    "/etc/config/GtoU.sys"       //GBK 转 UCICODE 编码表位置  
#define UtoG    "/etc/config/UtoG.sys"       //UCICODE 转 GBK 编码表位置  

FILE *GtoU_File;            //GtoU 文件工作区  
FILE *UtoG_File;            //UtoG 文件工作区  

//电话号码结构  
#define PHONE_NUMBER_MAX_LEN    24-2        //电话号码最大长度  
typedef struct  
{  
    u8      PhoneNumLen;        //电话号码长度  
    char    PhoneNumBuff[PHONE_NUMBER_MAX_LEN + 1];//电话号码缓冲区,电话号码前面的2位为地区编号,中国为86,打电话需要跳过前面的2位数字  
}PHONE_NUMBER;  

static PHONE_NUMBER SMSServeNumber;                                 //全局短信中心号码  

#endif