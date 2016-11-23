#include "slic_hand.h"
#include "user_handler.h"
#include "gsm.h"

void set_slic_state(char slic_state)
{
	if (slic_state == SLIC_OFF_HOOK)
	{
		st_slic_state.hook = SLIC_OFF_HOOK;//摘机
		st_slic_state.offhook_time = 10; // 100ms
	}
	else
	{
		st_slic_state.hook = SLIC_ON_HOOK;// 挂机
		st_slic_state.onhook_time = 100; // 1000ms
	}
}

char get_slic_state(void)
{
	if (st_slic_state.hook == SLIC_OFF_HOOK)
	{
		return SLIC_OFF_HOOK;
	}
	else
	{
		return SLIC_ON_HOOK;
	}
}

void slic_init()
{
	memset(&st_slic_state,0,sizeof(st_slic_state));	
}

void slic_lamp()
{
	if (slic_light == LIGHT_UP)
	{
		ioctl(fd4, LIGHT_UP);
	}
	else if (slic_light == LIGHT_DOWN)
	{
		ioctl(fd4, LIGHT_DOWN);
	}
	else if (slic_light == LIGHT_BLINK_SLOW)
	{		
		ioctl(fd4, LIGHT_UP);
		usleep(500000);		
		ioctl(fd4, LIGHT_DOWN);
		usleep(500000);		
	}
	else if (slic_light == LIGHT_BLINK_FAST)
	{
		ioctl(fd4, LIGHT_UP);
		usleep(100000);		
		ioctl(fd4, LIGHT_DOWN);
		usleep(100000);	
	}
	return ;
}

void *slic_lamp_handler(void *arg)
{
	while (1)
	{
		slic_lamp();	
	}
	pthread_exit(NULL);
}

int slic_pcm_read(int fd,char *buf,int size)
{
	int len;
	len=read(fd,buf,size);
	#if 0
	if (len<0)
	{
		return -1;
	}
	#endif
   return len;
}

int slic_pcm_write(int fd,char *buf,int size)
{
	int len;
	len = write(fd,buf,size);
	#if 0
	if (len<0)
	{
		printf("pcm write error\n");
		return -1;
	}
	#endif
   return len;
}

void *proslic_audio(void *arg)
{
	#if 0
	int res,i;
	memset(audiobuf,0,sizeof(audiobuf));
	while (1)
	{
		if (pcm_flag==1)
		{
			//pthread_mutex_lock(&audio_mutex);
			printf("proslic_audio\n");		
			//memset(audiobuf,0,sizeof(audiobuf));
		#ifdef DMA_CFG
		//res=slic_pcm_read(fd3,audiobuf,32);	// 获取slic audio
		
		#else
		//for(i=0;i<80;i++)
		{
			res=slic_pcm_read(fd2,audiobuf,320);
			printf("res=%d\n",res);
			//slic_pcm_write(fd2,audiobuf,res); //先读出来再发送声音很差
			//fseek(fp,0,SEEK_END);
			fwrite(audiobuf,res,1,fp);
			
			//slic_pcm_read(fd2,audiobuf,320);	// 获取slic audio
			//slic_pcm_write(fd3,audiobuf,32); // 发送到slic
		}
		//pthread_mutex_unlock(&audio_mutex);
		#endif
		#if 0
		//printf("res=%d\n",res);
		if (res<0)
			printf("pcm read error\n");
		if (res>0)
		{
			#if 0
			for(i=0;i<res;i++)
			{
				printf("audiobuf[%d]=%02x\t",i,audiobuf[i]);
			}
			printf("\n");
			#endif
			
		}
		
		#endif
		//slic_pcm_write(fd2,audiobuf,res); // 发送到gsm
	 }
	}
	#endif
	#if 0
	int res,i;
	//FILE *fp;
	uchar audiobuf[1024];
	//fp=fopen("./audio","w+");
	if (fp==NULL)
	{
		printf("fopen failed\n");
		pthread_exit(NULL);
	}
	while (1)
	{
		//pthread_mutex_lock(&recv_mutex);
		//sleep(3);
		memset(audiobuf,0,sizeof(audiobuf));
		//printf("hello world\n");
		res=slic_pcm_read(fd3,audiobuf,320);
		if (res==320)
		{
			//fseek(fp,0,SEEK_CUR);
			fwrite(audiobuf,res,1,fp);			
			for(i=0;i<res;i++)
			{
				printf("audiobuf[%d]=%02x\t",i,audiobuf[i]);
			}
		}
	}
	#endif

		#if 0
	int res,i;

	uchar audiobuf[1024];
	while (1)
	{
		if (!feof(fp))
		{
			printf("hello\n");
			memset(audiobuf,0,sizeof(audiobuf));
			fread(audiobuf,1024,1,fp);
			slic_pcm_write(fd3, audiobuf, 1024);
		}
		fseek(fp,0,SEEK_SET);
	}
	#endif

	pthread_exit(NULL);
}

void *proslic_audio_write(void *arg)
{
	char buf[320];
	int res;
	while (1)
	{
		if (pcm_flag)
		{
			//pthread_mutex_lock(&audio_mutex);
			#if 1
			while (!feof(fp)) // 结束返回非0值
			{
				printf("proslic_audio_write\n");
				memset(buf,0x0,320);
				res=fread(buf,320,1,fp);
				//uart_send(fd2,buf,res);			
				slic_pcm_write(fd3,buf,320);
			}
			fseek(fp,0,SEEK_SET);
			#endif
			#if 0
			if (strlen(audiobuf))// 如果为空就是真
			{
				printf("hello world\n");
				slic_pcm_write(fd2,audiobuf,strlen(audiobuf));
			}
			#endif
		//	pthread_mutex_unlock(&audio_mutex);
		}
	}
	prhread_exit(NULL);
}

