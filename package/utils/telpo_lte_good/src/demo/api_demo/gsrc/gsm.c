#include "gsm.h"
#include "dbug_log.h"
#include "sim7100_sms.h"

//GSM初始化配置列表
AT_ORDER_ID gsm_init_list[] =
{
	ATS0,  // 必须要插卡
	AT_CLIP,
	AT_CLCC,
#ifdef SMS_ENABLE
	AT_CMGF,	
	AT_CPMS,
	AT_CNMI,
	AT_CMGL,	
#endif
	//AT_CSQFMT,  	// 可以不要卡
	//AT_CGSN,			// 可以不要卡
	AT_CIMI,			// 必须要卡  写入到文件
	AT_COPS,			// 必须要卡  写入到文件
	//AT_CNSMOD,
	//AT_CSUB,			// 可以不要卡
	//AT_CSQ,			// 可以不要卡
	AT_CVHU,
};

AT_ORDER_ID gsm_noneed_simcard_query[] = 
{
	ATE1,
	AT_CGMM,
#ifndef SIM7600TP
	AT_CODECSEL,
#endif
	AT_CSQFMT,  	// 可以不要卡
	AT_CGSN,			// 可以不要卡   写入到文件
	AT_CSUB,			// 可以不要卡   写入到文件
	AT_CSQ,			// 可以不要卡		写入到文件
	//AT_CPIN_R,		// 可以不要卡   写入到文件
};

const AT_ORDER at_order_table[] =
{	
	AT_INIT,             "",
	AT,	                 "AT\r",                   //AT\CR\CR\LFOK\CR\LF   
	ATI,							"ATI\r",
	AT_CSQ,              "AT+CSQ\r",               //信号强度: 范围0~32  AT+CSQ\CR\CR\LF+CSQ: 13,0\CR\LF\CR\LFOK\CR\LF
	AT_STTONE,           "AT+STTONE=",   // 振铃音设置
	AT_CNMI,             "AT+CNMI=2,1,0,0\r\n",    //新短信查询
	ATH,                 "ATH\r",                  //挂机
	ATA,                 "ATA\r",                  //呼入应答
	AT_VTS,              "AT+VTS=",                //产生DTMF音
	AT_CLVL,             "AT+CLVL=",          //话筒音量
	AT_CFUN,             "AT+CFUN=0\r",            //关机
	AT_ECHO,             "AT+ECHO=32767,100,5,0\r",//ECHO控制
	AT_CMIC,             "AT+CMIC=0,8\r",          //MIC音量控制
	AT_CREG,             "AT+CREG?\r",             //有没有信号
	AT_CCLK,             "AT+CCLK\r",              //时钟查询
	AT_CLIP,             "AT+CLIP=1\r",            //主叫号码识别
	AT_CMGF,             "AT+CMGF=0\r",
	AT_CMGS,             "AT+CMGS=\"",             //发短信
	AT_CSCA,			 	"AT+CSCA=\"",			   //设置短信中心号	
	AT_CSCA_Q,           "AT+CSCA?\r",             //查询短信中心号
	AT_CMGD,             "AT+CMGD=",               //删短信
	AT_CMGR,             "AT+CMGR=",               //读短信
	//AT_CMGL,             "AT+CMGL=\"ALL\"\r\n",          //列举短信TEXT
	AT_CMGL,             "AT+CMGL=4\r\n",          //列举短信 PDU
	AT_CBAND,            "AT+CBAND?\r",            //查询当前频段
	AT_CPMS,             "AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n",               //对短信操作
	AT_CHFA,             "AT+CHFA=0\r",            //语音通道切换,1为MIC,0为手柄
	ATE0,                	"ATE0\r",                 			//取消回显
	AT_COLP,             "AT+COLP=1\r",            //呼出接通回显
	AT_CSMINS,           "AT+CSMINS?\r",           //SIM卡安装状态
	AT_CIPSTART,         "AT+CIPSTART",            //GPRS
	AT_CIPHEAD,          "AT+CIPHEAD",             //GPRS
	AT_CIPSEND,          "AT+CIPSEND",             //GPRS
	AT_CIPSHUT,          "AT+CIPSHUT",             //GPRS
	AT_CIPCLOSE,         "AT+CIPCLOSE",            //Close TCP/UDP
	AT_CSTT,             "AT+CSTT",                //用户密码
	AT_CGSN,             "AT+CGSN\r",              //读模块序列号IMEI码
	AT_CLCK,             "AT+CLCK=\"SC\",",        //锁机锁卡AT+CLCK="SC",1/0,Pincode;  1使能，0不使能 
    AT_CPWD,             "AT+CPWD=\"SC\",\"",      //修改Pincode
	AT_CLCK_R,			 "AT+CLCK=\"SC\",2\r", 	   //检测卡是否使能，+CLCK: 1/0
	AT_CPIN_R,           "AT+CPIN?\r",             //+CPIN: READY或+CPIN: SIM PUK
	AT_CPIN,			 "AT+CPIN=",			   //sim PIN CODE or PUK
    AT_CIMI,             "AT+CIMI\r",              //IMSI码
	ATD,                 "ATD",                    //格式为ATD112;\r,112为号码
	AT_W,                "AT&W\r",				// 保存数据
	AT_SMS,              "",	                                //"ON CAR"被叫挂机  
    AT_CPAMP,			 "AT+CPAMP=0\r",
    ATS0,					"ATS0=000\r",                        // 禁止自动接听
    ATE1,					"ATE1\r",											// 使能回显
    AT_CPCMREG0,		"AT+CPCMREG?\r",					//查询sim模块alsa守护进程状态
    AT_CPCMREG,			"AT+CPCMREG=1\r",						// 使能接收pcm数据
    AT_CPCMREGOVER,	"AT+CPCMREG=0\r",				// 使用完之后要关闭pcm
    AT_CVHU,					"AT+CVHU=0\r",
    AT_CHUP,					"AT+CHUP\r",
    AT_CODECSEL,			"AT+CODECSEL=0\r",
    AT_CLCC,					"AT+CLCC=1\r",
    AT_CNMP,					"AT+CNMP=",
    AT_MONI,					"AT+MONI?\r",
    AT_QCRMCALL,			"AT$QCRMCALL?\r",
    AT_QCRMCALL1,		"AT$QCRMCALL=1,1\r",
    AT_QCRMCALL0,		"AT$QCRMCALL=0,1\r",
    AT_CSUB,					"AT+CSUB\r",					//查询sim模块固件版本
    AT_CSQFMT,				"AT+CSQFMT=1\r",
    AT_COPS,					"AT+COPS?\r",
    AT_CMGW,				"AT+CMGW=",
    AT_CMSS,			    	"AT+CMSS=",
    AT_CNSMOD,			"AT+CNSMOD?\r",
    AT_CGMM,				"AT+CGMM\r",
};

int simcom_enable (SiVoiceChanType_ptr pChan)
{
    /*
    ** assert reset, wait 250ms, release reset, wait 250ms
    */
    //pChan->deviceId->ctrlInterface->Reset_fptr(pChan->deviceId->ctrlInterface->hCtrl,4); // 高电平
    ioctl(gpio_fd, 4);
    return RC_NONE;
}

int simcom_disable (SiVoiceChanType_ptr pChan)
{
    /*
    ** assert reset, wait 250ms, release reset, wait 250ms
    */
    //pChan->deviceId->ctrlInterface->Reset_fptr(pChan->deviceId->ctrlInterface->hCtrl,5); // 低电平
    ioctl(gpio_fd, 5);
    return RC_NONE;
}

