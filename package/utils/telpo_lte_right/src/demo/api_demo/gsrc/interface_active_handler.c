#include "gsm.h"
#include "interface_active.h"

void interacter_init()
{
	memset(&st_menu,0x0,sizeof(st_menu));
}

void *interface_active_handler(void  *arg)
{
	char	*lock_flag;
	char *cell_lock_flag;
	char *pin;
	char *oldpin;
	char *newpin;
	int input_nums = 3;
	int i, j, m = 0, count = 0, equal_flag = 0, n = 0;
	char *num = NULL;
	uchar *imei = NULL;
	char mac_addr[20] = {0};
	char modify_pin[40] = {0};
	char unlock_flag = 0;
	char auto_bind_flag = 0;
	char auto_unlock_failed = 0;
	char bind_flag = 0;
	char pin_input_num = 3;
	char puk_input_num = 10;
	char user_celllock_flag = 0;
	char cell_locked_buf[10][10] = {0};
	
	while (1)
	{		
		//gsm_init_and_sim_active();
		switch (st_menu.main_interface_index)
		{	
			case TS_CELLLOCK_DECTE:
				//检测是否启用锁小区
				if (0)
				{
					pthread_mutex_lock(&atorder_mutex);
					st_gsm_at.at.state = AT_IDLE;
					switch_to_gsm("13");
					st_menu.main_interface_index = TS_SWITCH_TO_GSM_WAIT;
				}
				else
				{
					st_menu.main_interface_index = TS_INIT;
				}
				break;
			case TS_SWITCH_TO_GSM_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
					pthread_mutex_lock(&atorder_mutex);
					//扫描当前小区
					query_gsm_cell();
					st_menu.main_interface_index = TS_QUERY_GSM_CELL_WAIT;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
					if (user_celllock_flag == 1)
					{
						st_menu.main_interface_index = TS_USER_IDLE;
					}
					else 
					{
						st_menu.main_interface_index = TS_INIT;
					}
				}
				break;
			case TS_QUERY_GSM_CELL_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
					//获取已经锁定的小区，保存到cell_locked_buf
					//假设获取到的小区数是m
					n = m;
					if (m == 0) //判断buf是否有小区数据
					{
						//把当前扫描到的小区全部保存到cell_locked_buf
						for (i = 0; i < st_gsm_at.cell_num; i++)
						{
							memcpy(cell_locked_buf[i],st_gsm_at.cellid[i],strlen(st_gsm_at.cellid[i]));
						}
						pthread_mutex_lock(&atorder_mutex);
						st_gsm_at.at.state = AT_IDLE;
						switch_to_gsm("2");
						st_menu.main_interface_index = TS_SWITCH_TO_NORMAL_MODE;
					}
					else 
					{
						for (i = 0; i < st_gsm_at.cell_num; i++)
						{
							for (j = 0; j < m; j++)
							{
								if (memcmp(cell_locked_buf[j], st_gsm_at.cellid[i], strlen(st_gsm_at.cellid[i])) == 0)
								{
									count++;
									equal_flag = 1;
									break;
								}
							}
							if (equal_flag == 1)// 表示与锁定的小区里面其中一个相同
							{
								equal_flag = 0;
							}
							else // 表示没有与其中任何一个相同
							{
								memcpy(cell_locked_buf[n++], st_gsm_at.cellid[i], strlen(st_gsm_at.cellid[i])); //暂时往锁定小区buf里面填充
							}
							
						}
						if (count > (m>>1)) //大部分相同
						{
							//把cell_locked_buf 里面的所有小区全部保存到系统里面
							pthread_mutex_lock(&atorder_mutex);
							st_gsm_at.at.state = AT_IDLE;
							switch_to_gsm("2");
							st_menu.main_interface_index = TS_SWITCH_TO_NORMAL_MODE;
						}
						else 
						{
							if (count != 0)
							{
								pthread_mutex_lock(&atorder_mutex);
								st_gsm_at.at.state = AT_IDLE;
								switch_to_gsm("2");
								st_menu.main_interface_index = TS_SWITCH_TO_NORMAL_MODE;
							}
							else
							{
								//不能上网，关闭数据连接
								if (user_celllock_flag == 1)
								{
									st_menu.main_interface_index == TS_USER_IDLE;
								}
								else
								{
									st_menu.main_interface_index = TS_INIT;
								}
							}
						}
					}
				}
				else if (st_gsm_at.at.state == AT_ERROR) //没有扫描到小区
				{
					pthread_mutex_unlock(&atorder_mutex);
					// 关闭数据连接，不能上网
					if (user_celllock_flag == 1)
					{
						st_menu.main_interface_index == TS_USER_IDLE;
					}
					else
					{
						st_menu.main_interface_index = TS_INIT;
					}
				}
				break;
			case TS_SWITCH_TO_NORMAL_MODE:
				if (st_gsm_at.at.state == AT_ANSWER_OK ||st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
					//开启数据连接，可以上网
					if (user_celllock_flag == 1)
					{
						st_menu.main_interface_index == TS_USER_IDLE;
					}
					else
					{
						st_menu.main_interface_index = TS_INIT;
					}
				}
				break;
			case TS_GET_IMEI:
				//sem_wait(&gsm_sem);
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;
				get_sim_imei();
				st_menu.main_interface_index = TS_GET_IMEI_WAIT;
				break;
			case TS_GET_IMEI_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);			
					st_menu.main_interface_index = TS_INIT;
				}
				break;
