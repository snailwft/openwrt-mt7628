#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <net/if.h> 
#include "gsm.h"

uchar *get_imei_from_gsm( )
{
	return st_gsm_at.imei;	
}

int get_mac_address(char *mac_addr)
{
	int sock_mac;
   struct ifreq ifr_mac;
   //char mac_addr[30]; 
   sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
   if ( sock_mac == -1)
   {
        perror("create socket falise...mac\n");
        return -1;
    }

    memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);

	if ( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
    {
        printf("mac ioctl error\n");
        return -1;
    }

    sprintf(mac_addr,"%02x%02x%02x%02x%02x%02x",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);
	#ifdef _DBUG_
    printf("local mac:%s \n",mac_addr);
	#endif
	dbug_log(__FILE__,__LINE__,"local mac:%s \n",mac_addr);
    close( sock_mac );
    return 0;
}