void ttyusb_exist(chanState ports)
{
	char ttybuf[20] = {0};
	int i = 0;
	if (access("/dev/ttyUSB0", F_OK) == -1) // 不存在
	{
		if (access("/dev/ttyUSB2", F_OK) != -1)
		{
			remove("/dev/ttyUSB2");
		}
	}
	for (i = 0; i < 6; i++)
	{
		sprintf(ttybuf, "/dev/ttyUSB%d", i);
		ttybuf[strlen(ttybuf)] = '\0';
		if (access(ttybuf, F_OK) == -1) //检测文件是否存在，如果不存在就等待
		{
			perror("");
			simcom_enable(ports.ProObj); //使能4G 模块
			sleep(25);
			break;
		}
	}
}

void gsm_delay_time()
{
	if (st_gsm_state.tx_overtime > 0)
	{
		st_gsm_state.tx_overtime--;
	}
	if (st_gsm_state.net_status_time > 0)
	{
		st_gsm_state.net_status_time--;
	}
	if (st_gsm_state.sim_status_time > 0)
	{
		st_gsm_state.sim_status_time--;
	}
	if (st_gsm_state.sim_csq_time > 0)
	{
		st_gsm_state.sim_csq_time--;
	}
	if (st_gsm_state.sim_network_time > 0 )
	{
		st_gsm_state.sim_network_time--;
	}
}

void gsm_cmgw()
{
	char *number = "13016054035";
	char buf[100];
	//获取目的号码
	//AT+CMGW="13016054035" <CR> (TEXT MODE)   PDU:AT+CMGW=长度(短信长度)
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data, 0x00, sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CMGW;
		//sprintf(buf, "\"%s\" <CR> (TEXT MODE)", number);
		sprintf(buf, "%d", 1); // PDU
		memcpy(st_gsm_at.at.tx_data, buf, strlen(buf));
		memcpy(&st_gsm_at.at.tx_data[strlen(buf)], (char *)"\r", strlen("\r"));
	}
}

void gsm_cmgd()
{
	int num = 26;
	char buf[4];
	//获取删除的短信标号
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data, 0x00, sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CMGD;
		sprintf(buf, "%d", num);
		memcpy(st_gsm_at.at.tx_data, buf, strlen(buf));
		memcpy(&st_gsm_at.at.tx_data[strlen(buf)], (char *)"\r", strlen("\r"));
	}
}

int send_one_message(char *pdu)
{
	int ret = 0;
	
	do{
		ret = uart_send(fd, pdu, strlen(pdu));		
	}while (ret <= 0);
	
	return ret;
}

int fill_short_sms()
{
	int ret = 0;
	//获取发送的内容
	char smsbuf[256] = "hello world";
	smsbuf[strlen(smsbuf)] = 0x1a;
	do{
		ret = uart_send(fd, smsbuf, strlen(smsbuf));		
	}while (ret <= 0);
	
	return 0;
}

int send_sms()
{
	char buf[5];
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data, 0x00, sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CMSS;
		sprintf(buf, "%d", st_gsm_at.sms_num);
		memcpy(st_gsm_at.at.tx_data, buf, strlen(buf));
		memcpy(&st_gsm_at.at.tx_data[strlen(buf)], (char *)"\r", strlen("\r"));
	}
}

void gsm_on_hook()
{
#ifdef _DBUG_
	printf("gsm_on_hook\n");
#endif
	at_fixed_format(ATH);
}

void play_congestion_tone()
{
	memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
	st_gsm_at.at.state = AT_WAIT_SEND;
	st_gsm_at.at.order = AT_STTONE;
	memcpy(st_gsm_at.at.tx_data,(char *)"1,3,100000\r",strlen("1,3,100000\r"));
}

AT_STATE get_gsm_at_state()
{
	return (st_gsm_at.at.state);
}

void set_gsm_at_state( AT_STATE state )
{
	st_gsm_at.at.state = state;
}
void gsm_callin_answer(AT_ORDER_ID order)
{
	at_fixed_format(order); 
}

void gsm_pcm_answer(AT_ORDER_ID order)
{
	at_fixed_format(order);
	
}

void set_gsm_atorder(AT_ORDER_ID atorder)
{
	st_gsm_at.at.order = atorder;
}

void gsm_pcm_over(AT_ORDER_ID order)
{
	at_fixed_format(order);
	pcm_flag = 0;
}

void clear_at_rx_buff()
{
	memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));
}

void gsm_init()
{
	pcm_flag = 0;
	answer_call = 0;
	miss_call = 0;
	answer_begin = 0;
	net_flag = 0;
	sms_update = 0;
	sms_operation = 0;
	memset(&st_gsm_at,0x00,sizeof(st_gsm_at));
	memset(&st_gsm_state,0x00,sizeof(st_gsm_state));	
	memset(&st_lock_sim,0x00,sizeof(st_lock_sim));
	
	atorder_file = fopen(AT_ORDER_FILE, "w+");// 可读可写，清空文件
	if (!atorder_file) 
	{
		fprintf(stderr, "Unable to create file '%s'\n", AT_ORDER_FILE);
		exit(1); // 进程退出
	}
}

void at_fixed_format(AT_ORDER_ID order)
{
	if (st_gsm_at.at.state == AT_IDLE)
	{
#ifdef _DBUG_
		printf("hello ...\n");
#endif
		memset(st_gsm_at.at.tx_data, 0x00, sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = order;  // 这条指令还没进行
	}
}

void dtmf_tone_dialing(char *number)
{
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_VTS;
		memcpy(st_gsm_at.at.tx_data,number,strlen(number));
		memcpy(&st_gsm_at.at.tx_data[strlen(number)],(char *)";\r",strlen(";\r"));
	}
}

void dialing(char *number)
{
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = ATD;
		memcpy(st_gsm_at.at.tx_data,number,strlen(number));
		memcpy(&st_gsm_at.at.tx_data[strlen(number)],(char *)";\r",strlen(";\r"));
	}
}
void get_sim_imei()
{
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = ATI;
	}
}

void query_gsm_cell()
{
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_MONI;
	}
}

void switch_to_gsm(char *mode)
{
	char temp[10];
	if (st_gsm_at.at.state == AT_IDLE)
	{
		memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CNMP;
		sprintf(temp,"%s%s",mode,"\r");
		memcpy(&st_gsm_at.at.tx_data,temp,strlen(temp));
	}
}

/*****************************************************************
函数：void input_puk_code(char *puk , char *newpin)
功能：通过PUK 修改PIN CODE
参数：at+cpin== 12345678 ,1234
注意：
设计：
时间：
修改：
******************************************************************/
void input_puk_code(char *puk , char *newpin)
{
	char temp[20] = {0};
	memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
	if (st_gsm_at.at.state == AT_IDLE)
	{
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CPIN;
		sprintf(temp,"%s,%s%s",puk,newpin,"\r");
		memcpy(st_gsm_at.at.tx_data,temp,strlen(temp));
	}
}

/*****************************************************************
函数：void modify_pin_code( uchar  channel,char *oldpassw ,char *newpassw )
功能：修改PIN CODE
参数：at++cpwd=="SC","5151","1234"
注意：修改前必须使能PIN
设计：
时间：
修改：
******************************************************************/
void modify_pin_code(char *oldpassw ,char *newpassw )
{
	char temp[20] = {0};
	memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
	if (st_gsm_at.at.state == AT_IDLE)
	{
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CPWD;
		sprintf(temp,"%s%s,%s%s%s",oldpassw,"\"","\"",newpassw,"\"\r");
		memcpy(st_gsm_at.at.tx_data,temp,strlen(temp));
	}
}

/*****************************************************************
函数：void set_sim_card( uchar  channel,char *passw ,uchar flag )
功能：SIM card 使能或不使能,
参数：flag:1表示使能，0表示不使能 passw:PIN CODE
注意：at++clck=="SC",0,"1234"
设计：
时间：
修改：
******************************************************************/
void set_sim_card(char *passw ,uchar flag)
{
	char temp[20] = {0};
	memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
	if (st_gsm_at.at.state == AT_IDLE)
	{
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CLCK;
		sprintf(temp,"%d%s%s%s",flag,",\"",passw,"\"\r");// "SC",1,1234
		memcpy(st_gsm_at.at.tx_data,temp,strlen(temp));
	}
}

