#include "user_handler.h"
#include "slic_hand.h"
#include "api_demo.h"
#include "proslic.h"
#include "tonegen.h"

#define SLEEP  500000
#if 0
int play_hurry_on_hook()
{
	if (st_user_state.hurry_delaytime > 400 && st_user_state.hurry_delaytime < 500)
	{
#ifdef _DBUGUSER_
		printf("-25dBm\n");
#endif
		//-25dBm
		//ToneGenSetAMP(ports[0].ProObj, 0x58000, 0x70000);
		//close_tone_gen_dial(&ports[0]);//关拨号音
		proslic_tone_gen_setup_new(ports[0].ProObj, 0, 0x000fa000L, 0x000fa000L);
		send_tone_gen_dial(&ports[0]);
	}
	else if (st_user_state.hurry_delaytime > 300 && st_user_state.hurry_delaytime < 400)
	{
#ifdef _DBUGUSER_
		printf("-20dBm\n");
#endif
		//-20
		//ToneGenSetAMP(ports[0].ProObj, 0x9e000, 0xc6000);
		//close_tone_gen_dial(&ports[0]);//关拨号音
		proslic_tone_gen_setup_new(ports[0].ProObj,0,0x001be000L,0x001be000L);
		send_tone_gen_dial(&ports[0]);
	}
	else if (st_user_state.hurry_delaytime>200&&st_user_state.hurry_delaytime<300)
	{
#ifdef _DBUGUSER_
		printf("-15dBm\n");
#endif
		//-15
		//ToneGenSetAMP(ports[0].ProObj, 0x118000, 0x162000);
		//close_tone_gen_dial(&ports[0]);//关拨号音
		proslic_tone_gen_setup_new(ports[0].ProObj,0,0x0031a000L,0x0031a000L);
		send_tone_gen_dial(&ports[0]);
	}
	else if (st_user_state.hurry_delaytime>100&&st_user_state.hurry_delaytime<200)
	{
#ifdef _DBUGUSER_
		printf("-10dBm\n");
#endif

		//-10
		//ToneGenSetAMP(ports[0].ProObj, 0x1f4000, 0x276000);
		//close_tone_gen_dial(&ports[0]);//关拨号音
		proslic_tone_gen_setup_new(ports[0].ProObj,0,0x00586000L,0x00586000L);
		send_tone_gen_dial(&ports[0]);
	}
	else if (st_user_state.hurry_delaytime>0&&st_user_state.hurry_delaytime<100)
	{
#ifdef _DBUGUSER_
		printf("-5dBm\n");
#endif
		//-5
		//ToneGenSetAMP(ports[0].ProObj, 0x378000, 0x460000);
		//close_tone_gen_dial(&ports[0]);//关拨号音
		proslic_tone_gen_setup_new(ports[0].ProObj,0,0x009d2000L,0x009d2000L);
		send_tone_gen_dial(&ports[0]);
	}
	//send_tone_gen_dial(&ports[0]);
}
#endif

void user_init()
{	
	open_flag = -1;
	pcm_start_flag = 0;
	on_hook_wait = 0;
	slic_light = LIGHT_DOWN;
	interupt_time = 0;
	spiread_error_times = 0;
	slic_pcm_read_open = 0;
	calltimes = 0;
	memset(&st_user_state,0x00,sizeof(st_user_state));

}

void dial_delay_time()
{
	if (st_user_state.dialing_delaytime > 0)
	{
		st_user_state.dialing_delaytime--;
	}
	if (st_user_state.dtmfing_delaytime > 0)
	{
		st_user_state.dtmfing_delaytime--;
	}
	if (st_user_state.busy_delaytime > 0)
	{
		st_user_state.busy_delaytime--;
	}
	if (st_user_state.hurry_delaytime > 0)
	{
		st_user_state.hurry_delaytime--;
	}
	if (st_slic_state.onhook_time > 0)
	{
		st_slic_state.onhook_time--;
	}
	if (st_slic_state.offhook_time > 0)
	{
		st_slic_state.offhook_time--;
	}
	if (interupt_time > 0)
	{
		interupt_time--;
	}
}

