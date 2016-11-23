#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/interrupt.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>


#define DEVICE_NAME     "pcmdev"   
#define PCM_MAJOR       0        

#define _DMA_
//#undef _DMA_

#define _IRQ_
//#undef _IRQ_

volatile unsigned long *PCM_GLB_CFG;
volatile unsigned long *PCM_CFG;
volatile unsigned long *PCM_INT_STATUS;
volatile unsigned long *PCM_INT_EN;
volatile unsigned long *PCM_CHA0_FF_STATUS;
volatile unsigned long *PCM_CHA_CFGN;
volatile unsigned long *PCM_FSYNC_CFG;
volatile unsigned long *PCM_CHA_CFG2;

volatile unsigned long *PCM_IP_INFO;
volatile unsigned long *PCM_DIVCOMP_CFG;
volatile unsigned long *PCM_DIVINT_CFG;
volatile unsigned long *PCM_DIGDELAY_CFG;


volatile unsigned long *DMA_SA4;
volatile unsigned long *DMA_DA4;
volatile unsigned long *DMA_SA6;
volatile unsigned long *DMA_DA6;
volatile unsigned long *DMA_CT04;
volatile unsigned long *DMA_CT14;
volatile unsigned long *DMA_CT06;
volatile unsigned long *DMA_CT16;
volatile unsigned long *DMA_UNMASKINT;
volatile unsigned long *DMA_DONEINT;
volatile unsigned long *DMA_GCT;
volatile unsigned long *GDMA_PERIADDR_START0;
volatile unsigned long *GDMA_PERIADDR_END0;
volatile unsigned long *GDMA_PERIADDR_START1;
volatile unsigned long *GDMA_PERIADDR_END1;
volatile unsigned long *GDMA_PERIADDR_START2;
volatile unsigned long *GDMA_PERIADDR_END2;
volatile unsigned long *GDMA_PERIADDR_START3;
volatile unsigned long *GDMA_PERIADDR_END3;
#ifdef _DMA_
#define PCM_CH0_FIFO 0x10002080 
#else
volatile unsigned long *PCM_CH0_FIFO;
#endif

volatile unsigned long *GPIOMODE;

#define BUF_SIZE 320
unsigned int src_phys;
unsigned int dst_phys;
static char *dst=NULL;
static char *src=NULL;

static int dma_config_channel4(void) 
{
	*DMA_SA4=PCM_CH0_FIFO;// source address 
	//*DMA_SA4=src_phys;
	*DMA_DA4=dst_phys;// dist address
#if 1
	*DMA_CT04&=~(1<<6); // dest address incremental
	*DMA_CT04&=~(0x7<<3);// 1DWS 4bytes
	*DMA_CT04|=(BUF_SIZE<<16)|(1<<7)|(0<<1);// BUF_SIZE bytes,source address fix mode,CH_EN=0
	*DMA_CT04&=~(1<<1);//CH_EN=0
	*DMA_CT04&=~(1<<0);//hardware mode
	*DMA_CT14&=~(0x3f<<16);
	*DMA_CT14&=~(0x3f<<8);
	*DMA_CT14|=(1<<22)|(1<<16)|(1<<14)|(32<<8)|(4<<3)|(0<<2);// 2segment,source DMA_REQ0,continous mode ,destination memory, clear CH_MASK of channel 4,destination is mempry avoid coherent
	*DMA_CT14&=~(1<<0);//channel4 is not mask
#endif
#if 0

	*DMA_CT04 = 0;					//	 init 
	*DMA_CT04 |= BUF_SIZE<<16;		//     TARGET_BYTE_CNT 320
	*DMA_CT04 |= 1<<8;				//	CURR_SEGMENT
	*DMA_CT04 |= 0<<7;    	 		//	SOURCE_ADDR_MODE
	*DMA_CT04 |= 0<<6;	   	 		//    DEST_ADDR_MODE
	*DMA_CT04 |= 1<<3;   	 		//    BURST_SIZE (3,4,5BIT)
	*DMA_CT04 |= 1<<2;	  			//	SEGMENT_DONE_INT_EN
	*DMA_CT04 |= 1<<1;	  	 		// 	CH_EN ==>CONT_MODE_EN = 0,TARGET_BYTE_CNT
	*DMA_CT04 |= 0<<0;		 		//	SW_MODE_EN 0 Hardware mode

	*DMA_CT14 = 0;					//	init
	*DMA_CT14 |= 8<<22;			//  	NUM_SEGMENT
	*DMA_CT14 |= 0X20<<16;			// 	SOURCE_DMA_REQ 32:MEMORY
	*DMA_CT14 |= 0<<14;			//	CONT_MODE_EN
	*DMA_CT14 |= 0X20<<8;			//	DEST_DMA_REQ	 32:MEMORY
	*DMA_CT14 |= 4<<3;				//	NEXT_CH2UNMASK 0--n 
	*DMA_CT14 |= 0<<2;				//	COHERENT_INT_EN 
	*DMA_CT14 |= 0<<1;				//	CH_UNMASK_FAIL_INT_EN
	*DMA_CT14 |= 0<<0;				//	CH_MASK	
		
	*GDMA_PERIADDR_START0=0x10002000;
	*GDMA_PERIADDR_END0=0x10002fff;
	*GDMA_PERIADDR_START1=0x10002000;
	*GDMA_PERIADDR_END1=0x10002fff;
	*GDMA_PERIADDR_START2=0x10002000;
	*GDMA_PERIADDR_END2=0x10002fff;
	*GDMA_PERIADDR_START3=0x10002000;
	*GDMA_PERIADDR_END3=0x10002fff;
#endif
	return 0;
}