/*****************************************************************
函数：void input_pin_code( uchar  channel,char *passw )
功能：输入PIN CODE
参数：at+cpin=82520634,1234
注意：
设计：
时间：
修改：
******************************************************************/
void input_pin_code(char *passw)
{
	char temp[50] = {0};
	memset(st_gsm_at.at.tx_data,0x00,sizeof(st_gsm_at.at.tx_data));
	if (st_gsm_at.at.state == AT_IDLE)
	{
		st_gsm_at.at.state = AT_WAIT_SEND;
		st_gsm_at.at.order = AT_CPIN;
		sprintf(temp,"%s%s",passw,"\r");
		memcpy(st_gsm_at.at.tx_data,temp,strlen(temp));
	}
}

/*****************************************************************
函数：void query_sim_card(uchar channel)
功能：查询SIM卡是否安装
******************************************************************/

void query_sim_card()
{
	at_fixed_format(AT_CPIN_R);
}

/*上网状态查询*/
void network_status()
{
	at_fixed_format(AT_CPIN_R);
}

void gsm_init_and_sim_active(chanState *ports)
{
	switch (st_gsm_state.main)
	{		   
		case HS_NONEED_SIMCARD_QUERY:
			pthread_mutex_lock(&atorder_mutex);
			st_gsm_at.at.state = AT_IDLE;
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM; // 10s的定时
			at_fixed_format(gsm_noneed_simcard_query[st_gsm_state.init_step]);	//检测sim卡是否安装和是否需要解锁
			st_gsm_state.main = HS_NONEED_SIMCARD_QUERY_WAIT;
			break;
		case HS_NONEED_SIMCARD_QUERY_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.init_step++;
				if ( st_gsm_state.init_step == sizeof(gsm_noneed_simcard_query)/sizeof(int))
				{
					//sem_post(&gsm_sem);
					//fclose(atorder_file);
					st_gsm_state.init_step = 0;
					st_gsm_state.main = HS_LOCKCARD_DETECT; //HS_LOCKCARD_DETECT
				}
				else
				{
					st_gsm_state.main = HS_NONEED_SIMCARD_QUERY;
				}
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR ||st_gsm_state.tx_overtime == 0)
			{
				//st_gsm_state.main= HS_AT_INIT_ERROR;
				//pthread_mutex_unlock(&atorder_mutex);
				goto error_exit;
			}
		break;
#if 1
		case HS_LOCKCARD_DETECT: 		//锁卡检测
			pthread_mutex_lock(&atorder_mutex);
			st_gsm_at.at.state = AT_IDLE;
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM; // 10s的定时
			at_fixed_format(AT_CPIN_R);	//检测sim卡是否安装和是否需要解锁
			st_gsm_state.main = HS_LOCKCARD_DETECT_WAIT;
			break;
		case HS_LOCKCARD_DETECT_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_UNLOCKSIM)
			{ 
				st_gsm_state.main = HS_AT_INIT;//HS_AT_INIT;HS_VOLUME
				pthread_mutex_unlock(&atorder_mutex);
#if 0
				if (st_lock_sim.card==1)
				{
					st_gsm_state.main=HS_AUTO_LOCKCARD_ENABLE;
				}
				else
				{
					st_gsm_state.main = HS_AT_INIT;//
				}
#endif
			}
			else if (st_gsm_at.at.state == AT_ANSWER_LOCKSIM)
			{		
				st_gsm_state.main = HS_LOCKCARD_DETECT;//	继续检测				
				pthread_mutex_unlock(&atorder_mutex);
				sleep(3);   // 等待一下再去检测
			}
			else if (st_gsm_at.at.state == AT_ERROR)
			{
				st_gsm_state.main = HS_NOSIM;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0) //没有返回值
			{
				goto error_exit;
			}
#endif
#if 0
			else if (st_gsm_state.tx_overtime == 0)
			{
				st_gsm_state.main = HS_LOCKCARD_DETECT;//	继续检测	
				pthread_mutex_unlock(&atorder_mutex);
			}
#endif
			break;
		case HS_NOSIM:
			break;
#if 0
		case HS_AUTO_LOCKCARD:
			if (st_gsm_state.tx_overtime==0)
			{
				st_gsm_at.at.state = AT_IDLE;
				input_pin_code((char *)DEFAULT_PINCODE); 
			}
			if (st_gsm_at.at.state==AT_ANSWER_OK)
			{
				if (st_lock_sim.card==1)
				{
					st_gsm_state.main=HS_AT_INIT;
				}
				else
				{
					st_gsm_state.main = HS_AUTO_LOCKCARD_DISABLE;//
				}
			}
			else if (st_gsm_at.at.state==AT_ANSWER_ERROR)
			{
#ifdef _DBUG_
				printf("pin code error\n");
				dbug_log(__FILE__,__LINE__,"PIN CODE ERROR\n");
#endif
				st_gsm_state.main = HS_AUTO_LOCKCARD_ERROR;
			}
			break;
		case HS_AUTO_LOCKCARD_DISABLE:
			if (st_gsm_state.tx_overtime==0)
			{
				st_gsm_at.at.state=AT_IDLE;
				set_sim_card(DEFAULT_PINCODE,0);
			}
			if (st_gsm_at.at.state ==AT_ANSWER_OK)
			{				
				st_gsm_state.main=HS_AT_INIT;
			}
			else if (st_gsm_at.at.state ==AT_ERROR)
			{
#ifdef _DBUG_
				printf("pin code error\n");
				dbug_log(__FILE__,__LINE__,"PIN CODE ERROR\n");
#endif
				st_gsm_state.main = HS_AUTO_LOCKCARD_ERROR;
			}
			break;
		case HS_AUTO_LOCKCARD_ENABLE:
			if (st_gsm_state.tx_overtime==0)
			{
				st_gsm_at.at.state=AT_IDLE;
				set_sim_card(DEFAULT_PINCODE,1);
			}
			if (st_gsm_at.at.state ==AT_ANSWER_OK)
			{
				if (st_lock_sim.pinmodify==1)
				{
					st_gsm_state.main=HS_AUTO_LOCKCARD_MODIFY;
				}
			}
			else if (st_gsm_at.at.state ==AT_ERROR)
			{
#ifdef _DBUG_
				printf("pin code error\n");
				dbug_log(__FILE__,__LINE__,"PIN CODE ERROR\n");
#endif
				st_gsm_state.main = HS_AUTO_LOCKCARD_ERROR;
			}
			break;
		case HS_AUTO_LOCKCARD_MODIFY:
			if (st_gsm_state.tx_overtime==0)
			{
				st_gsm_at.at.state=AT_IDLE;
				modify_pin_code((char *)DEFAULT_PINCODE,(char *)DEFAULT_LOCK_PINCODE);
			}
			if (st_gsm_at.at.state ==AT_ANSWER_OK)
			{
				st_gsm_at.at.state = AT_IDLE;
				st_gsm_state.main = HS_AT_INIT;

			}
			else if (st_gsm_at.at.state ==AT_ERROR)
			{
#ifdef _DBUG_
				printf("new pin code error\n");
				dbug_log(__FILE__,__LINE__,"NEW PIN CODE ERROR\n");
#endif
				st_gsm_state.main = HS_AUTO_LOCKCARD_ERROR;
			}
			break;
		case HS_AUTO_LOCKCARD_ERROR:
			break;