void  user_ring( chanState ports)
{
	uchar  channel;
	uchar  length; 
	uchar  dtmf[30] = {0};
	char   ascii[30] = {0};

	switch(st_user_state.sub)
	{
		case SUB_IDLE:		 	
			if (1)//(get_gsm_sub_state() == GSM_SUBSTATE_HANG_UP)
			{
				sleep(3);
				slic_light = LIGHT_BLINK_SLOW;
				get_time_of_day(&miss_call_list, END_TIME);
#ifdef _DBUG_
				printf("hello insert_miss_list_log\n");
#endif			
				insert_call_log(MISS_LIST, &miss_call_list);
				//pthread_mutex_lock(&cid_mutex);
				ProSLIC_RingStop(ports.ProObj); // 停止振铃
				toggle_ring_osc(&ports);	
				//pthread_mutex_unlock(&cid_mutex);
							
				set_gsm_sub_state(GSM_SUBSTATE_IDLE);
				st_user_state.main = USER_STATE_ONHOOK;
			}
			else if (get_slic_state() == SLIC_OFF_HOOK && st_slic_state.offhook_time == 0)
			{						
				ProSLIC_RingStop(ports.ProObj); // 停止振铃		
				toggle_ring_osc(&ports);
				slic_pcm_read_open = 1;
				st_user_state.sub = SUB_RINGSTOP;					
			}				
			break;	
		case SUB_RINGSTOP:	
			pthread_mutex_lock(&atorder_mutex);
			set_gsm_at_state(AT_IDLE);
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			gsm_callin_answer(ATA);
			st_user_state.sub = SUB_ATA_WAIT;
			break;
		case SUB_ATA_WAIT:
			if (get_gsm_at_state() == AT_CONNECT_OK)//AT_CONNECT_OK
			{		
				//proslic_pcm_start(ports.ProObj);
				//usleep(500000);
				set_gsm_at_state(AT_IDLE);
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_pcm_answer(AT_CPCMREG);//开启pcm	
				st_gsm_at.at_times = 0;
				st_user_state.sub = SUB_PCM_START_WAIT;				
			}		
			else if (get_gsm_at_state() == AT_ANSWER_ERROR)
			{
				slic_light = LIGHT_BLINK_SLOW;
				get_time_of_day(&miss_call_list,END_TIME);
#ifdef _DBUG_
				printf("hello insert_miss_list_log\n");
#endif
				insert_call_log(MISS_LIST, &miss_call_list);       	
				set_gsm_sub_state(GSM_SUBSTATE_IDLE);
				tone_gen_busy(&ports);//开忙音
				st_user_state.busy_delaytime = BUSY_DIALING_TIME; 
				st_user_state.main = USER_STATE_OFFHOOK;
				st_user_state.sub = SUB_WAITONHOOK;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case SUB_PCM_START_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{		
				st_gsm_at.at_times = 0;
				pcm_flag = 1;
				answer_begin = 1;
				set_gsm_sub_state(GSM_SUBSTATE_ANSWERING);
				st_user_state.main = USER_STATE_OFFHOOK;
				st_user_state.sub = SUB_ANSWERING;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (get_gsm_at_state() == AT_ANSWER_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);
				gsm_pcm_answer(AT_CPCMREG);//开启pcm	
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;		
		default:
			break;	
	}
	return ;
error_exit:
#if 0
	pthread_mutex_unlock(&atorder_mutex);
	//simcom_reset(ports[0].ProObj); //simcom 模块复位	
	simcom_disable(ports[0].ProObj);
	longjmp(env,1);
	//system("reboot");//系统复位	
#endif
	exit(1);

}

unsigned char int_to_char(int num)
{
	UINT8 i;
	
	if (num == 35)
		return '#';
	else if (num == 42)
		return '*';
	else
	{
		i = '0' + num - 48;
		return i;
	}
}

void dtmf_to_ascii(UINT8 *dtmf,char *ascii,UINT8 len)
{
	UINT8 i;	
	
	for ( i = 0; i < len; i++ )
	{
		if ( dtmf[i] <= 9 )
		{
			ascii[i] = dtmf[i]+'0';
		}
		else if ( dtmf[i] == 0x0a )
		{
			ascii[i] = '0';
		}
		else if ( dtmf[i] == 0x0b )
		{
			//ascii[i] = 0x40;   //'*'0x40
			ascii[i] = '*';
		}
		else if ( dtmf[i] == 0x0c )
		{
			//ascii[i] = 0x41;  //'#'0x41
			ascii[i] = '#';
		}
		else
		{
			break;
		}
	}
}

void  init_dtmf_rx_buff()
{
	memset(&st_dtmf_rx_buff,0x00,sizeof(ST_DTMF_RXBUFF));
}

uchar recv_dtmf(uInt8 uc_dtmf)
{
	if (st_dtmf_rx_buff.rx_addr < MAX_DTMF_BUFF)
	{
		st_dtmf_rx_buff.dtmf_buff[st_dtmf_rx_buff.rx_addr++] = uc_dtmf;	
	}
	return 0;
}

uchar  get_dtmf_rx_buff_length()
{
	if (st_dtmf_rx_buff.rx_addr < MAX_DTMF_BUFF)
	{
		return(st_dtmf_rx_buff.rx_addr);
	}
	else
	{
		return 0;
	}
}

void user_off_hook( chanState ports )
{
    char acsii[30] = {0};
    char *addr = NULL;
	char i = 0;
    uchar dtmf[30] = {0};
	uchar length; 
	char speed_dialing = 0;
	int ret = 0;
	
	switch(st_user_state.sub)
	{		 
		case SUB_RXDTMF_READY:
			pthread_mutex_lock(&dtmf_mutex);
			length = get_dtmf_rx_buff_length();
			pthread_mutex_unlock(&dtmf_mutex);
			if (length > 0)
			{				
				close_tone_gen_dial(&ports);//关拨号音
				st_user_state.sub = SUB_RXDTMF_END;
				st_user_state.last_rxdtmf_length = length;
				st_user_state.dtmfing_delaytime = dial_up_timeout * 100;  //拨号位间定时，默认5s
			}
			else if (length == 0 && st_user_state.dialing_delaytime == 0)
			{					
				close_tone_gen_dial(&ports);//关拨号音
				usleep(SLEEP);
				tone_gen_busy(&ports);//开忙音
				st_user_state.busy_delaytime = BUSY_DIALING_TIME;// 忙音超时 30s
				st_user_state.sub = SUB_WAITONHOOK; //等待挂机					
			}
			break;
		case SUB_RXDTMF_END:
			pthread_mutex_lock(&dtmf_mutex);
			length = get_dtmf_rx_buff_length();
			memset(st_dtmf_rx_buff.ascii_buff, 0, sizeof(st_dtmf_rx_buff.ascii_buff));
#ifndef DTMF_DECODE
			dtmf_to_ascii(st_dtmf_rx_buff.dtmf_buff, st_dtmf_rx_buff.ascii_buff, length);// 做一下转换
#else
			memcpy(st_dtmf_rx_buff.ascii_buff, st_dtmf_rx_buff.dtmf_buff, length);
#endif
			pthread_mutex_unlock(&dtmf_mutex);		
			if (st_user_state.last_rxdtmf_length == length)
			{
				if (st_user_state.dtmfing_delaytime == 0)
				{
					//拨号
					st_user_state.sub = SUB_DIALING;
					break;
				}
			}	
			else  //不相等表示有按键按下
			{
#ifdef _DBUG_
				printf("st_dtmf_rx_buff.rx_addr = %d\n", length);				
#endif
				dbug_log(__FILE__,__LINE__,"st_dtmf_rx_buff.rx_addr=%d\n", length);
				st_user_state.last_rxdtmf_length = length;
		   		st_user_state.dtmfing_delaytime = dial_up_timeout * 100;
			}
			//匹配数图
			if (map_enable == 1)
			{
				ret = pattern_separate_match(pattern, st_dtmf_rx_buff.ascii_buff);
#ifdef _DBUG_
				//printf("ret=%d\n",ret);
#endif
				if (ret == 1) //完全匹配
				{						
					st_user_state.sub = SUB_DIALING;
				}
				else
				{
					if (ret == 0) //部分匹配
					{					
#if 1
						if (dialing_key == 1 && st_dtmf_rx_buff.ascii_buff[length -1] == '#')
						{
#ifdef _DBUG_
							printf("st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '#'\n");
							dbug_log(__FILE__,__LINE__,"st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '#'\n");
#endif
							st_dtmf_rx_buff.rx_addr -= 1;
							st_user_state.sub = SUB_DIALING;
						}
						else if (dialing_key == 2 && st_dtmf_rx_buff.ascii_buff[length -1] == '*')
						{
#ifdef _DBUG_
							printf("st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '*'\n");
							dbug_log(__FILE__,__LINE__,"st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '*'\n");
#endif
							st_dtmf_rx_buff.rx_addr -= 1;
							st_user_state.sub = SUB_DIALING;
						}
						else if (unmatch_undial == 1)
						{
							tone_gen_busy(&ports);//开忙音
							st_user_state.busy_delaytime = BUSY_DIALING_TIME;
							st_user_state.sub = SUB_WAITONHOOK; //等待挂机				
						}
#endif
					}
				}
			}		
			else 
			{
#if 1
				if (dialing_key == 1 && st_dtmf_rx_buff.ascii_buff[length -1] == '#')
				{
#ifdef _DBUG_
					printf("st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '#'\n");
					dbug_log(__FILE__,__LINE__,"st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '#'\n");
#endif
					st_dtmf_rx_buff.rx_addr -= 1;
					st_user_state.sub = SUB_DIALING;
				}
				else if (dialing_key == 2 && st_dtmf_rx_buff.ascii_buff[length -1] == '*')
				{
#ifdef _DBUG_
					printf("st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '*'\n");
					dbug_log(__FILE__,__LINE__,"st_dtmf_rx_buff.ascii_buff[st_dtmf_rx_buff.rx_addr-1] == '*'\n");
#endif
					st_dtmf_rx_buff.rx_addr -= 1;
					st_user_state.sub = SUB_DIALING;
				}
#endif
			}
			break;
		case SUB_DIALING: 
			set_gsm_sub_state(GSM_SUBSTATE_DIALING);   							
			memset(st_dtmf_rx_buff.ascii_buff, 0, sizeof(st_dtmf_rx_buff.ascii_buff));
			pthread_mutex_lock(&dtmf_mutex);			
#ifndef DTMF_DECODE
			dtmf_to_ascii(st_dtmf_rx_buff.dtmf_buff, st_dtmf_rx_buff.ascii_buff, st_dtmf_rx_buff.rx_addr);// 做一下转换
#else
			memcpy(st_dtmf_rx_buff.ascii_buff, st_dtmf_rx_buff.dtmf_buff, st_dtmf_rx_buff.rx_addr);
#endif
			//init_dtmf_rx_buff();
			st_dtmf_rx_buff.rx_addr = 0;
			memset(st_dtmf_rx_buff.dtmf_buff, 0, sizeof(st_dtmf_rx_buff.dtmf_buff));
			pthread_mutex_unlock(&dtmf_mutex);
#ifdef _DBUG_
			printf("call_number=%s\n", st_dtmf_rx_buff.ascii_buff);
			dbug_log(__FILE__,__LINE__,"call_number=%s\n", st_dtmf_rx_buff.ascii_buff);
#endif				
			memset(dial_list.call_number, 0, 14);
			memcpy(dial_list.call_number, st_dtmf_rx_buff.ascii_buff, strlen(st_dtmf_rx_buff.ascii_buff));
			get_time_of_day(&dial_list, START_TIME);	//获取开始时间		
			pthread_mutex_lock(&atorder_mutex);
			set_gsm_at_state(AT_IDLE);// 设置at状态为空闲
			dialing(dial_list.call_number);// 拨号
			//proslic_pcm_start(ports.ProObj);
			usleep(SLEEP);
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			st_user_state.sub = SUB_CALLING_WAIT;
			//open_flag = 1;
			//fd2 = pcm_channel_open("/dev/ttyUSB4");
			break;
		case SUB_CALLING_WAIT:
			if (get_gsm_at_state() == AT_ATD_ANSWER_OK)
			{
				//st_user_state.sub = SUB_PCM_START;
				//pthread_mutex_unlock(&atorder_mutex);	
				set_gsm_at_state(AT_IDLE);				// 设置at状态为空闲
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_pcm_answer(AT_CPCMREG);		//开启pcm语音通道
				st_gsm_at.at_times = 0;
				st_user_state.sub = SUB_PCM_START_WAIT;
			}			
			else if (get_gsm_at_state() == AT_ATD_ANSWER_ERROR)
			{
				pthread_mutex_unlock(&atorder_mutex);		
				get_time_of_day(&dial_list, 1);	//获取时间	
				get_time_of_day(&dial_list, 0);
#ifdef _DBUG_
				printf("hello insert_dial_list_log\n");
#endif		
				//proslic_pcm_stop(ports.ProObj);
				insert_call_log(DIAL_LIST, &dial_list);
				tone_gen_busy(&ports);//开忙音  写数据
				st_user_state.busy_delaytime = BUSY_DIALING_TIME;
				st_user_state.sub = SUB_WAITONHOOK;// 等待挂机
			}
			else if (get_gsm_sub_state() == GSM_SUBSTATE_CALLING_CONNECT)
			{				
#ifdef _DBUG_
				printf("AT_CONNECT_OK\n");
				dbug_log(__FILE__,__LINE__,"AT_CONNECT_OK\n");
#endif
				answer_begin = 1; //对方接听标志位
				get_time_of_day(&dial_list, 1);	//获取时间	
				set_gsm_sub_state(GSM_SUBSTATE_ANSWERING);
				st_user_state.sub = SUB_ANSWERING;				
			}
			else if (get_gsm_sub_state() == GSM_SUBSTATE_CALLING_DISCONNECT)
			{
				get_time_of_day(&dial_list, 1);	//获取时间	
				get_time_of_day(&dial_list, 0);
				insert_call_log(DIAL_LIST, &dial_list);
				//proslic_pcm_stop(ports.ProObj);
				tone_gen_busy(&ports);//开忙音  写数据
				st_user_state.busy_delaytime = BUSY_DIALING_TIME;				
				set_gsm_sub_state(GSM_SUBSTATE_IDLE);
				pcm_flag = 0;
				usleep(ATH_TIME_BEGIN);
				pthread_mutex_lock(&atorder_mutex);	
				set_gsm_at_state(AT_IDLE);	
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_on_hook();
				st_gsm_at.at_times = 0;
				st_user_state.sub = SUB_ATH_WAIT;
			}
			else if (st_gsm_state.tx_overtime == 0 && pcm_start_flag != 1)
			{
				goto error_exit;
			}
			break;
		case SUB_PCM_START_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{					
				st_gsm_at.at_times = 0;
				pcm_flag = 1; //有回复之后置1
				pcm_start_flag = 1;
				st_user_state.sub = SUB_CALLING_WAIT;
				pthread_mutex_unlock(&atorder_mutex);
			}
			else if (get_gsm_at_state() == AT_ANSWER_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);				// 设置at状态为空闲
				gsm_pcm_answer(AT_CPCMREG);		//开启pcm语音通道
			}
			else if (st_gsm_state.tx_overtime == 0)
			{			
				goto error_exit;
			}
			break;
		case SUB_ATH_WAIT:				
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{				
				st_gsm_at.at_times = 0;
				set_gsm_at_state(AT_IDLE);
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_pcm_over(AT_CPCMREGOVER);
				st_user_state.sub = SUB_PCM_STOP_WAIT;
			}		
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);	
				gsm_on_hook();
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case SUB_PCM_STOP_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{				
				pthread_mutex_unlock(&atorder_mutex);	
				st_gsm_at.at_times = 0;
				st_user_state.sub = SUB_WAITONHOOK;	
			}
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);
				gsm_pcm_over(AT_CPCMREGOVER);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;	
		case SUB_ANSWERING:	//通话中						
			if (get_gsm_sub_state() == GSM_SUBSTATE_HANG_UP)// 表示对方挂机了
			{
				set_gsm_sub_state(GSM_SUBSTATE_IDLE);
#ifdef _DBUG_
				printf("AT_NOCAR\n");
				dbug_log(__FILE__,__LINE__,"AT_NOCAR\n");
#endif
				if (answer_call == 1)
				{
					answer_call = 0;
					get_time_of_day(&recv_call_list, 0);
#ifdef _DBUG_
					printf("hello insert_recv_list_log\n");
#endif
					dbug_log(__FILE__,__LINE__, "hello insert_recv_list_log\n");
					insert_call_log(RECV_LIST, &recv_call_list);
				}
				else
				{
					get_time_of_day(&dial_list, 0);
#ifdef _DBUG_
					printf("hello insert_dial_list_log\n");
#endif
					dbug_log(__FILE__,__LINE__, "hello insert_dial_list_log\n");
					insert_call_log(DIAL_LIST, &dial_list);
				}
				tone_gen_busy(&ports);//开忙音
				//proslic_pcm_stop(ports.ProObj);
				st_user_state.busy_delaytime = BUSY_DIALING_TIME;
				pcm_flag = 0;
				usleep(ATH_TIME_BEGIN);
				pthread_mutex_lock(&atorder_mutex);	
				set_gsm_at_state(AT_IDLE);	
				st_gsm_at.at_times = 0;
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_on_hook();
				st_user_state.sub = SUB_ATH_WAIT;
			}		
#if 1
			else if (get_dtmf_rx_buff_length() > 0)// 二次拨号
			{
				memset(st_dtmf_rx_buff.ascii_buff, 0x0, sizeof(st_dtmf_rx_buff.ascii_buff));
#ifndef DTMF_DECODE
				dtmf_to_ascii(st_dtmf_rx_buff.dtmf_buff, st_dtmf_rx_buff.ascii_buff, 1);// 做一下转换
#else
				memcpy(st_dtmf_rx_buff.ascii_buff, st_dtmf_rx_buff.dtmf_buff, 1);
#endif
				pthread_mutex_lock(&atorder_mutex);	
				set_gsm_at_state(AT_IDLE);
				st_gsm_at.at_times = 0;
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				dtmf_tone_dialing(st_dtmf_rx_buff.ascii_buff);//有问题
				//init_dtmf_rx_buff();
				st_dtmf_rx_buff.rx_addr = 0;
				memset(st_dtmf_rx_buff.dtmf_buff, 0, sizeof(st_dtmf_rx_buff.dtmf_buff));
				st_user_state.sub = SUB_VTS_WAIT;
			}	
#endif
			break;
		case SUB_VTS_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{
				pthread_mutex_unlock(&atorder_mutex);	
				st_gsm_at.at_times = 0;
				st_user_state.sub = SUB_ANSWERING;
			}
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);
				dtmf_tone_dialing(st_dtmf_rx_buff.ascii_buff);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case SUB_WAITONHOOK:  //只有挂钩挂上上去了才会进入挂机模块，否则等到忙音时间到
			if (st_user_state.busy_delaytime == 0)
			{				
				close_tone_gen_dial(&ports);//关拨号音
				usleep(SLEEP);
				proslic_tone_gen_setup_new(ports.ProObj, 0, 0x01178000L, 0x01178000L);	// 0dbm
				send_tone_gen_dial(&ports);
				st_user_state.sub = SUB_HURRYONHOOK;// 催挂机
			}
			break;
		case SUB_HURRYONHOOK:
			break;
		default:
			break; 
	}
	return ;	
