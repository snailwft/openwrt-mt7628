#ifndef	_AT_
#define	_AT_

#ifndef _DBUG_ 
#define _DBUG_
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>    
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>
#include "api_demo.h"
#include <setjmp.h>
//#include <stdbool.h>
//#include "si_voice_datatypes.h"

#define GSM_RX_MAXBUFF       4096
#define MAX_GSM         4
#define FALSE -1
#define TRUE 1
#define GSM_RX_OVERTIM       1000    //50*10=500ms
#define GSM_CARR_OVERTIM  600  // 6S
#define GSM_HANG_OVERTIM  4000 // 10s
#define SIM_ALSA_STATUS_TIME		4000
#define NET_STATUS_TIME		6000
#define SIM_CSQ_TIME				3000
#define SIM_NETWORK_TIME   5000
#define uchar unsigned char
#define  DEFAULT_PINCODE          "1234"  
#define  DEFAULT_LOCK_PINCODE     "5678"

#define VOLUME_SYS_FILE	 		"/etc/config/gainset"
#define DIGITAL_MAP					"/etc/config/dialset"
#define MISCELLANEOUS_FILE 	"/etc/config/miscellaneous"
#define TONE_DC_RING_FILE	 	"/etc/config/regional"
#define SMS_CONFIG_FILE			"/etc/config/smscnf"
#define DIAL_LIST					 		"/usr/log/dialerlist"
#define RECV_LIST						"/usr/log/recvlist"
#define MISS_LIST						"/usr/log/misslist" 
#define DBUG_LOG						"/usr/log/test.log"
#define LOCKCARD_FILE				"/usr/log/lockcard"
#define SHORT_SMS_FILE			"/usr/log/smsfile"
#define AT_ORDER_FILE				"/etc/config/atorderfile"
#define NETWORK_FILE				"/etc/config/network"
#define CALL_COUNT	10
#define START_TIME		1
#define END_TIME			0
#define WATCHDOG_ENABLE 1
#define WATCHDOG_LOW 2
#define WATCHDOG_HIGET 3
#define AT_TIMES	3
#define SMS_IDLE 0
#define SMS_SEND 1
#define SMS_DELETE 2

#define DTMF_DECODE
//#undef DTMF_DECODE
#define SMS_ENABLE
#undef SMS_ENABLE
#define SIM7600TP
#undef SIM7600TP

#define GSM_CFG
#undef GSM_CFG

typedef enum
{
	AT_INIT = 0,
	AT,	
	ATI,
	AT_CSQ,
	AT_STTONE,
	AT_CNMI,
	ATH,
	ATA,
	AT_VTS,
	AT_CLVL,
	AT_CFUN,
	AT_ECHO,
	AT_CMIC,
	AT_CREG,
	AT_CCLK,
	AT_CLIP,
	AT_CMGF,
	AT_CMGS,
	AT_CSCA,
	AT_CSCA_Q,
	AT_CMGD,
	AT_CMGR,
	AT_CMGL,
	AT_CBAND,
	AT_CPMS,
	AT_CHFA,
	ATE0,												// 25
	AT_COLP,
	AT_CSMINS,
	AT_CIPSTART,
	AT_CIPHEAD,
	AT_CIPSEND,
	AT_CIPSHUT,
	AT_CIPCLOSE,
	AT_CSTT,
	AT_CGSN,
	AT_CLCK,
	AT_CPWD,
	AT_CLCK_R,
	AT_CPIN_R,
	AT_CPIN,
	AT_CIMI,
	ATD,
	AT_W,
	AT_SMS,
	AT_CPAMP,
	ATS0,
	ATE1,
	AT_CPCMREG0,
	AT_CPCMREG,
	AT_CPCMREGOVER,
	AT_CVHU,
	AT_CHUP,
	AT_CODECSEL,
	AT_CLCC,
	AT_CNMP,
	AT_MONI,
	AT_QCRMCALL,
	AT_QCRMCALL1,
	AT_QCRMCALL0,	
	AT_CSUB,
	AT_CSQFMT,
	AT_COPS,
	AT_CMGW,
	AT_CMSS,
	AT_CNSMOD,
	AT_CGMM,
}AT_ORDER_ID;

typedef struct
{
	AT_ORDER_ID	str_id;
	char        *order;
}AT_ORDER;