#endif
		case HS_SEM_WAIT:
			//sem_wait(&gsm_sem);
			st_gsm_state.main = HS_LOCKCARD_DETECT;// HS_AT_INIT;//HS_LOCKCARD_DETECT
			break;
		case HS_AT_INIT:								//初始化		
			pthread_mutex_lock(&atorder_mutex);
			st_gsm_at.at.state = AT_IDLE;
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;	
			at_fixed_format(gsm_init_list[st_gsm_state.init_step]);
			st_gsm_state.main = HS_AT_INIT_WAIT;			
			break;
		case HS_AT_INIT_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.init_step++;
				if (st_gsm_state.init_step == sizeof(gsm_init_list)/sizeof(int))
				{
					//sem_post(&gsm_sem);
					//fclose(atorder_file);
					st_gsm_state.main = HS_VOLUME; //HS_LOCKCARD_DETECT
				}
				else
				{
					st_gsm_state.main = HS_AT_INIT;
				}
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR ||st_gsm_state.tx_overtime == 0)
			{
				//st_gsm_state.main= HS_AT_INIT_ERROR;
				//pthread_mutex_unlock(&atorder_mutex);
				goto error_exit;
			}
#if 0
			else if (st_gsm_state.tx_overtime==0)
			{
				st_gsm_state.main = HS_AT_INIT;
				pthread_mutex_unlock(&atorder_mutex);
			}
#endif
			break;	
		case HS_AT_INIT_ERROR:
			break;
#if 0
		case HS_NOSIM:			// 检查SIM卡是否安装
			if ( st_gsm_state.tx_overtime == 0 )	
			{
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				query_sim_card();			
			}
			if (st_gsm_at.at.state == AT_ANSWER_OK )    // 有插卡
			{
				st_gsm_at.at.state = AT_IDLE;
				st_gsm_state.main = HS_VOLUME;//HS_VOLUME

			}
			else
			{
				//继续检测是否有SIM卡 延长检测时间
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			}
			break;
#endif
		case HS_VOLUME:
			pthread_mutex_lock(&atorder_mutex);
			st_gsm_at.at.state = AT_IDLE;// 使空闲状态
			st_gsm_at.at_times = 0;
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			volume_adjust();			
			st_gsm_state.main = HS_VOLUME_WAIT;
			break;
		case HS_VOLUME_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)    // 
			{
				//st_gsm_state.net_status_time = 6000; //定时1分钟，检测网络状态
				st_gsm_at.at_times = 0;
				st_gsm_state.sim_status_time = SIM_ALSA_STATUS_TIME; //定时30秒，检测sim alsa守护进程状态
				st_gsm_state.sim_csq_time = SIM_CSQ_TIME;
				st_gsm_state.main = HS_NORMAL;		 
				pthread_mutex_unlock(&atorder_mutex);
			}		
			else if (st_gsm_at.at.state == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				volume_adjust();	
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case HS_NORMAL:			// 常态
			//break;
#ifdef _DBUG_
			//printf("gsm_init_and_sim_active exit\n");
#endif
			//pthread_exit(NULL);
			//simcom_reset(ports[0].ProObj); //simcom 模块复位
			//break;
#if 1		
			if (st_gsm_state.net_status_time == 0 && !net_flag)	
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				at_fixed_format(AT_QCRMCALL); //查询网络状态
				st_gsm_state.net_status_time = NET_STATUS_TIME; //定时1分钟，检测网络状态
				st_gsm_state.main = HS_NETWORK_SEEK;  
			}
			else if (st_gsm_state.sim_status_time == 0)
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				at_fixed_format(AT_CPCMREG0); //检测sim alsa守护进程状态
				st_gsm_state.sim_status_time = SIM_ALSA_STATUS_TIME; //定时30秒，检测sim alsa守护进程状态
				st_gsm_state.main = HS_SIMCOM_ALSA_SEEK;
			}
			else if (st_gsm_state.sim_csq_time == 0)
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				at_fixed_format(AT_CSQ); //检测sim alsa守护进程状态
				st_gsm_state.sim_csq_time = SIM_CSQ_TIME; //定时30秒，检测sim alsa守护进程状态
				st_gsm_state.main = HS_SIMCOM_CSQ_SEEK;
			}
			else if (st_gsm_state.sim_network_time == 0)
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				at_fixed_format(AT_CNSMOD); //检测sim alsa守护进程状态
				st_gsm_state.sim_network_time = SIM_NETWORK_TIME; //定时50秒，检测sim alsa守护进程状态
				st_gsm_state.main = HS_NETWORK_MODE_SEEK;
			}
#ifdef SMS_ENABLE
			else if (sms_update == 1) //更新收件箱
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				at_fixed_format(AT_CMGL);
				st_gsm_state.main = HS_SIMCOM_SMS_UPDATE;
			}
			else if (sms_operation == SMS_SEND) // 发送短信
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_cmgw();
				st_gsm_state.main = HS_SIMCOM_SMS_CMGW_WAIT;
			}
			else if (sms_operation == SMS_DELETE) //删除短信
			{
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at_times = 0;
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_cmgd();
				st_gsm_state.main = HS_SIMCOM_SMS_CMGD_WAIT;
			}
#endif
			break;
#ifdef SMS_ENABLE
		case HS_SIMCOM_SMS_CMGW_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{ 
				set_gsm_sub_state(GSM_SUBSTATE_SEND_SMS);
				set_gsm_atorder(AT_INIT);
				set_gsm_at_state(AT_IDLE);
				//fill_short_sms();  // text mode 
				//char psms[] = "你好吗hello 你";
				/*
				获取短信内容，获取目的号码
				*/
				char psms[] = "hello";			
				printf("psms:%s\n", psms);
				GSM_SendSMS(psms, "8613016054035");
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				st_gsm_state.main = HS_SIMCOM_SMS_SEND_WAIT;
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case HS_SIMCOM_SMS_CMGD_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)    // 
			{				
				sms_operation = SMS_IDLE;
				st_gsm_at.at_times = 0;
				st_gsm_state.main = HS_NORMAL;		 
				pthread_mutex_unlock(&atorder_mutex);
			}		
			else if (st_gsm_at.at.state == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case HS_SIMCOM_SMS_SEND_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{ 
				st_gsm_at.at.state = AT_IDLE;// 使空闲状态
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				send_sms();
				st_gsm_state.main = HS_SIMCOM_SMS_SEND;
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case HS_SIMCOM_SMS_SEND:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{ 
				st_gsm_state.main = HS_NORMAL; 
				sms_operation = SMS_IDLE;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case HS_SIMCOM_SMS_UPDATE:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{ 
				st_gsm_state.main = HS_NORMAL; 
				sms_update = 0;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
#endif
		case HS_NETWORK_SEEK:
			if (st_gsm_at.at.state == AT_ANSWER_OK && net_enable == 1) //拨号成功,并且需要上网
			{ 
				st_gsm_state.main = HS_NORMAL; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ANSWER_OK && net_enable == 0) //拨号成功，但是不需要上网
			{
				st_gsm_state.main = HS_DIAG_DOWN; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR && net_enable == 1) //拨号失败，需要上网
			{
				st_gsm_state.main = HS_DIAG_UP; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR && net_enable == 0) //拨号失败，但不需要上网
			{
				printf("st_gsm_at.at.state == AT_ERROR && net_enable == 0\n");
				st_gsm_state.main = HS_NORMAL; 
				net_flag = 1;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;		
		case HS_DIAG_UP:
			pthread_mutex_lock(&atorder_mutex);
			st_gsm_at.at.state = AT_IDLE;// 使空闲状态
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			at_fixed_format(AT_QCRMCALL1); //拨号上网
			st_gsm_state.main = HS_DIAG_UP_WAIT;  
			break;
		case HS_DIAG_UP_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.main = HS_NORMAL; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR ||st_gsm_state.tx_overtime == 0)
			{
				st_gsm_state.main = HS_DIAG_UP; 
				pthread_mutex_unlock(&atorder_mutex);
			}
#if 0
			else if (st_gsm_state.tx_overtime == 0)
			{
				//goto error_exit;
			}
#endif
			break;
		case HS_DIAG_DOWN:
			pthread_mutex_lock(&atorder_mutex);
			st_gsm_at.at.state = AT_IDLE;// 使空闲状态
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			at_fixed_format(AT_QCRMCALL0); // 禁止拨号上网
			st_gsm_state.main = HS_DIAG_DOWN_WAIT;  
			break;
		case HS_DIAG_DOWN_WAIT:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.main = HS_NORMAL; 
				net_flag = 1;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR || st_gsm_state.tx_overtime == 0)
			{
				st_gsm_state.main = HS_DIAG_DOWN; 
				pthread_mutex_unlock(&atorder_mutex);
			}
#if 0
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
#endif
			break;
		case HS_SIMCOM_ALSA_SEEK:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.main = HS_NORMAL; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_at.at.state == AT_ERROR ||st_gsm_state.tx_overtime == 0)
			{
				//st_gsm_state.main = HS_NORMAL; 
				goto error_exit;		
			}
			break;
		case HS_SIMCOM_CSQ_SEEK:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.main = HS_NORMAL; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				//st_gsm_state.main = HS_NORMAL; 
				goto error_exit;		
			}
			break;
		case HS_NETWORK_MODE_SEEK:
			if (st_gsm_at.at.state == AT_ANSWER_OK)
			{
				st_gsm_state.main = HS_NORMAL; 
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				//st_gsm_state.main = HS_NORMAL; 
				goto error_exit;		
			}
			break;
#endif
		default:
			break;
	}
	
	pthread_mutex_lock(&send_atorder_mutex);
	send_at_order();
	pthread_mutex_unlock(&send_atorder_mutex);
	return ;