static int dma_config_channel6(void)
{
   *DMA_SA6=src_phys; // source address
   *DMA_DA6=PCM_CH0_FIFO; // dist address
#if 1
   *DMA_CT06&=~(1<<7);// source address incremental
   *DMA_CT06&=~(0x7<<3);// 1DWS
   *DMA_CT06|=(BUF_SIZE<<16)|(1<<6);// BUF_SIZE bytes,dist address fix mode
   *DMA_CT06&=~(1<<0);// hardware mode
   *DMA_CT16&=~(1<<0);//channel 6 is not mask
   *DMA_CT16|=(1<<22)|(32<<16)|(1<<14)|(1<<8)|(6<<3);// 2segments,continuous mode,clear CH_MASK of channel 6,destination DMA_REQ1

#endif
#if 0
	
    *DMA_CT06 = 0;					//	 init 
    *DMA_CT06 |= BUF_SIZE<<16;		//     TARGET_BYTE_CNT 320
    *DMA_CT06 |= 1<<8;				//	CURR_SEGMENT
    *DMA_CT06 |= 0<<7;    	 		//	SOURCE_ADDR_MODE
    *DMA_CT06 |= 0<<6;	   	 		//    DEST_ADDR_MODE
    *DMA_CT06 |= 1<<3;   	 			//    BURST_SIZE (3,4,5BIT)
    *DMA_CT06 |= 1<<2;	  			//	SEGMENT_DONE_INT_EN
    *DMA_CT06 |= 1<<1;	  	 		// 	CH_EN ==>CONT_MODE_EN = 0,TARGET_BYTE_CNT
    *DMA_CT06 |= 0<<0;		 		//	SW_MODE_EN 0 Hardware mode

    *DMA_CT16 = 0;					//	init
    *DMA_CT16 |= 8<<22;				//  	NUM_SEGMENT
    *DMA_CT16 |= 0X20<<16;			// 	SOURCE_DMA_REQ 32:MEMORY
    *DMA_CT16 |= 0<<14;				//	CONT_MODE_EN
    *DMA_CT16 |= 0X20<<8;			//	DEST_DMA_REQ	 32:MEMORY
    *DMA_CT16 |= 4<<3;				//	NEXT_CH2UNMASK 0--n 
    *DMA_CT16 |= 0<<2;				//	COHERENT_INT_EN 
    *DMA_CT16 |= 0<<1;				//	CH_UNMASK_FAIL_INT_EN
    *DMA_CT16 |= 0<<0;				//	CH_MASK		

#endif
    return 0;
}

static struct class *pcm_drv_class;

