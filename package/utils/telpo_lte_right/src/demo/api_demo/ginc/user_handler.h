#ifndef _USER_
#define _USER_
#include "gsm.h"

#define uint     unsigned short
#define UINT8    unsigned char
#define uchar  unsigned char
#define MAX_DTMF_BUFF 30
#define START_DIALING_TIME   3000   //3000*10=30s
#define STOP_DTMF_TIME		500 
#define BUSY_DIALING_TIME   3000   //300*10ms=3S
#define HURRY_DIALING_TIME   500
#define LIGHT_UP 1
#define LIGHT_DOWN 0
#define LIGHT_BLINK_SLOW 2 
#define LIGHT_BLINK_FAST	3
#define ATH_TIME_BEGIN		500000
#define FSK		0
#define DTMF 1
#define DISABLE 0
#define ENABLE 1
#define CALL_TIMES	20


int pcm_start_flag;
int on_hook_wait;
int open_flag;
int slic_light;
int interupt_time;
int spiread_error_times;
int calltimes;

#define _DBUGUSER_
//#undef _DBUGUSER_

#define OPEN_FLAG
//#undef OPEN_FLAG


typedef enum MAIN_STATE_TAG
{
	USER_STATE_INIT = 0,
	USER_STATE_IDLE,         //空闲    
	USER_STATE_RING,         //来电
	USER_STATE_OFFHOOK, 	 //摘机    
	USER_STATE_ONHOOK,		 //挂机   
	USER_STATE_ONHOOK_WAIT,
}MAIN_STATE;

typedef enum SUB_STATE_TAG
{
	SUB_IDLE = 0,           		//空闲    
	SUB_RXDTMF_READY, 	    	//准备接收DTMF 
	SUB_RXDTMF_END,         	//DTMF接收完成
	SUB_DIALING_READY,		//准备拨号
	SUB_DIALING_READY_END,	//停拔号音，只有在GSM时需要
	SUB_DIALING,            //拔号
	SUB_SPEED_DIALING,
	SUB_CALLING,            //等待接通
	SUB_CALLING_WAIT,
	SUB_CALL_ATH_WAIT,
	SUB_CALLING_OVER,
	SUB_ANSWERING,
	SUB_PHONE_END,
	SUB_WAITONHOOK,
	SUB_WAITONHOOK_ATH,
	SUB_HURRYONHOOK,
	SUB_RINGSTOP,
	SUB_ATA_WAIT,
	SUB_ATH,
	SUB_ATH_WAIT,
	SUB_ATH_ONHOOK_WAIT,
	SUB_PCM_START,
	SUB_PCM_START_WAIT,
	SUB_PCM_CALL_ATH_WAIT,
	SUB_PCM_CALLING_OVER,
	SUB_PCM_STOP,
	SUB_PCM_STOP_WAIT,
	SUB_CALL_END,
	SUB_VTS_WAIT,
	SUB_ERROR_EXIT,         //错误提示
}SUB_STATE;

typedef struct
{	
	 MAIN_STATE   main;
	 SUB_STATE    sub;
	 uint         dialing_delaytime;  //启动拔号延时
	 uint			  dtmfing_delaytime;
	 uint			  busy_delaytime;
	 uint				hurry_delaytime;
	 uchar        channel;
	 uchar        last_rxdtmf_length; //上次接收DTMF的长度
	 uchar        Empty;
}ST_USER_STATE;

ST_USER_STATE  st_user_state;

typedef struct
{
	uchar  rx_addr;                        //接收指针
	char   dtmf_buff[MAX_DTMF_BUFF];       //接收缓冲
	char	  ascii_buff[MAX_DTMF_BUFF];
}ST_DTMF_RXBUFF;
ST_DTMF_RXBUFF    st_dtmf_rx_buff;

void *user_hanlder(void *arg);
void *slic_lamp_handler(void *arg);
void user_init();
void dial_delay_time();
uchar  get_dtmf_rx_buff_length();
void user_off_hook();

#endif