error_exit:
	
	pthread_mutex_unlock(&atorder_mutex);
	printf("DBG: %s: software running error need restart\n", __FUNCTION__);
	simcom_disable(ports->ProObj); //模块直接断电
	ProSLIC_ShutdownChannel(ports->ProObj);
	SiVoice_Reset(ports->ProObj);
	ProSLIC_destroyChannel(&(ports->ProObj));	
	exit(1);


}

void send_at_order()
{
	char	temp[50] = {0};
	uint	length;
	if (st_gsm_at.at.state == AT_WAIT_SEND)
	{
		length = strlen(at_order_table[st_gsm_at.at.order].order);		
#ifdef _DBUG_
		printf("at_order_table[st_gsm_at.at.order].order=%s\n",at_order_table[st_gsm_at.at.order].order);		
#endif
		dbug_log(__FILE__,__LINE__,"at_order_table[st_gsm_at.at.order].order=%s\n",at_order_table[st_gsm_at.at.order].order);
		memcpy(temp, at_order_table[st_gsm_at.at.order].order, length);
		memcpy(&temp[length], st_gsm_at.at.tx_data, strlen(st_gsm_at.at.tx_data));
		st_gsm_at.at.state = AT_WAIT_ANSWER;
		uart_send(fd, temp, strlen(temp));		
	}
	return ;
}

void *gsm_init_handler(void * arg)
{
	chanState *ports = (chanState *)arg;
	while (1)
	{
		usleep(10000);
		gsm_init_and_sim_active(ports);		
	}	
	pthread_exit(NULL);
}

int uart_pcm_send(int fd, char *send_buf,int data_len)
{
    int ret;  
    ret = write(fd,send_buf,data_len);
    if (ret == data_len)
    {	
    	return ret;
    } 
    else 
    {    
    	tcflush(fd,TCOFLUSH);    
		return FALSE;
    }
}

int uart_send2(int fd, char *send_buf,int data_len)
{
	int ret,fs_sel;
    fd_set fs_write;  
    struct timeval time;   
   	FD_ZERO(&fs_write);
    FD_SET(fd,&fs_write);   
    time.tv_sec = 0;
    time.tv_usec = 0;	// 200ms

	//使用select实现串口的非阻塞
    fs_sel = select(fd+1,NULL, &fs_write, NULL, &time);
    if (fs_sel)
    {
    	if (FD_ISSET(fd, &fs_write))
    	{
			ret = write(fd, send_buf, data_len);
			return ret;
    	}
    }
	else
	{
		return FALSE;
	}	
    
}

int uart_send(int fd, char *send_buf,int data_len)
{
	int ret,fs_sel;
    fd_set fs_write;  
   	FD_ZERO(&fs_write);
    FD_SET(fd, &fs_write);   
	struct timeval time;  
	time.tv_sec = 0;
	time.tv_usec = 0;
	
	 //使用select实现串口的非阻塞
    fs_sel = select(fd + 1,NULL, &fs_write, NULL, &time);
    if (fs_sel)
    {
    	if (FD_ISSET(fd, &fs_write))
    	{
			ret = write(fd, send_buf, data_len);
	    	if (ret == data_len )
	    	{
	    		return ret;
	    	} 
	    	else 
	    	{    
	    		tcflush(fd,TCOFLUSH);    
				return ret;
	    	}	
    	}
    }
	else
	{
		return FALSE;
	}	
}

int uart_rev(int fd, char *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;  
    struct timeval time;   
   	FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);   
    time.tv_sec = 0;
    time.tv_usec = 0;

	//使用select实现串口的多路通信
    fs_sel = select(fd + 1,&fs_read,NULL,NULL,&time);
    if (fs_sel)
    {
    	if (FD_ISSET(fd, &fs_read))
		{
			len = read(fd,rcv_buf,data_len);
			if (len == data_len)
			{
				return len;
			}
			else
			{
				//tcflush(fd,TCIFLUSH); 
			}
			return len;
    	}
    }	
	return FALSE;
}

int gsm_recv()
{
	int res = 0;	
	int i = 0;
	
	res = uart_rev(fd, st_gsm_at.rx.rx_data, sizeof(st_gsm_at.rx.rx_data));
	if (res > 0)
	{
		st_gsm_at.rx.rx_length = res;		
#ifdef _DBUG_
		printf("st_gsm_at.rx.rx_data = %s\n", st_gsm_at.rx.rx_data);		
#endif	
		dbug_log(__FILE__,__LINE__, "st_gsm_at.rx.rx_data = %s\n", st_gsm_at.rx.rx_data);
	}
	return res;
}

void  set_gsm_sub_state(GSM_SUBSTATE state )
{
	st_gsm_state.sub = state;
}

GSM_MAINSTATE get_gsm_mainstate()
{
	return (st_gsm_state.main);
}

GSM_SUBSTATE get_gsm_sub_state()
{
	return (st_gsm_state.sub);
}

void write_cmgl_information(const char *filename, const char *buf, int size)
{
	FILE *cmgl_short_sms;
	cmgl_short_sms = fopen(filename, "w+");
	fwrite(buf, 1, size, cmgl_short_sms);
	fclose(cmgl_short_sms);	
}

void save_sms_message(const char *filename,sms_info *psms, int size)
{
	int i = 0;
	char buf[512];
	FILE *sms_file = NULL;
	sms_file = fopen(filename, "w+");
	for(i = 0; i < size; i++)
	{
		sprintf(buf, "%d,%d,	%s,	20%d/%d/%d/%d:%d\r\n%s\r\n", psms[i].IndexNum, psms[i].message_type, psms[i].NumBuff, 
			psms[i].Timer.Year, psms[i].Timer.Month, psms[i].Timer.Day, psms[i].Timer.Hour, psms[i].Timer.Minute, psms[i].tp_ud);
		fwrite(buf, strlen(buf), 1, sms_file);		
	}
	fclose(sms_file);
}