error_exit:
#if 0
	pthread_mutex_unlock(&atorder_mutex);
	//simcom_reset(ports[0].ProObj); //simcom 模块复位	
	simcom_disable(ports[0].ProObj);
	longjmp(env,1);
	//system("reboot");//系统复位			
#endif
	exit(1);

}

void  user_on_hook( chanState ports )
{
	switch(st_user_state.sub)
	{
		case SUB_RXDTMF_READY:
			st_user_state.dialing_delaytime = 0;	//时间清零
			close_tone_gen_dial(ports.ProObj);	//关拨号音
			st_user_state.main = USER_STATE_ONHOOK;
			break;
			
		case SUB_DIALING:	
		case SUB_RXDTMF_END:
			init_dtmf_rx_buff();
			st_user_state.dtmfing_delaytime = 0;
			st_user_state.main = USER_STATE_ONHOOK;
			break;
#if 0
		case SUB_PCM_START:	
			pthread_mutex_lock(&atorder_mutex);
#endif
		case SUB_CALLING_WAIT:								
			set_gsm_sub_state(GSM_SUBSTATE_IDLE);   	
			//proslic_pcm_stop(ports.ProObj);
			pcm_flag = 0;
			usleep(ATH_TIME_BEGIN);
			if (pcm_start_flag == 1)
				pthread_mutex_lock(&atorder_mutex);		
			set_gsm_at_state(AT_IDLE);// 设置at状态为空闲
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			st_gsm_at.at_times = 0;
			gsm_on_hook();
			on_hook_wait = 1;
			st_user_state.sub= SUB_CALL_ATH_WAIT;
			break;
		case SUB_CALL_ATH_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{
#ifdef _DBUG_
				printf("ATH_OVER\n");
#endif
				dbug_log(__FILE__,__LINE__, "ATH_OVER\n");
				st_gsm_at.at_times = 0;
				if (pcm_start_flag == 1)
				{
					pcm_start_flag = 0;
					get_time_of_day(&dial_list,1);
					get_time_of_day(&dial_list,0);
					insert_call_log(DIAL_LIST,&dial_list);
					set_gsm_at_state(AT_IDLE);
					st_gsm_at.at_times = 0;
					st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
					gsm_pcm_over(AT_CPCMREGOVER);
					on_hook_wait = 1;
					st_user_state.sub = SUB_PCM_STOP_WAIT;
				}
				else
				{
					pthread_mutex_unlock(&atorder_mutex); //先不解锁，直接发送AT+CPCMREG=0
					get_time_of_day(&dial_list,1);
					get_time_of_day(&dial_list,0);
					insert_call_log(DIAL_LIST,&dial_list);
					st_user_state.main = USER_STATE_ONHOOK;
				}
				
#ifdef _DBUG_
				printf("hello insert_dial_list_log\n");
#endif			
				dbug_log(__FILE__,__LINE__, "hello insert_dial_list_log\n");
			}
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);// 设置at状态为空闲
				gsm_on_hook();
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case SUB_PCM_STOP_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{
				pthread_mutex_unlock(&atorder_mutex);
				st_gsm_at.at_times = 0;
				st_user_state.main = USER_STATE_ONHOOK;
			}
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);
				gsm_pcm_over(AT_CPCMREGOVER);
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;
		case SUB_PCM_START_WAIT:
			set_gsm_sub_state(GSM_SUBSTATE_IDLE);   		
			//proslic_pcm_stop(ports.ProObj);
			pcm_flag = 0;
			usleep(ATH_TIME_BEGIN);
			set_gsm_at_state(AT_IDLE);// 设置at状态为空闲
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			gsm_on_hook();
			pcm_start_flag = 1;
			on_hook_wait = 1;
			st_user_state.sub = SUB_CALL_ATH_WAIT;			
			break;
		case SUB_VTS_WAIT:
			pthread_mutex_unlock(&atorder_mutex);
		case SUB_ANSWERING:		
			set_gsm_sub_state(GSM_SUBSTATE_IDLE);   	
			//proslic_pcm_stop(ports.ProObj);
			pcm_flag = 0;
			usleep(ATH_TIME_BEGIN);
			pthread_mutex_lock(&atorder_mutex);
			set_gsm_at_state(AT_IDLE);	// 设置at状态为空闲
			st_gsm_at.at_times = 0;
			st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
			gsm_on_hook();
			on_hook_wait = 1;
			st_user_state.sub = SUB_ATH_ONHOOK_WAIT;
			break;	
		case SUB_ATH_ONHOOK_WAIT:
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{
				if (answer_call == 1)
				{
					answer_call = 0;
					get_time_of_day(&recv_call_list, 0);
#ifdef _DBUG_
					printf("hello insert_recv_list_log\n");
#endif
					insert_call_log(RECV_LIST, &recv_call_list);					
				}
				else 
				{
					get_time_of_day(&dial_list, 0);
#ifdef _DBUG_
					printf("hello insert_dial_list_log\n");
#endif
					dbug_log(__FILE__,__LINE__, "hello insert_dial_list_log\n");
					insert_call_log(DIAL_LIST, &dial_list);
				}
				st_gsm_at.at_times = 0;
				set_gsm_at_state(AT_IDLE);
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_pcm_over(AT_CPCMREGOVER);
				on_hook_wait = 1;
				st_user_state.sub = SUB_PCM_STOP_WAIT;
			}
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);	// 设置at状态为空闲
				gsm_on_hook();
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}
			break;			
		case SUB_ATH_WAIT:
			close_tone_gen_dial(&ports);//关忙音
			st_user_state.busy_delaytime = 0;
			//usleep(500000); //这个地方存在风险，如果挂机没有成功，发送这条指令会造成alsa进程退出
			if (get_gsm_at_state() == AT_ANSWER_OK)
			{				
				set_gsm_at_state(AT_IDLE);
				st_gsm_at.at_times = 0;
				st_gsm_state.tx_overtime = GSM_RX_OVERTIM;
				gsm_pcm_over(AT_CPCMREGOVER);
				on_hook_wait = 1;
				st_user_state.sub = SUB_PCM_STOP_WAIT;
			}		
			else if (get_gsm_at_state() == AT_ERROR)
			{
				st_gsm_at.at_times++;
				if (st_gsm_at.at_times >= AT_TIMES)
					goto error_exit;
				set_gsm_at_state(AT_IDLE);	
				gsm_on_hook();
			}
			else if (st_gsm_state.tx_overtime == 0)
			{
				goto error_exit;
			}		
			break;			
		case SUB_WAITONHOOK:
		case SUB_HURRYONHOOK:
			close_tone_gen_dial(&ports);//关忙音
			st_user_state.busy_delaytime = 0;
			st_user_state.main = USER_STATE_ONHOOK;
			break;					
	}

	return ;