#if 1
			case TS_INIT:				
				imei = get_imei_from_gsm();
				get_mac_address(mac_addr);
				encryp_codec(imei, mac_addr, strlen(imei), modify_pin, 1);
				setvar_to_sys_file(LOCKCARD_FILE,"pin",modify_pin);
				pthread_mutex_lock(&atorder_mutex);
				st_gsm_at.at.state = AT_IDLE;
				query_sim_card();
				st_menu.main_interface_index = TS_LOCK_CARD_DETECT_WAIT;
				break;
			case TS_LOCK_CARD_DETECT_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_UNLOCKSIM)
				{
#ifdef _DBUG_
					printf("this sim card isn't  set lock\n");
#endif
					pthread_mutex_unlock(&atorder_mutex);				
					sem_post(&gsm_sem);
					//检测是否需要绑定
					if (1)//系统自动绑定
					{
						pthread_mutex_lock(&atorder_mutex);
						st_gsm_at.at.state = AT_IDLE;
						set_sim_card("1234",ENABLE);
						unlock_flag = 1;
						auto_bind_flag = 1;
						st_menu.main_interface_index = TS_LOCK_CARD_ENABLE_WAIT;
					}
					else 
					{
						unlock_flag = 1;
						st_menu.main_interface_index = TS_IDLE;						
					}
				}
				else if (st_gsm_at.at.state == AT_ANSWER_LOCKSIM)
				{
					pthread_mutex_unlock(&atorder_mutex);
#ifdef _DBUG_
					printf("this sim card is set lock,pelase beblocking\n");
#endif
					dbug_log(__FILE__,__LINE__,"this sim card is set lock,pelase beblocking\n");			
					//检测系统自动绑定是否开启
					if (1)
					{
						pthread_mutex_lock(&atorder_mutex);
						st_gsm_at.at.state = AT_IDLE;
						input_pin_code(modify_pin);
						st_menu.main_interface_index = TS_UNLOCK_CARD_WAIT;
					}
					else
					{
						auto_bind_flag = 1;
						st_menu.main_interface_index = TS_IDLE;
					}
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
#ifdef _DBUG_
					printf("there is no simcard\n");
#endif
					dbug_log(__FILE__,__LINE__,"there is no simcard\n");
					st_menu.main_interface_index = TS_NO_SIM_CARD;
				}
				break;
			case TS_LOCK_CARD_ENABLE_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
					//	if (auto_bind_flag == 1)
					{
						unlock_flag = 0;
						pin_input_num = 3;
						if (modify_pin[0] != '\0')
						{
							pthread_mutex_lock(&atorder_mutex);
							st_gsm_at.at.state = AT_IDLE;
							if (auto_bind_flag == 1)
							{
								modify_pin_code("1234",modify_pin);
							}
							else 
							{
								// 获取输入的pin
								modify_pin_code("YYYY",modify_pin);
							}
							st_menu.main_interface_index = TS_MACHINE_CARD_BIND_WAIT;
						}
						else
						{
#ifdef _DBUG_
							printf("modify_pin is NULL\n");
#endif
							dbug_log(__FILE__,__LINE__,"modify_pin is NULL\n");
							st_menu.main_interface_index = TS_ERROR;
						}
					}				
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
					if (auto_bind_flag == 1)
					{
						auto_bind_flag = 0;
					}
					unlock_flag = 1;
					pin_input_num --;
					if (pin_input_num == 0)
					{
						st_menu.main_interface_index = TS_USER_INPUT_PUK;
					}
					else
					{
						st_menu.main_interface_index = TS_IDLE;
					}
				}			
				break;
			case TS_MACHINE_CARD_BIND_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