#if 1

static int pcm_config(void)
{
	*PCM_GLB_CFG|=(4<<20)|(4<<16)|(1<<0);//4 words,4 words, enable pcm channel 0
	*PCM_CFG&=~(1<<27);//FSYNC is provided by internal circuit,master mode
	*PCM_CFG&=~(0x7<<0);
	*PCM_CFG|=(1<<30)|(1<<26)|(1<<25)|(1<<24)|(3<<0); // internal clock, long fsync ,fsync is high active,Tristate,2.048M
	*PCM_CFG&=~(1<<26);// short fsync
	//*PCM_CFG&=~(1<<24);
	*PCM_INT_EN|=(0xff<<0);//enable all interrupts
	*PCM_CHA_CFGN&=~(0x7<<27);// disable HW converter, raw data(16-bit)
	//*PCM_CHA_CFGN|=(1<<28);//|(1<<27)|(1<<29);
	*PCM_CHA_CFGN&=~(0x3ff<<0);// clear 0~9
	*PCM_CHA_CFGN|=(0<<0);// timeslot start 0
	*PCM_CHA_CFG2&=~(1<<0);// MSB order

	*PCM_FSYNC_CFG&=~(1<<30);// this configuration should be 0 if DTX_TRI=1,falling edge capture data
	*PCM_FSYNC_CFG&=~(1<<28);//falling edge capture FSYNC

	*PCM_FSYNC_CFG|=(1<<31)|(1<<29)|(1<<27)|(1<<0);//enable config fsync,positive drive data,positive drive fsync,1 interval
	//*PCM_FSYNC_CFG&=~(1<<29);
	//*PCM_FSYNC_CFG&=~(1<<27);
	*PCM_DIVCOMP_CFG|=(1<<31)|(0);
	*PCM_DIVINT_CFG |=(117<<0);// 2.048M
	*PCM_GLB_CFG|=(1<<30);
	#ifdef _DMA_
	*PCM_GLB_CFG|=(1<<31)|(1<<30);// enable pcm ,dma
	 #else
	*PCM_GLB_CFG|=(1<<31);// enable pcm ,disable dma
	#endif
	return 0;
}
#endif
static int pcm_drv_open(struct inode *inode, struct file *file)
{
	printk("%s:Hello pcm\n", __FUNCTION__);	// print
	#ifdef _DMA_
	dma_config_channel4();
	dma_config_channel6();
	#endif
	pcm_config();
	return 0;
}

static ssize_t pcm_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	int i=0;	
	int ret = 0;
   	//printk("<1>pcm read!\n");
	//dma_config_channel4();
	#ifdef _DMA_
	//*DMA_CT14&=~(1<<0);//channel4 is not mask
	//*DMA_CT04|=(1<<1); // dma channel4 enable
   	//udelay(10);
	ret = copy_to_user(buf,dst,size);
   	if(ret)
		printk("copy_to_user error\n");
	//*DMA_CT04&=~(1<<1); // dma channel4 disable
	printk("buf=%s\n", buf);
	printk("dst=%s\n", dst);
	printk("ret=%d\n",ret);  
	return size;
	#else
	//printk("pcm read size=%d\n",size);
	for(i=0;i<size;)
	{		
		if(copy_to_user(&buf[i],PCM_CH0_FIFO,4))
			break;
		i+=4;
	}
	
   	//if(ret)
		//printk("copy_to_user error\n");	
	//printk("i=%d\n",i);	
  	return i;
	#endif
}
static ssize_t pcm_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{	
	int i=0;
	//*DMA_CT06&=~(1<<1);
	//dma_config_channel6();
	//*DMA_CT16&=~(1<<0);
	#ifdef _DMA_
	//*DMA_CT06|=(1<<1);//	

	//printk("%s:Hello pcm\n", __FUNCTION__);	// printk

    if(copy_from_user(src,buf,count))
    {
        printk("no enough memory!\n");
        return -1;
    }
	 printk("buf=%s\n", buf);
        printk("src=%s\n", src);
        printk("count=%d\n",count);

	#else
    //printk("<1>pcm write!,count=%d\n",count);
   for(i=0;i<count;)
   {	
	  //printk("<1>pcm write!,count=%d\n",count);
    	if(copy_from_user(PCM_CH0_FIFO,&buf[i],4))
			break;
	  	i+=4;
   }
	//printk("*****i=%d\n",i);
	#endif
	return i;
}