error_exit:
#if 0
	pthread_mutex_unlock(&atorder_mutex);
	//simcom_reset(ports[0].ProObj); //simcom 模块复位	
	simcom_disable(ports[0].ProObj);
	longjmp(env,1);
	//system("reboot");//系统复位	
#endif
	exit(1);

}

void *user_hanlder(void *arg)
{	
	char callnum[14];
	chanState *ports = (chanState *)arg;
#if 0
	 if (setjmp(env))
	 {
		printf("DBG: %s: software running error need restart\n", __FUNCTION__);
		ProSLIC_ShutdownChannel(ports[0].ProObj);
		sivoice_reset(ports[0].ProObj);
		ProSLIC_destroyChannel(&(ports[0].ProObj));
		ProSLIC_destroyDevice(&(ProSLICDevices[0]));
		/* Close any system resources */
		destroyControlInterfaces(&ProHWIntf);
		/* Free the memory for the control interfaces */
		SiVoice_destroyControlInterface(&ProHWIntf);
		exit(1);
	 }
#endif
	while (1)
	{
		switch (st_user_state.main)
		{			
			case USER_STATE_INIT:
				st_user_state.main = USER_STATE_IDLE;
				st_user_state.sub = SUB_IDLE;
				break;
			case USER_STATE_IDLE:	
				if (get_slic_state() == SLIC_OFF_HOOK && st_slic_state.offhook_time == 0)// 检测是否摘机
				{					
#ifdef _DBUG_				
					printf("send_tone_gen_dial>>>\n");
#endif
					//send_tone_gen_dial(&ports[0]);			//送拨号音
					send_tone_gen_dial(ports);			//送拨号音
					slic_pcm_read_open = 1;
					init_dtmf_rx_buff();
					st_user_state.main = USER_STATE_OFFHOOK; // 主状态为摘机
					st_user_state.sub = SUB_RXDTMF_READY;// 次状态为准备接收dtmf
					st_user_state.dialing_delaytime = START_DIALING_TIME;//摘机不拨号定时30秒
				}
				else if (1)//(get_gsm_sub_state() == GSM_SUBSTATE_RING)
				{					
					sleep(1);
					answer_call = 1; //来电标志位
					memset(callnum, 0x0, sizeof(callnum));
					strcpy(callnum, "13016054035");
				//	get_call_number(callnum);
#ifdef _DBUGUSER_
					printf("callnum = %s\n", callnum);
					dbug_log(__FILE__,__LINE__,"callnum = %s\n", callnum);
#endif
					memset(recv_call_list.call_number, 0, 14);
					memset(miss_call_list.call_number, 0, 14);
					memcpy(recv_call_list.call_number, callnum, strlen(callnum));
					memcpy(miss_call_list.call_number, callnum, strlen(callnum));
					get_time_of_day(&recv_call_list, START_TIME);	//获取时间	
					get_time_of_day(&miss_call_list, START_TIME);	//获取时间	
#ifdef _DBUG_
					dbug_log(__FILE__,__LINE__,"send cid stream\n");
#endif
#if 1					
					slic_light = LIGHT_BLINK_SLOW;					//电话指示灯闪烁
					if (call_id_enable == ENABLE && call_id_mode == FSK)
					{
						//pthread_mutex_lock(&cid_mutex);
						//send_cid_stream_new(&ports[0], callnum);//发送来电显示 FSK模式
						//sendCIDStream(&ports[0]);
						sendCIDStream(ports);
						sleep(2);						
						//ProSLIC_RingStart(ports[0].ProObj); // 开振铃	
						//toggle_ring_osc(&ports[0]); // 打开晶振
						ProSLIC_RingStart(ports->ProObj); // 开振铃	
						toggle_ring_osc(ports); // 打开晶振
						//pthread_mutex_unlock(&cid_mutex);
						calltimes++;
						printf("calltimes = %d\n", calltimes);
					}
#if 0
					else if (call_id_enable == ENABLE && call_id_mode == DTMF) //还未做
					{
						
					}
#endif
							
#endif
					//open_flag = 1;// ttyUSB4打开标志位
					//fd2 = pcm_channel_open("/dev/ttyUSB4");
					st_user_state.main = USER_STATE_RING; //USER_STATE_IDLE
					st_user_state.sub = SUB_IDLE;
				}
				break;
			case USER_STATE_RING:			
				//user_ring(ports[0]);				
				user_ring(*ports);	
				break;	
			case USER_STATE_OFFHOOK:
				if (get_slic_state() == SLIC_OFF_HOOK && st_slic_state.offhook_time == 0 && on_hook_wait != 1)//摘机
				{	
					slic_pcm_read_open = 1;
					user_off_hook(*ports);
				}
				else if ((get_slic_state() == SLIC_ON_HOOK && st_slic_state.onhook_time == 0) ||on_hook_wait == 1)                                              
				{			 	
					slic_pcm_read_open = 0;
					user_on_hook(*ports);   //挂机
				}	 
				break;
			case USER_STATE_ONHOOK:
				set_gsm_sub_state(GSM_SUBSTATE_IDLE);
				//ProSLIC_ToneGenSetup(ports[0].ProObj, 0);			
				ProSLIC_ToneGenSetup(ports->ProObj, 0);		
				open_flag = 0; 
				answer_begin = 0;
				on_hook_wait = 0;
				pcm_start_flag = 0;
				st_user_state.main = USER_STATE_IDLE;
				st_user_state.sub = SUB_IDLE;
				if (calltimes >= CALL_TIMES)
						//longjmp(env,1);
				break;
		}
		pthread_mutex_lock(&send_atorder_mutex);
		send_at_order();
		pthread_mutex_unlock(&send_atorder_mutex);
		//interrupt_handler(&ports[0]);
		interrupt_handler(ports);
	}
	pthread_exit(NULL);
}