int parse_pdu_sms(char *smsptr, sms_info *pmsg)
{
	int sms_num = 0;
	while ((smsptr = strstr(smsptr, "+CMGL:")) != NULL) 
	{ 
	    smsptr += 6;        														// 跳过"+CMGL:" 			         	
	    pmsg[sms_num].IndexNum = atoi(smsptr);			       
	    smsptr += 3;		       
	    pmsg[sms_num].message_type = atoi(smsptr);			        
	    smsptr = strstr(smsptr, "\r\n");    								// 找下一行 
	    smsptr += 2;        														// 跳过"\r\n" 
		gsm_parsepdu_sms(smsptr, pmsg[sms_num].tp_ud, &pmsg[sms_num]);			       
	    sms_num++;        														// 准备读下一条短消息 
	} 

	return sms_num;
}

int inser_atorder_file(const char *file, char *buf, int size)
{
	FILE *fp,*fp1;
	char buf1[100][100];
	char temp[100] = {0};
	int k = 0, i = 0, result = 0, num = 0;

	if (atorder_file)
	{
		fclose(atorder_file);
		atorder_file = NULL;
	}
	fp = fopen(file, "a+");
	if (!fp)
		perror(" ");
	while ((fgets(temp, 50, fp)) !=0)
	{		
		strcpy(buf1[k], temp);
		k++;
	}
	for (i = 0; i < k; i++)
	{
		if (memcmp((char *)"AT+CSQ", buf1[i], 6) == 0)
		{
			fclose(fp);
			//remove(file); //不能删除 因为有别的进程来读
			if ((fp1 = fopen(file,"w+")) == 0)  // 有就清空
			{
				perror("can not recreate file");
				return 0;
			}
			else
			{
				for (num = 0; num < i; num++)
				{
					fputs(buf1[num], fp1); 
				}
				fseek(fp1, 0, SEEK_END);
				fwrite(buf, 1, size, fp1);
				fflush(fp1);
				result = 1;
			}			
#if 0
			result = 1;
			break;
#endif
		}
		else if (memcmp((char *)"AT+CPIN?", buf1[i], 8) == 0)
		{
			for (num = i; num < k; num++)
			{
				fputs(buf1[num], fp1); 
			}
			fclose(fp1);
			return 0;
		}
	}
	if (result == 0)
	{
		fseek(fp, 0, SEEK_END);
		fwrite(buf, 1, size, fp);
		fclose(fp);
		return 0;
	}
	fclose(fp1);
	return 0;
#if 0
	if (result == 0)
	{
		fseek(fp, 0, SEEK_END);
		fwrite(buf, 1, size, fp);
		fclose(fp);
		return 0;
	}
	else
	{
		fclose(fp);
		//remove(file); //不能删除 因为有别的进程来读
		if ((fp1 = fopen(file,"w+")) == 0)  // 有就清空
		{
			perror("can not recreate file");
			return 0;
		}
		else
		{
			for(num = 0; num < i; num++)
			{
				fputs(buf1[num], fp1); 
			}
			fseek(fp1, 0, SEEK_END);
			fwrite(buf, 1, size, fp1);
			fclose(fp1);
			return 0;
		}
	}
#endif
}

int insert_atorder_cnsmod(const char *file, char *buf, int size)
{
	FILE *fp,*fp1;
	char buf1[100][100];
	char temp[100] = {0};
	int k = 0, i = 0, result = 0, num = 0;

	if (atorder_file)
	{
		fclose(atorder_file);
		atorder_file = NULL;
	}
	fp = fopen(file, "a+");
	if (!fp)
		perror(" ");
	while ((fgets(temp, 50, fp)) !=0)
	{		
		strcpy(buf1[k], temp);
		k++;
	}
	for (i = 0; i < k; i++)
	{
		if (memcmp((char *)"AT+CNSMOD?", buf1[i], 10) == 0)
		{
			fclose(fp);
			if ((fp1 = fopen(file,"w+")) == 0)  // 有就清空
			{
				perror("can not recreate file");
				return 0;
			}
			else
			{
				for (num = 0; num < i; num++)
				{
					fputs(buf1[num], fp1); 
				}
				fseek(fp1, 0, SEEK_END);
				fwrite(buf, 1, size, fp1);
				fflush(fp1);
				result = 1;
			}			
		}
	}
	if (result == 0) // 没有就直接在后面加
	{
		fseek(fp, 0, SEEK_END);
		fwrite(buf, 1, size, fp);
		fclose(fp);
		return 0;
	}
	fclose(fp1);
	return 0;
}

void at_swicth()
{
	int i, j, n = 0;
	int length;	
	int ret = 0;
	char *smsptr = NULL;          // 内部用的数据指针 
	sms_info pmsg[50];
	int sms_num = 0;
	
	switch(st_gsm_at.at.order)
	{		
		case AT:
		case ATE0:
		case ATE1:
		case ATS0:
		case AT_W:
		case AT_CHFA:
		case AT_COLP:	
		case AT_CLIP:
		case AT_STTONE:  
		case AT_CMGF:	
		case AT_SMS:
		case AT_CMGD:
		case AT_CSCA:	
		case AT_CPWD:
		case AT_CPIN:
		case AT_CLCK:
		case AT_CLVL:	
		case AT_CMIC:
		case AT_VTS:
		case AT_CPAMP:	
		case AT_CVHU:
		case AT_CHUP:
		case AT_CODECSEL:
		case AT_CLCC:
		case AT_CNMP:
		case AT_CSQFMT:
		case AT_CPMS:
		case AT_CNMI:
		case AT_CMSS:
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"OK",&st_gsm_at.rx.rx_data[i], 2) == 0)// 只是读
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i], 5) == 0)
				{
#ifdef _DBUG_
					printf("error\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}						
			}
			break;
		case AT_CPCMREGOVER:
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"OK",&st_gsm_at.rx.rx_data[i],2) == 0)// 只是读
				{
#ifdef _DBUG_
					printf("AT_CPCMREGOVER OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0 )
				{
#ifdef _DBUG_
					printf("AT_CPCMREGOVER error\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}			
			}
			break;
		case ATH:	
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"OK",&st_gsm_at.rx.rx_data[i],2) == 0)// 只是读
				{
#ifdef _DBUG_
					printf("ATH OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0 )
				{
#ifdef _DBUG_
					printf("ATH ERROR\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}			
			}
			break;
		case AT_QCRMCALL0:
		case AT_QCRMCALL1:
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"OK",&st_gsm_at.rx.rx_data[i],2) == 0)// 只是读
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0 )
				{
#ifdef _DBUG_
					printf("error\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}				
			}
		break;
		case AT_MONI:
			st_gsm_at.cell_num = 0;
			n = 0;
			memset(st_gsm_at.cellid,0x0,sizeof(st_gsm_at.cellid));
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"Id:",&st_gsm_at.rx.rx_data[i],3) == 0)
				{
					i += 5; //中间存在一个空格
					j = i;					
					for (;i < st_gsm_at.rx.rx_length; i++)
					{
						if (st_gsm_at.rx.rx_data[i] == ',' || st_gsm_at.rx.rx_data[i] == ']')
						{
							memcpy(st_gsm_at.cellid[n++],&st_gsm_at.rx.rx_data[i],i - j);
#ifdef _DBUG_
							printf("cellid%d: %s\n",n,st_gsm_at.cellid[n - 1]);
#endif
							dbug_log(__FILE__,__LINE__,"cellid%d: %s\n",n,st_gsm_at.cellid[n - 1]);
							break;
						}
					}				
				}
			}
			if (n > 0)
			{
				st_gsm_at.cell_num = n;
				st_gsm_at.at.state = AT_ANSWER_OK;
			}
			else
			{
				st_gsm_at.at.state = AT_ERROR;
			}
			break;
		case ATI:
			for (i=0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"IMEISV:",&st_gsm_at.rx.rx_data[i],7) == 0)// 只是读
				{
					i+=8;
					memset(&st_gsm_at.imei,0x0,sizeof(st_gsm_at.imei));
					memcpy(&st_gsm_at.imei,&st_gsm_at.rx.rx_data[i],15);
#ifdef _DBUG_
					printf("imei=%s\n",st_gsm_at.imei);
#endif
					dbug_log(__FILE__,__LINE__,"imei=%s\n",st_gsm_at.imei);
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
			}
			break;
		case AT_CPCMREG:
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"OK",&st_gsm_at.rx.rx_data[i],2) == 0)// 只是读
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0 )
				{
#ifdef _DBUG_
					printf("error...........\n");
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_ERROR;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
			}
			break;

		case AT_CPIN_R:// 检测有没有sim卡   // 这个地方回显不妥
			atorder_file = fopen(AT_ORDER_FILE, "a+");// 可读可写，清空文件
			if (!atorder_file) 
			{
				fprintf(stderr, "Unable to create file '%s'\n", AT_ORDER_FILE);
				exit(1); // 进程退出
			}
			ret = fwrite(st_gsm_at.rx.rx_data, 1, st_gsm_at.rx.rx_length, atorder_file);
			fflush(atorder_file);
			st_gsm_at.at.order = AT_INIT;
			length = strlen("+CPIN");
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"+CPIN",&st_gsm_at.rx.rx_data[i],length) == 0)
				{
					//"+CPIN:  READY",表示安装
					i+=length;
					for (;i<st_gsm_at.rx.rx_length;i++)
					{
						if (memcmp((char *)"READY",&st_gsm_at.rx.rx_data[i],5) == 0)
						{
							st_gsm_at.at.state = AT_ANSWER_UNLOCKSIM;			
							memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
							break;
						}
						else if (memcmp((char *)"SIM PIN",&st_gsm_at.rx.rx_data[i],7)==0)
						{
							st_gsm_at.at.state = AT_ANSWER_LOCKSIM;
							memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
							break;
						}
#if 1
						else if ((memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0) )// 表示没有安装
						{
							st_gsm_at.at.state = AT_ERROR;
							//st_gsm_state.main = HS_NOSIM;
#ifdef _DBUG_
							printf("There is no SIMCARD\n");
							dbug_log(__FILE__,__LINE__,"There is no SIMCARD\n");
#endif
							memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));
							break;
						}