static long pcm_drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static struct file_operations pcm_drv_fops = {
	.owner  	= THIS_MODULE,  
	.open   	= pcm_drv_open,     
	.read	= pcm_drv_read,	   
	.write	= pcm_drv_write,	   
	.unlocked_ioctl = pcm_drv_ioctl,
};

int major;

#ifdef _IRQ_
static irqreturn_t ralink_pcm_interrupt(int irq, void *irqaction) 
{ 
    int ret;

    printk("interrupt handler...\n"); 
   
     //wake_up_interruptible(&key_waitq); /*»½ÐÑµÈ´ý¶ÓÁÐ*/

    printk("interrupt wake up...\n"); 
        
    return IRQ_RETVAL(IRQ_HANDLED); 
}
#endif


static int __init pcm_drv_init(void)
{
	
	#ifdef _DMA_
	src = (char *)dma_alloc_coherent(NULL,BUF_SIZE,&src_phys,GFP_KERNEL);
	if(src==NULL)
	{
		dma_free_coherent(NULL,BUF_SIZE,src,src_phys);
		printk("get memory failed \n");
		return -1;
	}

	dst=(char *)dma_alloc_coherent(NULL, BUF_SIZE,&dst_phys,GFP_KERNEL);//
	if(dst==NULL)
	{
		dma_free_coherent(NULL,BUF_SIZE,dst,dst_phys);
		printk("get memory failed \n");
		return -1;
	}

	#endif

	major = register_chrdev(PCM_MAJOR, DEVICE_NAME, &pcm_drv_fops);
	if (major < 0)
	{
		printk(DEVICE_NAME " can't register major number\n");
		return major;
	}
	
	#ifdef _IRQ_
	int ret=0;
	ret = request_irq(4,ralink_pcm_interrupt , IRQF_DISABLED, DEVICE_NAME, NULL); 
	if(ret)
		return ret;
	#endif
	
	pcm_drv_class = class_create(THIS_MODULE, "pcm");
	device_create(pcm_drv_class, NULL, MKDEV(major, 0), NULL, "pcm");// /dev/pcm
	
	printk("%s:Hello pcm\n", __FUNCTION__);	// printk

	#if 1
	GPIOMODE=(volatile unsigned long *)ioremap(0x10000060,4);

	PCM_GLB_CFG=(volatile unsigned long *)ioremap(0x10002000,4);
	PCM_CFG=(volatile unsigned long *)ioremap(0x10002004,4);
	PCM_INT_STATUS=(volatile unsigned long *)ioremap(0x10002008,4);
	PCM_INT_EN=(volatile unsigned long *)ioremap(0x1000200c,4);
	PCM_CHA0_FF_STATUS=(volatile unsigned long *)ioremap(0x10002010,4);
	PCM_CHA_CFGN=(volatile unsigned long *)ioremap(0x10002020,4);
	PCM_FSYNC_CFG=(volatile unsigned long *)ioremap(0x10002030,4);
	PCM_CHA_CFG2=(volatile unsigned long *)ioremap(0x10002034,4);
	PCM_IP_INFO=(volatile unsigned long *)ioremap(0x10002040,4);
	PCM_DIVCOMP_CFG=(volatile unsigned long *)ioremap(0x10002050,4);
	PCM_DIVINT_CFG=(volatile unsigned long *)ioremap(0x10002054,4);
	PCM_DIGDELAY_CFG=(volatile unsigned long *)ioremap(0x10002060,4);
	#ifndef _DMA_
	PCM_CH0_FIFO=(volatile unsigned long *)ioremap(0x10002080,4);
	#endif
	
	*GPIOMODE&=~(0x3<<6);
	*GPIOMODE|=(1<<7); //pcm function
	#endif

	DMA_SA4=(volatile unsigned long *)ioremap(0x10002840,4);
	DMA_DA4=(volatile unsigned long *)ioremap(0x10002844,4);
	DMA_SA6=(volatile unsigned long *)ioremap(0x10002860,4);
	DMA_DA6=(volatile unsigned long *)ioremap(0x10002864,4);
	DMA_CT04=(volatile unsigned long *)ioremap(0x10002848,4); 
	DMA_CT14=(volatile unsigned long *)ioremap(0x1000284C,4);
	DMA_CT06=(volatile unsigned long *)ioremap(0x10002868,4); 
	DMA_CT16=(volatile unsigned long *)ioremap(0x1000286C,4);
	DMA_UNMASKINT=(volatile unsigned long *)ioremap(0x10002a00,4);
	DMA_DONEINT=(volatile unsigned long *)ioremap(0x10002a04,4);
	DMA_GCT=(volatile unsigned long *)ioremap(0x10002a20,4);

	GDMA_PERIADDR_START0=(volatile unsigned long *)ioremap(0x10002a30,4);
	GDMA_PERIADDR_END0=(volatile unsigned long *)ioremap(0x10002a34,4);
	GDMA_PERIADDR_START1=(volatile unsigned long *)ioremap(0x10002a38,4);
	GDMA_PERIADDR_END1=(volatile unsigned long *)ioremap(0x10002a3c,4);
	GDMA_PERIADDR_START2=(volatile unsigned long *)ioremap(0x10002a40,4);
	GDMA_PERIADDR_END2=(volatile unsigned long *)ioremap(0x10002a44,4);
	GDMA_PERIADDR_START3=(volatile unsigned long *)ioremap(0x10002a48,4);
	GDMA_PERIADDR_END3=(volatile unsigned long *)ioremap(0x10002a4c,4);

	return 0;
}