//sim卡状态
typedef enum GSM_MAINSTATE_TAG
{
	HS_NONEED_SIMCARD_QUERY = 0,
	HS_NONEED_SIMCARD_QUERY_WAIT,
	HS_LOCKCARD_DETECT,
	HS_LOCKCARD_DETECT_WAIT,  
	HS_SEM_WAIT,
	HS_AT_INIT,
	HS_AT_INIT_WAIT,
	HS_AT_INIT_ERROR,
	HS_NORMAL,
	HS_NETWORK_SEEK,
	HS_SIMCOM_ALSA_SEEK,
	HS_SIMCOM_CSQ_SEEK,
	HS_NETWORK_MODE_SEEK,
	HS_DIAG_UP,
	HS_DIAG_UP_WAIT,
	HS_DIAG_DOWN,
	HS_DIAG_DOWN_WAIT,
	HS_SIMCOM_SMS_UPDATE,
	HS_SIMCOM_SMS_CMGW_WAIT,
	HS_SIMCOM_SMS_CMGD_WAIT,
	HS_SIMCOM_SMS_SEND_WAIT,
	HS_SIMCOM_SMS_SEND,
	HS_NOSIM,
	HS_VOLUME,
	HS_VOLUME_WAIT,
	HS_SIM_INSERTED,
	HS_CHECK_SIMCARD_LOCK,
	HS_NO_SERVICE,
	HS_READY_SEARCH,
	HS_SEARCHING,
	HS_CHANGESIM,
	HS_SEEKING_EMER,
	HS_EMER_LOCKED,
	HS_NO_SERVICE_LOCKED,
	HS_SEARCHING_LOCKED,
	HS_AUTO_LOCKCARD,        //锁卡
	HS_AUTO_LOCKCARD_ENABLE, //锁卡使能
	HS_AUTO_LOCKCARD_DISABLE,
	HS_AUTO_LOCKCARD_MODIFY, //修改卡PINCODE
	HS_AUTO_LOCKCARD_ERROR,  //锁卡错误
	HS_AUTO_LOCKTERMINAL,       //锁机
	HS_AUTO_LOCKTERMINAL_ERROR, //锁机错误提示
	HS_AUTO_LOCKCARD_WAITING, //锁卡等待
	HS_AUTO_LOCKTERMINAL_WAITING,//锁机等待
}GSM_MAINSTATE;

typedef enum GSM_SUBSTATE_TAG
{
	GSM_SUBSTATE_IDLE = 0,
	GSM_SUBSTATE_DIALING,	// 拨号
	GSM_SUBSTATE_CALLING_CONNECT,
	GSM_SUBSTATE_CALLING_DISCONNECT,
	GSM_SUBSTATE_CALLING,	// 
	GSM_SUBSTATE_RING, // 振铃来电
	GSM_SUBSTATE_HANG_UP, //对方挂机
	GSM_SUBSTATE_RING_ANSWER, // 振铃回应
	GSM_SUBSTATE_SEND_SMS,// 发送短信
	GSM_SUBSTATE_ANSWERING,// 正在通话
}GSM_SUBSTATE;

typedef struct
{
	int   	tx_overtime;                //超时
	int	net_status_time;
	int 	sim_status_time;
	int	sim_csq_time;
	int	sim_network_time;
	int	carr_overtime;
	int	hang_overtime;
	uchar          init_step;                  	//初始化步骤
	GSM_MAINSTATE  main;                  //GSM主状态
	GSM_SUBSTATE   sub;						//GSM次状态
}ST_GSM_STATE;

typedef enum 
{
	AT_IDLE = 0,        			//空闲
	AT_WAIT_SEND,       		//等待发送
	AT_SENDING,         			//正在发送
	AT_WAIT_ANSWER,     	//等待应答
	AT_ANSWER_OK,       		//应答正确
	AT_ATD_ANSWER_OK,		// 拨号应答正常
	AT_ATD_CONNECT_OK,	//拨号对方已接听
	AT_ATD_ANSWER_ERROR,	//拨号失败
	AT_ERROR,
	AT_ANSWER_LOCKSIM,  //锁卡
	AT_ANSWER_UNLOCKSIM,//不需要锁卡
	AT_ANSWER_PUK,	    	//PUK
	AT_ANSWER_IMSI,			//IMSI
	AT_ANSWER_ERROR,    	//无人应答
	AT_NOCAR,           			//断线
	AT_CONNECT_OK,			//线路接通应答
}AT_STATE;