#ifdef _DBUG_
					printf("machine and card bind success\n");
#endif
					dbug_log(__FILE__,__LINE__, "machine and card bind success\n");
					pin_input_num = 3;
					st_menu.main_interface_index = TS_USER_IDLE;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
#ifdef _DBUG_
					printf("machine and card bind failed\n");
#endif
					dbug_log(__FILE__,__LINE__, "machine and card bind failed\n");
					pin_input_num --;
					if (pin_input_num == 0)
					{
						st_menu.main_interface_index = TS_USER_INPUT_PUK;
					}
					else
					{
						st_menu.main_interface_index = TS_IDLE;
					}
				}
				break;

			case TS_UNLOCK_CARD_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
					sem_post(&gsm_sem);
#ifdef _DBUG_
					printf("unlockcard success\n");
#endif
					dbug_log(__FILE__,__LINE__, "unlockcard success\n");
					auto_unlock_failed = 0;
					pin_input_num = 3;
					if (bind_flag == 1)
					{
						bind_flag = 0;
						pthread_mutex_lock(&atorder_mutex);
						st_gsm_at.at.state = AT_IDLE;
						// 获取输入的pin
						modify_pin_code("YYYY",modify_pin);
						st_menu.main_interface_index = TS_MACHINE_CARD_BIND_WAIT;
					}
					else
					{
						st_menu.main_interface_index = TS_USER_IDLE;
					}
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
#ifdef _DBUG_
					printf("unlockcard faild\n");