static void __exit pcm_drv_exit(void)
{
	unregister_chrdev(major, "pcm");	
	device_destroy(pcm_drv_class, MKDEV(major, 0));	
	class_destroy(pcm_drv_class);	 


	iounmap(GPIOMODE);
	iounmap(PCM_GLB_CFG);
	iounmap(PCM_CFG);
	iounmap(PCM_INT_STATUS);
	iounmap(PCM_INT_EN);
	iounmap(PCM_CHA0_FF_STATUS);
	iounmap(PCM_CHA_CFGN);
	iounmap(PCM_FSYNC_CFG);
	iounmap(PCM_CHA_CFG2);
	iounmap(PCM_IP_INFO);
	iounmap(PCM_DIVCOMP_CFG);
	iounmap(PCM_DIVINT_CFG);
	iounmap(PCM_DIGDELAY_CFG);
	#ifndef _DMA_
	iounmap(PCM_CH0_FIFO);
	#endif
	
	iounmap(DMA_SA4);
	iounmap(DMA_DA4);
	iounmap(DMA_SA6);
	iounmap(DMA_DA6);
	iounmap(DMA_CT04);
	iounmap(DMA_CT14);
	iounmap(DMA_CT06);
	iounmap(DMA_CT16);
	iounmap(DMA_UNMASKINT);
	iounmap(DMA_DONEINT);
	iounmap(DMA_GCT);
	iounmap(GDMA_PERIADDR_START0);
	iounmap(GDMA_PERIADDR_END0);
	iounmap(GDMA_PERIADDR_START1);
	iounmap(GDMA_PERIADDR_END1);
	iounmap(GDMA_PERIADDR_START2);
	iounmap(GDMA_PERIADDR_END2);
	iounmap(GDMA_PERIADDR_START3);
	iounmap(GDMA_PERIADDR_END3);

	
	dma_free_coherent(NULL,BUF_SIZE,src,src_phys);
	dma_free_coherent(NULL,BUF_SIZE,dst,dst_phys);
	
	#ifdef _IRQ_
	free_irq(4,NULL);
	#endif


	printk("%s:Hello pcm\n", __FUNCTION__);	// printk
}

module_init(pcm_drv_init);
module_exit(pcm_drv_exit);

MODULE_AUTHOR("http://www.pcm.com");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("MT7620 FIRST Driver");
MODULE_LICENSE("GPL");