#endif
					}
					break;			
				}		
#if 0
				else if ((memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0) )// 表示没有安装
				{
					st_gsm_at.at.state = AT_ERROR;
#ifdef _DBUG_
					printf("There is no SIMCARD\n");
					dbug_log(__FILE__,__LINE__,"There is no SIMCARD\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));
					break;
				}
#endif
			}
			break;		
		case ATD:
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"BEGIN",&st_gsm_at.rx.rx_data[i],5)==0)
				{
#ifdef _DBUG_
					printf("ATD/ATA OK\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA OK\n");
#endif
					st_gsm_at.at.state = AT_ATD_CONNECT_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0)
				{
#ifdef _DBUG_
					printf("ATD/ATA ERROR\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA ERROR\n");
#endif
					st_gsm_at.at.state = AT_ATD_ANSWER_ERROR;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"NO CARRIER",&st_gsm_at.rx.rx_data[i],10) == 0)
				{
#ifdef _DBUG_
					printf("ATD/ATA NO CARRIER\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA NO CARRIER\n");
#endif
					st_gsm_at.at.state = AT_ATD_ANSWER_ERROR;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"OK", &st_gsm_at.rx.rx_data[i], 2) == 0)
				{
#ifdef _DBUG_
					printf("ATD/ATA OK\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA OK\n");
#endif
					st_gsm_at.at.state = AT_ATD_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
			}
			break;
			
		case ATA:
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"BEGIN",&st_gsm_at.rx.rx_data[i],5)==0)
				{
#ifdef _DBUG_
					printf("ATD/ATA OK\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA OK\n");
#endif
					st_gsm_at.at.state = AT_CONNECT_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0)
				{
#ifdef _DBUG_
					printf("ATD/ATA ERROR\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA ERROR\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_ERROR;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"NO CARRIER",&st_gsm_at.rx.rx_data[i],10) == 0)
				{
#ifdef _DBUG_
					printf("ATD/ATA NO CARRIER\n");
					dbug_log(__FILE__,__LINE__,"ATD/ATA NO CARRIER\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_ERROR;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
			}
			break;
		case AT_QCRMCALL:
			for ( i = 0;i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"$QCRMCALL:",&st_gsm_at.rx.rx_data[i],10) == 0)// 表示拨号成功
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
#if 1
				else if (memcmp((char *)"ERROR",&st_gsm_at.rx.rx_data[i],5) == 0) // 表示没有插卡
				{
#ifdef _DBUG_
					printf("error\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"OK",&st_gsm_at.rx.rx_data[i],2) == 0) // 表示已经插了卡，拨号没有成功
				{
#ifdef _DBUG_
					printf("error\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
#endif
			}
			break;
		case AT_CPCMREG0:
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"+CPCMREG:", &st_gsm_at.rx.rx_data[i], 9) == 0)
				{
					if (memcmp((char *)"+CPCMREG: 0", &st_gsm_at.rx.rx_data[i], 11) == 0)// 表示sim模块alsa进程正常
					{
#ifdef _DBUG_
						printf("OK...........\n");
						dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
						st_gsm_at.at.state = AT_ANSWER_OK;
						memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
						break;
					}
					else if (memcmp((char *)"+CPCMREG: 1", &st_gsm_at.rx.rx_data[i], 11) == 0)// 表示sim模块alsa进程正常
					{
#ifdef _DBUG_
						printf("OK...........\n");
						dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
						st_gsm_at.at.state = AT_ANSWER_OK;
						memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
						break;

					}
					else
					{
#ifdef _DBUG_
						printf("error\n");
						st_gsm_at.at.state = AT_ERROR;
						dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
						memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));	
						break;
					}
				}
			}
			break;
		case AT_CMGL:
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (strstr(st_gsm_at.rx.rx_data, "OK") != NULL)
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					smsptr = st_gsm_at.rx.rx_data;
					sms_num = parse_pdu_sms(smsptr, pmsg);
					save_sms_message(SHORT_SMS_FILE, pmsg, sms_num);
					//write_cmgl_information(SHORT_SMS_FILE, st_gsm_at.rx.rx_data, st_gsm_at.rx.rx_length);  //TEXT MODE
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (strstr(st_gsm_at.rx.rx_data, "ERROR") != NULL)					
				{
#ifdef _DBUG_
					printf("error\n");
					st_gsm_at.at.state = AT_ERROR;
					dbug_log(__FILE__,__LINE__,"ERROR\n");
#endif
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}	
				else if (memcmp((char *)"AT+CMGL",&st_gsm_at.rx.rx_data[i], strlen("AT+CMGL")) == 0)
				{
					printf("AT+CMGL\n");
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
			}
			break;
		case AT_CMGW:
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)">", &st_gsm_at.rx.rx_data[i], 1) == 0)
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
			}
			break;
		case AT_CGMM:
			for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
			{
				if (memcmp((char *)"SIMCOM_SIM7100C", &st_gsm_at.rx.rx_data[i], strlen("SIMCOM_SIM7100C")) == 0)
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"SIMCOM_SIM7100E", &st_gsm_at.rx.rx_data[i], strlen("SIMCOM_SIM7100E")) == 0)
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_state.init_step++; // 跳过AT_CSCODEL指令
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
					break;
				}
				else if (memcmp((char *)"SIMCOM_SIM7100A", &st_gsm_at.rx.rx_data[i], strlen("SIMCOM_SIM7100A")) == 0)
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_state.init_step++; // 跳过AT_CSCODEL指令
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
				}
				else if (memcmp((char *)"SIMCOM_SIM7500A", &st_gsm_at.rx.rx_data[i], strlen("SIMCOM_SIM7500A")) == 0)
				{
#ifdef _DBUG_
					printf("OK...........\n");
					dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif
					st_gsm_state.init_step++; // 跳过AT_CSCODEL指令
					st_gsm_at.at.state = AT_ANSWER_OK;
					memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
				}
			}
			break;			