#endif
					dbug_log(__FILE__,__LINE__, "unlockcard faild\n");
					pin_input_num --;
					if (pin_input_num == 0)
					{
						auto_unlock_failed = 1;
						st_menu.main_interface_index = TS_IDLE;
					}
					else
					{
						st_menu.main_interface_index = TS_USER_INPUT_PUK;
					}					
				}				
				break;

			case TS_USER_IDLE:
				//检测是否禁用pin码
				if (0)
				{
					pthread_mutex_lock(&atorder_mutex);
					st_gsm_at.at.state = AT_IDLE;
					set_sim_card("YYYY",DISABLE);
					st_menu.main_interface_index = TS_USER_DISABLE_LOCK_CARD_WAIT;
				}
				else if (0)// 检测是否修改pin
				{
					pthread_mutex_lock(&atorder_mutex);
					st_gsm_at.at.state = AT_IDLE;
					modify_pin_code("XXXX","YYYY");
					st_menu.main_interface_index = TS_USER_PIN_MODIFY_WAIT;
				}
				break;

			case TS_IDLE:
				//检测是否启用pin
				if (0)
				{
					if (unlock_flag == 1)
					{
						pthread_mutex_lock(&atorder_mutex);
						st_gsm_at.at.state = AT_IDLE;
						set_sim_card("YYYY",ENABLE);
						st_menu.main_interface_index = TS_LOCK_CARD_ENABLE_WAIT;
					}
					else
					{
						if (auto_unlock_failed == 1)
						{
							pthread_mutex_lock(&atorder_mutex);
							st_gsm_at.at.state = AT_IDLE;
							input_pin_code("YYYY");
							bind_flag = 1;
							st_menu.main_interface_index = TS_UNLOCK_CARD_WAIT;
						}
						else 
						{
							pthread_mutex_lock(&atorder_mutex);
							st_gsm_at.at.state = AT_IDLE;
							modify_pin_code("YYYY","XXXX");
							st_menu.main_interface_index = TS_MACHINE_CARD_BIND_WAIT;
						}
					}
				}
				break;
			case TS_USER_DISABLE_LOCK_CARD_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
					pin_input_num = 3;
					st_menu.main_interface_index = TS_IDLE;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
					pin_input_num --;
					if (pin_input_num == 0)
					{
						st_menu.main_interface_index = TS_USER_INPUT_PUK;
					}
					else
					{
						st_menu.main_interface_index = TS_USER_IDLE;
					}
				}							
				break;
			case TS_USER_PIN_MODIFY_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pthread_mutex_unlock(&atorder_mutex);
					pin_input_num = 3;
					st_menu.main_interface_index = TS_USER_IDLE;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
					pin_input_num --;
					if (pin_input_num == 0)
					{
						st_menu.main_interface_index = TS_USER_INPUT_PUK;
					}
					else
					{
						st_menu.main_interface_index = TS_USER_IDLE;
					}
				}
				break;
			case TS_USER_INPUT_PUK:
				//检测是否启用PUK
				if (0)// 如果启用
				{
					pthread_mutex_lock(&atorder_mutex);
					st_gsm_at.at.state = AT_IDLE;
					input_puk_code("XXXX","YYYY");
					st_menu.main_interface_index = TS_USER_DETEC_PUK_WAIT;
				}
				else
				{
					
				}
				break;
			case TS_USER_DETEC_PUK_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					pin_input_num = 3;
					puk_input_num = 10;
					st_menu.main_interface_index = TS_IDLE;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					puk_input_num --;
					if (puk_input_num ==0)
					{
						//卡已报废
						st_menu.main_interface_index = TS_NO_SIM_CARD;
					}
					else
					{
						st_menu.main_interface_index = TS_USER_INPUT_PUK;
					}
				}
				break;
			case TS_NO_SIM_CARD:

				break;
			
			case TS_ERROR:

				break;
				
#if 0
			case TS_INIT:
			//读文件
			if (!getvar_from_sys_file(LOCKCARD_FILE,"lock_flag",&lock_flag))
			{
#ifdef _DBUG_
				printf("get  lock_flag from sysfile error\n");
#endif
				dbug_log(__FILE__,__LINE__, "get lock_flag from sysfile error\n");
				
				break;
			}
			if (atoi(lock_flag) == 2)
			{
				//st_menu.main_interface_index=TS_INIT;
			}
			else if (atoi(lock_flag) == 0)
			{
				st_menu.main_interface_index = TS_PIN_DISABLE;
			}
			else if (atoi(lock_flag) == 1)
			{
				st_menu.main_interface_index = TS_PIN_ENABLE;			
			}
#ifdef _CELL_
			if (!getvar_from_sys_file(LOCKCARD_FILE,"cell_lock_flag",&cell_lock_flag))
			{
#ifdef _DBUG_
				printf("get  lock_flag from sysfile error\n");
#endif
				dbug_log(__FILE__,__LINE__, "get lock_flag from sysfile error\n");
				
				break;
			}
			if (atoi(cell_lock_flag) == 1)
			{
				st_menu.main_interface_index = TS_CELL_ENABLE;	
			}
			else if (atoi(cell_lock_flag) == 0)
			{
				st_menu.main_interface_index = TS_CELL_DISABLE;	
			}
			else if (atoi(cell_lock_flag) == 2)
			{
				st_menu.main_interface_index = TS_INIT;
			}
			break;
			case TS_CELL_ENABLE:
				//if (st_gsm_state.tx_overtime==0)
				{
					pthread_mutex_lock(&atorder_mutex);
					st_gsm_at.at.state = AT_IDLE;
					switch_to_gsm("13");
					st_menu.main_interface_index = TS_CELL_ENABLE_WAIT;
				}
				break;
			case TS_CELL_ENABLE_WAIT:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
					st_gsm_at.at.state = AT_IDLE;
					pthread_mutex_unlock(&atorder_mutex);
					st_menu.main_interface_index = TS_GSM_SCANCELL;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
					pthread_mutex_unlock(&atorder_mutex);
