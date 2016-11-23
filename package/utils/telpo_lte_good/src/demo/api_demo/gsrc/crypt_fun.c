/*---------------------------------------------------------------------------*/

#include "crypt_fun.h"
#include "des.h"

#define DES_LEN      8

/*加密时数据源不足8位后面补的数据值,现在是以0x00做为填充*/
#define DES_PADDING  0x00



/*******************************************************************************
函数：int crypt_des(unsigned char *Key, unsigned char *InBuff,
               int InLen, unsigned char *OutBuff, char IsEncrypt)

功能：对的消息进行Des的解密,
参数:
       unsigned char *Key      DES的密钥，长度固定是8位
       unsigned char *InBuff   需要加密的输入缓冲区
       int InLen               输入缓冲区的长度
       unsigned char *OutBuff  加密后的输出缓冲区
       char IsEncrypt         是加密还是解密, 0 解密; 1 加密 

返回： 输出缓冲区的长度

设计: Zhou Guo-rui
时间: 20100604
返回：
*******************************************************************************/
int crypt_des(unsigned char *Key, unsigned char *InBuff,
             int InLen, unsigned char *OutBuff, char IsEncrypt)
{
    unsigned char tmp[DES_LEN];
    int n;
    int flag;
    des_context ctx;
    unsigned char *pInBuff;
    unsigned char *pOutBuff;

    flag = 0;
    n = 0;
    memset(&ctx,0,sizeof(des_context));
    pInBuff = InBuff;
    pOutBuff = OutBuff;

    ctx.mode = IsEncrypt;

    if (IsEncrypt == DES_ENCRYPT)
    {
        des_setkey_enc(&ctx,Key);
    }
    else
    {
        des_setkey_dec(&ctx,Key);
    }

    while (!flag)
    {
        if ((InLen - n) > DES_LEN)
        {
            des_crypt_ecb(&ctx,pInBuff,pOutBuff);
            pInBuff += DES_LEN;
            pOutBuff += DES_LEN;
        }
        else
        {
            flag = 1;
            memset(tmp,DES_PADDING,DES_LEN);
            memcpy(tmp,pInBuff, InLen - n);
            des_crypt_ecb(&ctx,tmp,pOutBuff);
        }

        n += DES_LEN;
    }

    return n;
}