#if 1
		case AT_CIMI:
		case AT_COPS:
		case AT_CGSN:
			fwrite(st_gsm_at.rx.rx_data, 1, st_gsm_at.rx.rx_length, atorder_file);
			fflush(atorder_file);
			st_gsm_at.at.order = AT_INIT;
			st_gsm_at.at.state = AT_ANSWER_OK;
			memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));	
			break;	
		case AT_CSUB:
			fwrite(st_gsm_at.rx.rx_data, 1, st_gsm_at.rx.rx_length, atorder_file);
			fflush(atorder_file);
			//fclose(atorder_file);
			st_gsm_at.at.order = AT_INIT;
			st_gsm_at.at.state = AT_ANSWER_OK;
			memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));	
			break;	
		case AT_CSQ:
			inser_atorder_file(AT_ORDER_FILE, st_gsm_at.rx.rx_data, st_gsm_at.rx.rx_length);
			st_gsm_at.at.order = AT_INIT;
			st_gsm_at.at.state = AT_ANSWER_OK;
			memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));	
			break;
		case AT_CNSMOD:
			insert_atorder_cnsmod(AT_ORDER_FILE, st_gsm_at.rx.rx_data, st_gsm_at.rx.rx_length);
			st_gsm_at.at.order = AT_INIT;
			st_gsm_at.at.state = AT_ANSWER_OK;
			memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));	
			break;
#endif
		default:
			break; 
	}
}

void get_call_number(char *callnum)
{
	strncpy(callnum,st_gsm_at.callin_nu,strlen(st_gsm_at.callin_nu));
	//strncpy(callnum,"13016054035",strlen("13016054035"));
}

void gsm_sms_handler()
{
	int i = 0;
	for (i = 0; i < st_gsm_at.rx.rx_length; i++)
	{
		if (memcmp((char *)"+CMTI: \"SM\"",&st_gsm_at.rx.rx_data[i],strlen((char *)"+CMTI: \"SM\"")) == 0)
		{
			//有短信来了，需要更新短信箱
			sms_update = 1;
			memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX));
		}
	}
}

void gsm_rx_query_handler()
{
	int i,j;
	if (get_gsm_sub_state() == GSM_SUBSTATE_IDLE)			// 次状态为空闲状态并且主状态为常态 &&(get_gsm_mainstate() == HS_NORMAL)
	{
		for (i = 0; i < st_gsm_at.rx.rx_length; i++ )
		{
			if (memcmp((char *)"CLIP",&st_gsm_at.rx.rx_data[i],4) == 0)
			{
				i += 4;
				for ( ; i < st_gsm_at.rx.rx_length; i++ )
				{
					if (memcmp((char *)"\"",&st_gsm_at.rx.rx_data[i],1) == 0)
					{
						i += 1;
						j = i;	
						for ( ; i < st_gsm_at.rx.rx_length; i++ )
						{
							if (memcmp((char *)"\"",&st_gsm_at.rx.rx_data[i],1) == 0)
							{
								memset(&st_gsm_at.callin_nu[0],0x00,14);
								if ( (i-j) > 14 )
								{
									memcpy(&st_gsm_at.callin_nu[0],&st_gsm_at.rx.rx_data[j],14);
								}
								else
								{
									memcpy(&st_gsm_at.callin_nu[0],&st_gsm_at.rx.rx_data[j],i-j);
								}
#ifdef _DBUG_
								printf("st_gsm_at.callin_nu=%s\n",st_gsm_at.callin_nu);							
#endif
								dbug_log(__FILE__,__LINE__,"st_gsm_at.callin_nu=%s\n",st_gsm_at.callin_nu);
								set_gsm_sub_state(GSM_SUBSTATE_RING);			// 设置次状态为振铃模式							 
								memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));
								return ;
							}
						}
					}
				}
			}
		}
	}
	else if (get_gsm_sub_state() == GSM_SUBSTATE_DIALING)
	{
		for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
		{
			if (memcmp((char *)"BEGIN",&st_gsm_at.rx.rx_data[i],5)==0)
			{
#ifdef _DBUG_
				printf("ATD OK\n");
#endif
				dbug_log(__FILE__,__LINE__,"ATD OK\n");
				set_gsm_sub_state(GSM_SUBSTATE_CALLING_CONNECT);
				memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
				return ;
			}			
			else if (memcmp((char *)"NO CARRIER",&st_gsm_at.rx.rx_data[i],10) == 0)
			{
#ifdef _DBUG_
				printf("ATD NO CARRIER\n");		
#endif
				dbug_log(__FILE__,__LINE__,"ATD NO CARRIER\n");
				set_gsm_sub_state(GSM_SUBSTATE_CALLING_DISCONNECT);
				memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
				return ;
			}
		}
	}
	else if ((get_gsm_sub_state() == GSM_SUBSTATE_RING) ||(get_gsm_sub_state() == GSM_SUBSTATE_ANSWERING )) //(get_gsm_sub_state() == GSM_SUBSTATE_RING_ANSWER)	// 等待来电或者正有来电
	{
		for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
		{
			if (!memcmp((char *)"NO CAR",&st_gsm_at.rx.rx_data[i],3) ||
			!memcmp((char *)"MISSED_CALL",&st_gsm_at.rx.rx_data[i],11)) //断线
			{
#ifdef _DBUG_
				printf("NO CAR\n");
#endif
				dbug_log(__FILE__,__LINE__,"NO CAR\n");
				set_gsm_sub_state(GSM_SUBSTATE_HANG_UP);
				memset(&st_gsm_at.rx, 0x00, sizeof(ST_AT_RX)); 
				return ;
			}
		}
	}
	else if (get_gsm_sub_state() == GSM_SUBSTATE_SEND_SMS)
	{
		for ( i = 0; i < st_gsm_at.rx.rx_length; i++ )
		{
			if (memcmp((char *)"+CMGW:", &st_gsm_at.rx.rx_data[i], 6) == 0) // +CMGW: 19
			{
#ifdef _DBUG_
				printf("OK...........\n");
				dbug_log(__FILE__,__LINE__,"OK..............\n");
#endif			
				i += 7; 
				st_gsm_at.sms_num = atoi(&st_gsm_at.rx.rx_data[i]);
				st_gsm_at.at.state = AT_ANSWER_OK;
				set_gsm_sub_state(GSM_SUBSTATE_IDLE);	
				memset(&st_gsm_at.rx,0x00,sizeof(ST_AT_RX));	
				return ;
			}
		}
	}
	at_swicth();
}

void *gsm_send_handler(void *arg)
{
	while (1)
	{
		send_at_order();
	}
	pthread_exit(NULL);
}

void *gsm_recv_handler(void *arg)
{
	int ret = 0;

	while (1)
	{
		usleep(10000);
		ret = gsm_recv();
		if (ret > 0)
		{
#ifdef SMS_ENABLE
			gsm_sms_handler();
#endif
			gsm_rx_query_handler();	
		}
	}

	pthread_exit(NULL);
}

void *gsm_audio_recv(void *arg)
{
#if 0
	int res,i;
	FILE *fp;
	uchar audiobuf[320];
	fp=fopen("./audio","w+");
	if (fp==NULL)
	{
		printf("fopen failed\n");
		pthread_exit(NULL);
	}
	while (1)
	{
		if (pcm_flag==1)
		{
			memset(audiobuf,0,sizeof(audiobuf));
			res=uart_rev(fd2,audiobuf,sizeof(audiobuf));	//gsm audio
			if (res<0)
				printf("no PCM\n");
			if (res>0)
			{
				uart_send(fd3, audiobuf, res);// 发送到slic audio
			}
		}
	}
#endif
	pthread_exit(NULL);
}