typedef struct
{
	AT_STATE       state;
	AT_ORDER_ID	   order;
	char           tx_data[30];
}ST_AT_ORDER;

typedef struct
{
	int          rx_length;                  //AT接收的长度
	char       rx_data[GSM_RX_MAXBUFF];    //AT接收的缓冲
}ST_AT_RX;

typedef struct
{
	uchar	com_state;
	uchar	callin_nu[14];     //呼入来显
	uchar	imei[16];
	uchar	cellid[10][10];		
	uchar 	cell_num;
	uchar	at_times;
	int	sms_num;
	ST_AT_ORDER    at;                        //AT发送状态与缓冲 
	ST_AT_RX       rx;				// 接收缓冲区
}ST_GSM_AT;

typedef struct
{
	uchar  terminal;                          //锁机标志
	uchar  card;                              //锁卡标志
	uchar 	pinmodify;
	char   imsi[15];                          //imsi码,锁机时使用
	char   checksum;                      //校验和
}ST_LOCK_SIM;

struct calltime
{
	int tm_sec;  //代表目前秒数, 正常范围为0-59, 但允许至61 秒
    int tm_min;  //代表目前分数, 范围0-59
    int tm_hour;  //从午夜算起的时数, 范围为0-23
    int tm_mday;  //目前月份的日数, 范围01-31
    int tm_mon;  //代表目前月份, 从一月算起, 范围从0-11
    int tm_year;  //从1900 年算起至今的年数
    int tm_wday;  //一星期的日数, 从星期一算起, 范围为0-6
    int tm_yday;  //从今年1 月1 日算起至今的天数, 范围为0-365
    int tm_isdst;  //日光节约时间的旗标
};

typedef struct
{
	uchar	num_list;
	uchar 	call_number[14];
	struct calltime start_time;
	struct calltime end_time;
	int 		talk_time;
}call_log;

call_log  dial_list,recv_call_list,miss_call_list;

int fd, fd3, fd4, fd5;
int gpio_fd;
volatile int  fd2;
FILE *fp,*atorder_file;
//uchar audiobuf[320];
char *audiobuf;
char rcv_buf[1024];
char pcm_flag;
char answer_begin;
char slic_pcm_read_open;
char *pattern;
char patternx[1024];

int sms_update;
int sms_operation;
int map_enable;			// 数图使能
int unmatch_undial;	// 不匹配不拨号
int net_enable;			// 拨号上网开关
int net_flag;
int dialing_key;			// 快拨键
int call_antipole; 		// 通话反极
int dial_up_timeout; 	// 拨号超时
int call_id_enable;		// 来电显示开关
int call_id_mode;		// 来电显示模式
int jitter_delay;			// 最小抖动延时
int max_jitter_delay;	// 最大抖动延时

int answer_call;
int miss_call;
int watchdog_state;
jmp_buf env;
int nums;

//pthread_mutex_t gsm_recv_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t audio_mutex;
pthread_mutex_t cid_mutex;
pthread_mutex_t dtmf_mutex;
pthread_mutex_t atorder_mutex;
pthread_mutex_t send_atorder_mutex;
pthread_mutex_t pcm_mutex;
pthread_mutex_t pcm_recv_mutex;
pthread_mutex_t pcm_send_mutex;
sem_t	gsm_sem;
sem_t	gsm_pcm_recv, gsm_pcm_send, pcm_record, gsm_pcm_sem, gsm_pcm_sem2;

void at_fixed_format(AT_ORDER_ID order);
void gsm_init_and_sim_active();
void send_at_order();
void gsm_rx_query_handler();
void  set_gsm_sub_state(GSM_SUBSTATE state );
GSM_SUBSTATE get_gsm_sub_state();
GSM_MAINSTATE get_gsm_mainstate();
void gsm_pcm_answer(AT_ORDER_ID order);
void set_gsm_at_state( AT_STATE state );
void clear_at_rx_buff();
void at_swicth();
void *gsm_send_handler(void *arg);
void *gsm_init_handler(void * arg);
void *gsm_recv_handler(void *arg);
void *gsm_audio_recv(void *arg);
void dialing(char *number);
void time_handler(int k);

ST_GSM_STATE  st_gsm_state;
ST_GSM_AT     	st_gsm_at;
ST_LOCK_SIM		st_lock_sim;

#endif