#ifdef _DBUG_
					printf("switch to gsm mode failed\n");
#endif
					dbug_log(__FILE__,__LINE__,"switch to gsm mode failed\n");
					setvar_to_sys_file(LOCKCARD_FILE,"cell_lock_flag","2");
				}
				break;
				
				if (st_gsm_at.at.state == AT_ANSWER_OK)// 还是需要加互斥锁,因为可能tx_overtime!=0,也就是指令还没发
				{
					st_gsm_at.at.state = AT_IDLE;
					st_menu.main_interface_index = TS_GSM_SCANCELL_WAITE;
				}
				else
				{
#ifdef _DBUG_
					printf("switch to gsm failed\n");
#endif
					dbug_log(__FILE__,__LINE__,"switch to gsm failed\n");
				}
				break;
			case TS_GSM_SCANCELL_WAITE:
					
				break;
#endif
			case TS_PIN_ENABLE:
				if (!getvar_from_sys_file(LOCKCARD_FILE,"pin",&pin))
				{
#ifdef _DBUG_
					printf("get pin from sysfile error\n");
#endif
					dbug_log(__FILE__,__LINE__, "get pin from sysfile error\n");
				}
				else if (pin!=NULL)
				{
					if (st_gsm_state.tx_overtime == 0)
					{
						st_gsm_at.at.state = AT_IDLE;
						input_pin_code(pin);
						st_menu.main_interface_index = TS_AUTO_UNLOCKCARD_WAITING;
					}
				}
				break;
			case TS_AUTO_UNLOCKCARD_WAITING:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
#ifdef _DBUG_
					printf("unlockcard success\n");
#endif
					dbug_log(__FILE__,__LINE__, "unlockcard success\n");
					input_nums=3;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
					setvar_to_sys_file(LOCKCARD_FILE,"lock_success","1");
					setvar_to_sys_file(LOCKCARD_FILE,"lock_flag","2");
					setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					st_menu.main_interface_index = TS_INIT;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
#ifdef _DBUG_
					printf("unlockcard faild or lockcard enable\n");
#endif
					dbug_log(__FILE__,__LINE__, "unlockcard faild or lockcard enable\n");
					if (st_gsm_state.tx_overtime == 0)
					{
						st_gsm_at.at.state = AT_IDLE;
						set_sim_card(pin,1);
						st_menu.main_interface_index = TS_AUTO_LOCKCARDENABLE_WAITING;
					}
				}
				break;
			case TS_AUTO_LOCKCARDENABLE_WAITING:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
#ifdef _DBUG_
					printf("lockcard enable success\n");
#endif
					dbug_log(__FILE__,__LINE__, "lockcard enable success\n");
					input_nums=3;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
					setvar_to_sys_file(LOCKCARD_FILE,"lock_success","1");
					setvar_to_sys_file(LOCKCARD_FILE,"lock_flag","2");
					setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					st_menu.main_interface_index= TS_INIT;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
#ifdef _DBUG_
					printf("lockcard enable failed pin code error\n");
#endif
					dbug_log(__FILE__,__LINE__, "lockcard enable failed pin code error\n");
					input_nums--;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
					setvar_to_sys_file(LOCKCARD_FILE,"lock_success","0");
					setvar_to_sys_file(LOCKCARD_FILE,"lock_flag","2");
					setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					st_menu.main_interface_index = TS_INIT;
				}
				break;
			case TS_PIN_DISABLE:
				if (!getvar_from_sys_file(LOCKCARD_FILE,"pin",&pin))
				{
#ifdef _DBUG_
					printf("get pin from sysfile error\n");
#endif
					dbug_log(__FILE__,__LINE__, "get pin from sysfile error\n");
				}
				else if (pin!=NULL)
				{
					if (st_gsm_state.tx_overtime == 0)
					{
						st_gsm_at.at.state = AT_IDLE;
						set_sim_card(pin,0);
						st_menu.main_interface_index = TS_AUTO_LOCKCARDDISABLE_WAITING;
					}
				}
				break;
			case TS_AUTO_LOCKCARDDISABLE_WAITING:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
#ifdef _DBUG_
					printf("lockcard disable success\n");
#endif
					dbug_log(__FILE__,__LINE__, "lockcard disable success\n");
					input_nums=3;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
					setvar_to_sys_file(LOCKCARD_FILE,"lock_success","1");
					setvar_to_sys_file(LOCKCARD_FILE,"lock_flag","2");
					setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					st_menu.main_interface_index = TS_INIT;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
#ifdef _DBUG_
					printf("lockcard disable faild pin code failed\n");
#endif
					dbug_log(__FILE__,__LINE__, "lockcard disable faild pin code failed\n");
					input_nums--;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
					setvar_to_sys_file(LOCKCARD_FILE,"lock_success","0");
					setvar_to_sys_file(LOCKCARD_FILE,"lock_flag","2");
					setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					st_menu.main_interface_index = TS_INIT;
				}
				break;
			case TS_PIN_MODIFY:
				if (!getvar_from_sys_file(LOCKCARD_FILE,"oldpin",&oldpin))
				{
#ifdef _DBUG_
					printf("get  oldpin from sysfile error\n");
#endif
					dbug_log(__FILE__,__LINE__, "get oldpin from sysfile error\n");
				
					break;
				}
#if 1
				imei = get_imei_from_gsm();
				get_mac_address(mac_addr);
				encryp_codec(imei, mac_addr, strlen(mac_addr), modify_pin, 1);
#else
				if (!getvar_from_sys_file(LOCKCARD_FILE,"newpin",&newpin))
				{
#ifdef _DBUG_
					printf("get  newpin from sysfile error\n");
#endif
					dbug_log(__FILE__,__LINE__, "get newpin from sysfile error\n");

					break;
				}
#endif
				if (oldpin!=NULL&&newpin!= NULL)
				{
					if (st_gsm_state.tx_overtime == 0)
					{
						st_gsm_at.at.state =AT_IDLE;
						modify_pin_code(oldpin,newpin);
						st_menu.main_interface_index = TS_AUTO_PINMODIFY_WAITING;
					}
				}
				
				
				break;
			case TS_AUTO_PINMODIFY_WAITING:
				if (st_gsm_at.at.state == AT_ANSWER_OK)
				{
#ifdef _DBUG_
					printf("pinmodify success\n");
#endif
					dbug_log(__FILE__,__LINE__, "pinmodify success\n");
					input_nums = 3;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
					setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					setvar_to_sys_file(LOCKCARD_FILE,"pin_modify_flag","0");
					st_menu.main_interface_index= TS_INIT;
				}
				else if (st_gsm_at.at.state == AT_ERROR)
				{
#ifdef _DBUG_
					printf("pinmodify failed\n");
#endif
					dbug_log(__FILE__,__LINE__, "pinmodify failed\n");
					input_nums--;
					//itoa(input_nums,num,10);
					sprintf(num, "%d", input_nums);
				   	setvar_to_sys_file(LOCKCARD_FILE,"input_nums",num);
					setvar_to_sys_file(LOCKCARD_FILE,"pin_modify_flag","0");
					st_menu.main_interface_index = TS_INIT;
				}
				break;
#endif

#endif
		}
	}
	
	pthread_exit(NULL);
}

void *send_sms_handler(void *arg)
{
	while (1)
	{
		get_sms_flag();				
	}
	pthread_exit(NULL);
}
