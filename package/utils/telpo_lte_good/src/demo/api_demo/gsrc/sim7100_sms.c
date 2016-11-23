#include "sim7100_sms.h"

/************************************************************************************************************************* 
*函数         :   u32 GSM_StringToDec(char *pStr, u8 NumDigits) 
*功能         :   将10进制样式字符串转换为整型数(必须保证完全为数字字符) 
*参数         :   pStr:字符串起始指针 
*                   NumDigits:数字位数,10进制数字位数 
*返回         :   转换后的数字 
*依赖         :   无 
*作者         :   cp1300@139.com 
*时间         :   2013-04-30 
*最后修改时间 :   2013-04-30 
*说明         :   比如字符串"1865"转换后为1865,位数为4位 
                    必须保证完全为数字字符 
*************************************************************************************************************************/  
u32 GSM_StringToDec(char *pStr, u8 NumDigits)  
{  
    u32 temp;  
    u32 DEC = 0;  
    u8 i;  
    u8 j;  
      
    NumDigits = (NumDigits > 10) ? 10 : NumDigits;   //最大支持10位10进制数  
      
    for(i = 0;i < NumDigits;i ++)  
    {  
        temp = pStr[i] - '0';  
        if(temp > 9)         //只能是数字范围  
            return 0;  
        for(j = 1;j < (NumDigits - i);j ++)  
        {  
            temp *= 10;  
        }  
        DEC += temp;  
    }  
    return DEC;  
}  

/************************************************************************************************************************* 
*函数         :   u32 GSM_StringToHex(char *pStr, u8 NumDigits) 
*功能         :   将16进制样式字符串转换为16进制整型数(必须保证字符串字母都是大写) 
*参数         :   pStr:字符串起始指针 
*                   NumDigits:数字位数,16进制数字位数 
*返回         :   转换后的数字 
*依赖         :   无 
*作者         :   cp1300@139.com 
*时间         :   2013-04-30 
*最后修改时间 :   2013-10-17 
*说明         :   比如字符串"A865"转换后为0xA865,位数为4位 
                    必须保证字符串字母都是大写 
*************************************************************************************************************************/  
static u32 GSM_StringToHex(char *pStr, u8 NumDigits)  
{  
    u8 temp;  
    u32 HEX = 0;  
    u8 i;  
      
    NumDigits = (NumDigits > 8) ? 8 : NumDigits; //最大支持8位16进制数  
      
    for(i = 0;i < NumDigits;i ++)  
    {  
        HEX <<= 4;  
        temp = pStr[i];  
        temp = (temp > '9') ? temp - 'A' + 10 : temp - '0';  
        HEX |= temp;  
    }  
    return HEX;  
}  

/************************************************************************************************************************* 
*函数         :   static u8 ChartoPhoneNum(char *pChar, char *pNum, u8 CharLen) 
*功能         :   将字符转换为电话号码 
*参数         :   pCHAR:字符缓冲区指针 
*                   pNum:电话号码指针 
*                   charLen:字符号码长度 
*返回         :   电话长度 
*依赖         :   底层宏定义 
*作者         :   cp1300@139.com 
*时间         :   2013-04-04 
*最后修改时间 :   2013-10-17 
*说明         :   主要用于电话号码,短信中心号码转换 
*                   如果电话长度为奇数,则将补成偶数,并在最后一个字符的高位保留位0xf; 
*                   转换后为字符 
*************************************************************************************************************************/  
static u8 ChartoPhoneNum(char *pChar, char *pNum, u8 CharLen)  
{  
    u32 i;  
    u8 temp;  
  
    for(i = 0;i < CharLen;i ++)  
    {  
        temp = pChar[i];  
        if(temp == 'F') //还有一位就结束了  
        {  
            pNum[i] =   pChar[i+1];   
            return i + 1;  
        }  
        else if(temp > '9')  //非数字  
        {  
            return 0;   //电话号码格式错误  
        }  
           
        else if(i % 2)  //位数为奇数  
            pNum[i-1] =  temp;  
        else    //位数为偶数  
            pNum[i+1] = temp;     
    }  
    return i;  
}  

// 7-bit编码  
// pSrc: 源字符串指针  
// pDst: 目标编码串指针  
// nSrcLength: 源字符串长度  
// 返回: 目标编码串长度  
static int gsmEncode7bit(const char* pSrc,u8* pDst)  
{  
    int nSrc;        // 源字符串的计数值  
    int nDst;        // 目标编码串的计数值  
    int nChar;       // 当前正在处理的组内字符字节的序号，范围是0-7  
    unsigned char nLeft=0;    // 上一字节残余的数据  
    int nSrcLength = strlen(pSrc);  
  
    // 计数值初始化  
    nSrc = 0;  
    nDst = 0;  
  
    // 将源串每8个字节分为一组，压缩成7个字节  
    // 循环该处理过程，直至源串被处理完  
    // 如果分组不到8字节，也能正确处理  
    while(nSrc<nSrcLength)  
    {  
        // 取源字符串的计数值的最低3位  
        nChar = nSrc & 7;  
  
        // 处理源串的每个字节  
        if(nChar == 0)  
        {  
            // 组内第一个字节，只是保存起来，待处理下一个字节时使用  
            nLeft = *pSrc;  
        }  
        else  
        {  
            // 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节  
            *pDst = (*pSrc << (8-nChar)) + nLeft;  
  
            // 将该字节剩下的左边部分，作为残余数据保存起来  
            nLeft = *pSrc >> nChar;  
            // 修改目标串的指针和计数值 pDst++;  
            //SIM900_debug("%c",*pDst);  
            pDst++;  nDst++;  
        }  
        // 修改源串的指针和计数值  
        pSrc++; nSrc++;  
    }  
  
    //Nleft还有剩余，需要一个自己保留。  
    nChar = nSrc & 7;  
    if(nChar != 0)  
    {  
        *pDst=nLeft;  
        nDst++;   
        pDst++;  
    }  
    //*pDst='\0';  
  
    // 返回目标串长度  
    return nDst;  
}  

// 7-bit解码  
// pSrc: 源编码串指针,7bit编码  
// pDst: 目标字符串指针  
// nSrcLength: 源编码串长度  
// 返回: 目标字符串长度  
static int gsmDecode7bit(const u8 *pSrc, char *pDst, int nSrcLength)  
{  
    int nSrc;        // 源字符串的计数值  
    int nDst;        // 目标解码串的计数值  
    int nByte;       // 当前正在处理的组内字节的序号，范围是0-6  
    unsigned char nLeft;    // 上一字节残余的数据  
          
    // 计数值初始化  
    nSrc = 0;  
    nDst = 0;  
      
    // 组内字节序号和残余数据初始化  
    nByte = 0;  
    nLeft = 0;  
      
    // 将源数据每7个字节分为一组，解压缩成8个字节  
    // 循环该处理过程，直至源数据被处理完  
    // 如果分组不到7字节，也能正确处理  
      
    while(nSrc<nSrcLength)  
    {  
        // 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节  
        *pDst = (((*pSrc) << nByte) | nLeft) & 0x7f;  
        // 将该字节剩下的左边部分，作为残余数据保存起来  
        nLeft = (*pSrc) >> (7-nByte);  
      
        // 修改目标串的指针和计数值  
        pDst++;  
        nDst++;  
      
        // 修改字节计数值  
        nByte++;  
      
        // 到了一组的最后一个字节  
        if(nByte == 7)  
        {  
            // 额外得到一个目标解码字节  
            *pDst = nLeft;  
      
            // 修改目标串的指针和计数值  
            pDst++;  
            nDst++;  
      
            // 组内字节序号和残余数据初始化  
            nByte = 0;  
            nLeft = 0;  
        }  
      
        // 修改源串的指针和计数值  
        pSrc++;  
        nSrc++;  
    }  
      
    *pDst = 0;  //添加结束符        
    // 返回目标串长度  
    return nDst;  
}  

/************************************************************************************************************************* 
* 函数    :   u16 OneUNICODEtoGBK(u16 unicode) 
* 功能    :   将unicode编码转换为GBK编码 
* 参数    :   unicode 
* 返回    :   GBK  
* 依赖    :   底层读写函数 
* 作者    :   cp1300@139.com 
* 时间    :   20120602 
* 最后修改时间 : 20120602 
* 说明    :   需要flash中的码表支持 
            GBK码范围,高8位:0x81~0xfe;低8位:0x40~0xfe 
*************************************************************************************************************************/   
u16 OneUNICODEtoGBK(u16 unicode)  //用二分查找算法  
{  
    u32 offset;  
    u16 temp;  
    //UINT bw;  
    u8 buff[2];  
  
    if(unicode<=0X9FA5)  
    {  
        if(unicode>=0X4E00)  
            offset=unicode-0X4E00;//0x1b87      //0X4E00,汉字偏移起点  
        else  
            return 0x2020;      //不能显示的字符就给两个空格填充,否则乱码  
    }     
    else if(unicode>0X9FA5)//是标点符号  
    {  
        if(unicode<0XFF01||unicode>0XFF61)  
            return 0x2020;//没有对应编码  //不能显示的字符就给两个空格填充,否则乱码  
        offset=unicode-0XFF01+0X9FA6-0X4E00;      
    }  
    offset *= 2;  
  
  
    //f_lseek(&UtoG_File, offset);                        		 //文件指针调到偏移位置
    fseek(UtoG_File, offset, SEEK_SET);
    //if(f_read(&UtoG_File, buff, 2, &bw) != FR_OK)   //读取2字节
    if (fread(buff, 1, 2, UtoG_File) == 0)
    {  
       return 0x2020;  
    }  
  
    temp = buff[0];  
    temp <<= 8;  
    temp += buff[1];  
    return temp;    //返回找到的编码                  
}  
  

/************************************************************************************************************************* 
*函数         :   u32 UnicodeStrToString(u8 *pucode,char *pStr,u32 SrtLen) 
*功能         :   将字符unicode转换为字符串 
*参数         :   pucode:转换结果缓冲区 
*                   pStr:字符缓冲区指针 
*                   SrtLen:字符串字节长度 
*返回         :   转换成为字符后的长度 
*依赖         :   u16 OneUNICODEtoGBK(u16 unicode); 
*作者         :   cp1300@139.com 
*时间         :   2013-04-04 
*最后修改时间 :   2013-10-26 
*说明         :   用于将PUD格式短信解析,本函数不添加字符串结束符 
                    2013-10-26:解决短信中句号无法解析 
*************************************************************************************************************************/  
static u32 UnicodeStrToString(u8 *pucode,char *pStr,u32 SrtLen)  
{  
    u32 i;  
    u16 temp;  
    u32 cnt = 0;  
    u8 H,L;  
  
    for(i = 0;i < SrtLen;i+=4)  
    {  
        if(pucode[i] == '0')    //0  
        {  
            H = pucode[i+2];  
            L = pucode[i+3];  
            H = (H > '9') ? H - 'A' + 10 : H - '0';  
            L = (L > '9') ? L - 'A' + 10 : L - '0';    
            pStr[cnt++] = (H << 4) + L;     
        }  
        else  
        {  
            H = pucode[i];  
            L = pucode[i+1];  
            H = (H > '9') ? H - 'A' + 10 : H - '0';  
            L = (L > '9') ? L - 'A' + 10 : L - '0';    
            temp = (H << 4) + L;  
            temp <<= 8;  
            H = pucode[i+2];  
            L = pucode[i+3];  
            H = (H > '9') ? H - 'A' + 10 : H - '0';  
            L = (L > '9') ? L - 'A' + 10 : L - '0';    
            temp |= (H << 4) + L;  
            switch(temp)  
            {  
                case 0x3002:    //句号无法显示,转换为GBK编码句号  
                    temp = 0xA1A3;
					break;//'。';  break;  
                default :   
                    temp = OneUNICODEtoGBK(temp);
					break; //编码转换  
            }  
            pStr[cnt++] = temp >> 8 ;  //buf[0]
            pStr[cnt++] = temp & 0xff;  //buf[1]
        }  
    }  
    return cnt;  
}  

/************************************************************************************************************************* 
*函数         :   bool gsm_parsepdu_sms(char *pPDU,char *pSMS,u32 len,sms_info *pInfo) 
*功能         :   解析一条PDU格式短信                           读短信
*参数         :   pPDU:短信PDU数据缓冲区指针 
*                   pSMS:解析后的短信缓冲区指针 
*                   pInfo:短信信息指针 
*返回         :   TRUE:成功;FALSE:失败 
*依赖         :   void UnicodeToGBK(u16 *pUnicode, u16 *pGBK, u32 cnt); 
*作者         :   cp1300@139.com 
*时间         :   2013-04-04 
*最后修改时间 :   2013-05-01 
*说明         :   无 
*************************************************************************************************************************/  
bool gsm_parsepdu_sms(char *pPDU, char *pSMS, sms_info *pInfo)  
{  
    u16 cnt = 0;  
    u16 temp;  
    char *p;  
    u16 SMS_Size;  
    //获取短信中心号码长度  
    temp = GSM_StringToHex(&pPDU[cnt], 2);  //将16进制样式字符串转换为整型数  
    cnt += 2;           //跳过前面的短信中心号码长度字节  
    cnt += temp*2;      //跳过前面的短信中心信息  
      
    //解析PDU数据  RT  UDHI  SRI -  -  MMS   MTI  MTI  //UDHI为1,代表用户数据有头部信息,用于标识短信拆分信息  
    pInfo->PDU = GSM_StringToHex(&pPDU[cnt], 2); //将16进制样式字符串转换为整型数//PDU数据  
    cnt += 2;           //跳过PDU头数据字节  
  
    //计算发送短信的号码的长度  
    temp = GSM_StringToHex(&pPDU[cnt], 2);  //将16进制样式字符串转换为整型数  
    cnt += 2;           //跳过电话号码长度字节  
    cnt += 2;           //跳过地址类型,常为"91",一字节  
    pInfo->NumLen = ChartoPhoneNum((char *)&pPDU[cnt], (char *)&(pInfo->NumBuff[0]), (temp > SMS_NUM_LEN_MAX - 2) ? (SMS_NUM_LEN_MAX - 2) : temp); //转换发送号码  
    pInfo->NumBuff[pInfo->NumLen] = 0;    //结束符  
 
    cnt += (temp%2) ? (temp+1) : temp;  //跳过发送号码长度的字节数  
    cnt+=2; //跳过PID,2Byte  
    pInfo->DSC = GSM_StringToHex(&pPDU[cnt], 2); //获取DSC信息  
    cnt+=2; //跳过DSC,2Byte  
    //cnt+=2;   //跳过VP,2B   //没有这个标志  
    //cnt+=2;   //跳过UDL,2B//没有这个标志  
  
      
    pInfo->Timer.Year = (pPDU[cnt + 1] - '0') * 10 + (pPDU[cnt] - '0');      cnt += 2;   //年  
    pInfo->Timer.Month = (pPDU[cnt + 1] - '0') * 10 + (pPDU[cnt] - '0'); cnt += 2;   //月
    pInfo->Timer.Day = (pPDU[cnt + 1] - '0') * 10 + (pPDU[cnt] - '0');       cnt += 2;   //日  
    pInfo->Timer.Hour = (pPDU[cnt + 1] - '0') * 10 + (pPDU[cnt] - '0');      cnt += 2;   //时  
    pInfo->Timer.Minute = (pPDU[cnt + 1] - '0') * 10 + (pPDU[cnt] - '0');    cnt += 2;   //分  
    pInfo->Timer.Second = (pPDU[cnt + 1] - '0') * 10 + (pPDU[cnt] - '0');    cnt += 2;   //秒                                  
    cnt += 2;   //跳过时区2字节  
    SMS_Size = GSM_StringToHex(&pPDU[cnt], 2);      //计算短信字符数量,不管英文,中文都算一个字符   
    cnt += 2;                                           //跳过短信长度字节,2B  
  
    if(pInfo->PDU & 0x40)    //用户数据有头部信息,标识短信已经被分割为几条  
    {  
        cnt += 8;           //跳过前面8个数据,只要后面的4个,标识  
        SMS_Size -= 12;     //短信长度减去偏移  
          
        pInfo->AllNum = GSM_StringToHex(&pPDU[cnt], 2);//计算总分割数  
        cnt += 2;       //跳过2B的总数  
        pInfo->PreNum = GSM_StringToHex(&pPDU[cnt], 2);//计算当前位置  
        cnt += 2;       //跳过2B的当前位置  
    }  
    else  
    {  
        pInfo->AllNum = pInfo->PreNum = 0;    //短信没有被分割  
    }  
      
    //DCS 00:7BIT编码;08:UCS2编码;04:8bit编码  
    switch((pInfo->DSC) & 0x0f)  
    {  
        case 0x00:  //7bit编码  
        {  
            pInfo->SMS_Size = (SMS_Size > 160) ? 160 : SMS_Size;      //短信大小  
            pInfo->TEXT_MODE = 1;      
            SMS_Size = (SMS_Size * 7 / 8) + (((SMS_Size * 7) % 8) ? 1 : 0);//计算短信占用空间大小  
            pPDU += cnt;  
            for(temp = 0;temp < SMS_Size;temp ++)                //将PDU数据转换为16进制数据  
            {  
                pPDU[temp] = GSM_StringToHex(&pPDU[temp << 1], 2);    //B数据转换为PDU格式后占用2B  
            }  
            gsmDecode7bit((u8 *)pPDU, (char *)pSMS, SMS_Size);  //7bit->8bit,数据长度会发生变化  
          
        }break;  
        case 0x04:  //8bit编码  
        {  
            printf("sms not support\n");
            return FALSE;  
        }  
        case 0x08:  //UCS2编码  
        {   
            pInfo->TEXT_MODE = 0;      
            SMS_Size = (SMS_Size > 140) ? 140 : SMS_Size;        //短信字符限制为140B  
            //UNICODE PDU转换为字符串 --> GBK,返回短信大小,每个短信字符占用2字节,每个字节转换为PDU后占用2B  
            pInfo->SMS_Size = UnicodeStrToString((u8 *)pPDU+cnt,(char *)pSMS,SMS_Size<<1);       
        }break;  
        default:
			printf("unknown format\n");
			return FALSE;  
    }  
  
    pSMS[pInfo->SMS_Size] = '\0';                //添加结束符       
      
    return TRUE;  
}

/************************************************************************************************************************* 
* 函数                :   static u16 GSM_GetU2SCharOffset(char *pBuff,u16 CharNum) 
* 功能                :   计算指定字符的偏移位置 
* 参数                :   pBuff:字符串缓冲区; 
                        CharNum:字符偏移 
* 返回                :   字符串大小 
* 依赖                :   无 
* 作者                :   cp1300@139.com 
* 时间                :   2013-10-25 
* 最后修改时间    :   2013-10-25 
* 说明                :   计算指定数量的字符(不分中英文)的大小,比如PDU,U2S模式下,短信只能有70个字符,但是不分中英文 
                        此时英文只占用一个字节,但是中文占用2个字节 
*************************************************************************************************************************/  
static u16 GSM_GetU2SCharOffset(char *pBuff,u16 CharNum)  
{  
    u16 i;  
    u16 cnt = 0;  
      
  
    for(i = 0;i < CharNum;)  
    {  
        if((pBuff[i] & 0xff) >= 0x80) //中文  
        {  
            cnt +=2;  
            i +=2;  
        }  
        else if(pBuff[i] == 0)  //字符串结束  
        {  
            break;  
        }  
        else                    //ASCII  
        {  
            cnt += 1;  
            i ++;  
        }  
    }  
    return cnt;  
}  

/************************************************************************************************************************* 
* 函数                :   bool GSM_CheckNotASCII(char *pBuff,u16 Len) 
* 功能                :   检查字符串中是否含有非ASCII编码 
* 参数                :   pBuff:字符串缓冲区;Len:长度 
* 返回                :   FALSE:字符串全部为ASCII编码;TRUE:字符串含有非ASCII编码,一般为汉字编码 
* 依赖                :   无 
* 作者                :   cp1300@139.com 
* 时间                :   2013-10-25 
* 最后修改时间    :   2013-10-25 
* 说明                :   用于选择发送短信的模式,选择U2S或者7BIT编码 
*************************************************************************************************************************/  
bool GSM_CheckNotASCII(char *pBuff,u16 Len)  
{  
    u16 i;  
      
    for(i = 0;i < Len;i ++)  
    {  
        if((pBuff[i] & 0xff) >= 0x80)  
            return TRUE;  
    }  
    return FALSE;  
}  

//PDU模式短信限制长度,最大70个字符,不分中英文  
//返回:限制之后的字符个数  
u32 GSM_PDUStrRes(char *pStr)  
{  
    u32 n = 0;  
  
    while(*pStr != 0)  
    {  
        n ++;  
        if(n == 71)  
        {  
            //SIM900_debug("PDU模式短信长度超出70B,强制为70B!\r\n");  
            *pStr = 0;      //强制添加结束符  
            return n;  
        }  
        if((u8)*pStr < 0x80) //ASCII  
        {  
            pStr ++;  
        }  
        else if((u8)*pStr > 0x80)    //中文  
        {             
            pStr += 2;  
        }  
    }       
    return n;  
}  

/************************************************************************************************************************* 
*函数         :   void GSM_HexToString(u32 HexNum,c har *pStr, u8 NumDigits) 
*功能         :   将整型数字转换为16进制样式字符串(字母为大写,不带结束符) 
*参数         :   HexNum:16进制数字 
                    pStr:字符缓冲区指针 
*                   NumDigits:数字位数,16进制数字位数 
*返回         :   无 
*依赖         :   无 
*作者         :   cp1300@139.com 
*时间         :   2013-04-30 
*最后修改时间 :   2013-04-30 
*说明         :   比如字符串0xA865转换后为"A865",位数为4位 
*************************************************************************************************************************/  
static void GSM_HexToString(u32 HexNum,char *pStr, u8 NumDigits)  
{  
    u8 temp;  
    u8 i;  
      
    NumDigits = (NumDigits > 8) ? 8 : NumDigits; //最大支持8位16进制数  
      
    for(i = 0;i < NumDigits;i++)  
    {  
        temp = 0x0f & (HexNum >> (4 * (NumDigits - 1 - i)));  
        temp = (temp > 0x09) ? (temp - 0x0A + 'A') : (temp + '0');  
        pStr[i] = temp;  
    }  
}  

/************************************************************************************************************************* 
* 函数    :   u16 OneGBKtoUNICODE(u16 GBKCode) 
* 功能    :   将GBK编码转换为unicode编码 
* 参数    :   GBK  
* 返回    :   unicode 
* 依赖    :   底层读写函数 
* 作者    :   cp1300@139.com 
* 时间    :   20120602 
* 最后修改时间 : 20120602 
* 说明    :   需要flash中的码表支持 
            GBK码范围,高8位:0x81~0xfe;低8位:0x40~0xfe 
*************************************************************************************************************************/   
u16 OneGBKtoUNICODE(u16 GBKCode)  // 必须要有码表
{  
    u8 ch,cl;  
    //UINT bw;  
    u16 data;  
  
    ch = GBKCode >> 8;  
    cl = GBKCode & 0x00ff;  
  
    ch -= 0x81;  
    cl -= 0x40;  
      
    //f_lseek(&GtoU_File, (ch*0xbf+cl)*2);                        				//文件指针调到偏移位置  
    fseek(GtoU_File, (ch*0xbf+cl)*2, SEEK_SET);
    //if(f_read(&GtoU_File, (u8 *)&data, 2, &bw) != FR_OK)        //读取2字节  
   	if (fread((u8 *)&data, 1, 2, GtoU_File) == 0)
    {  
       return 0x1fff;  
    }  
      
    return (ch<=0x7d && cl<=0xbe) ? data : 0x1fff;  
    
   /*   
   ch = GBKCode >> 8; 
    cl = GBKCode & 0x00ff; 
 
    ch -= 0x81; 
    cl -= 0x40;  
    return (ch<=0x7d && cl<=0xbe) ? wUnicodes[ch*0xbf+cl] : 0x1fff;      
    */  
  
}  

/************************************************************************************************************************* 
*函数         :   static u32 StringToUnicodeStr(char *pStr, char *pucode,u32 SrtLen) 
*功能         :   将字符串转换为unicode,并存储为16进制样式的字符串 
*参数         :   pStr:字符缓冲区指针 
*                   pucode:转换结果缓冲区 
*                   SrtLen:字符串字节长度 
*返回         :   转换成为字符后的长度 
*依赖         :   u16 OneGBKtoUNICODE(u16 GBKCode) 
*作者         :   cp1300@139.com 
*时间         :   2013-04-04 
*最后修改时间 :   2013-10-17 
*说明         :   用于将短信内容转换为PUD格式,本函数不添加字符串结束符 
*                   如"a,b,c"--->"0,0,6,1,0,0,6,2,0,0,6,3" 
*                   输出缓冲区至少为输入的4倍 
*************************************************************************************************************************/  
static u32 StringToUnicodeStr(char *pStr, char *pucode,u32 SrtLen)  
{  
    u32 i;  
    u16 temp;  
    u8 m;  
    u8 chTmp= 0;  
    u32 cnt = 0;  

    for(i = 0;i < SrtLen;i ++)  
    {      	
		if( (pStr[i]&0xff) < 0x80)   //ASCII  
		{  
			temp = pStr[i];  
		}  
		else                //GBK  
		{  
			temp = pStr[i++];  
			temp <<= 8;  
			temp += pStr[i]&0xff;   
			temp = OneGBKtoUNICODE(temp);  
		}  

		for(m = 0; m <= 12; m+=4) // m <=8; m+=8  
		{  
			chTmp = (temp >> (12-m)) & 0x0F;            //先取高位   这个地方是不是有问题  8-m & 0xff
			if(chTmp > 0x09)   //! 0x0A-0x0F  
			pucode[cnt ++] = chTmp-0x0A+'A';       //! 'A'-'F'  
			else                                 //! 0x00-0x09  
			pucode[cnt ++] = chTmp-0x00+'0';       //! '0'-'9'  
		}  
    }  
    return cnt;  // 字符数增大2倍了
}

/************************************************************************************************************************* 
*函数         :   static u8 PhoneNumtoPDUChar(u8 *pNum, char *pCHAR,u8 NumLen) 
*功能         :   将电话号码字符转换为PDU要求的字符 
*参数         :   pNum:电话号码指针 
*                   pChar:字符缓冲区指针 
*                   NumLen:电话号码长度 
*返回         :   字符长度 
*依赖         :   底层宏定义 
*作者         :   cp1300@139.com 
*时间         :   2013-04-04 
*最后修改时间 :   2013-10-17 
*说明         :   主要用于电话号码,短信中心号码转换 
*                   如果长度为奇数,则将补成偶数,并在最后一个字符的高位保留位0xf; 
*                   本函数不添加结束符 
*************************************************************************************************************************/  
static u8 PhoneNumtoPDUChar(u8 *pNum, char *pChar,u8 NumLen)  
{  
    u8 i;  
    u8 temp;  
  
    for(i = 0;i < NumLen;i ++)  
    {  
        temp = (pNum[i]+'0') & 0x0f;  
        if(i % 2)   //位数为奇数  
            pChar[i-1] = (temp > 9) ? ('a' + temp - 10) :( temp + '0');  
        else        //位数为偶数  
            pChar[i+1] = (temp > 9) ? ('a' + temp - 10) : (temp + '0');    
    }  
    if(i % 2)  
    {  
        pChar[NumLen-1] = 'F';  
        return (NumLen + 1);  
    }  
      
    return NumLen;  
}

//发送一条短信  
//短信电话号码长度按照字节算但是短信中心号码按照半字节算  
//0891683108707505F011000D91683110064530F50008AA0C9E3F9E4475355B5079D16280  // 鸿鹄电子科技
/************************************************************************************************************************* 
*函数         :   bool GSM_SendOneSMS(char *pSMS, u8 *pPDU, char *pServeNumber, char *pPhoneNumber) 
*功能         :   发送一条普通短信,正常长度 
*参数         :   pSMS:短信内容缓冲区指针,内容为文本文档,并且字符串需要结束符 
*                   pPDU:PDU数据缓冲区指针 
                    pServeNumber:短信中心号码 
                    pPhoneNumber:目标手机号码结构指针 
*返回         :   TRUE:短信发送成功;FALSE:短信发送失败 
*依赖         :   底层 
*作者         :   cp1300@139.com 
*时间         :   2013-04-04 
*最后修改时间 :   201310-23 
*说明         :   短信文本需要添加结束符 
                    电话号码必须以86等国际区号开头 
                    PDU可以发送中文,但是text只能发送英文 
*************************************************************************************************************************/
bool GSM_SendOneSMS(char *pSMS, u8 *pPDU, char *pPhoneNumber)  // 服务中心号码一定要加86
{  
    //SIM900_ERROR error;  
    u16 OffsetCnt = 0;      //缓冲区偏移计数器  
    u32 cnt;  
    u16 temp;  
    char ComBuff[16];  
    u16  nSMSCenterLen= 0, nSMSPduLen = 0;  
    u16 SMSLen = 0;         //短信字符长度  
    u8 EnableU2S = DISABLE;    //使能U2S编码模式,默认为7BIT编码模式  
    u8 *p = (u8 *)pSMS;  
      
    while(*p != 0)  
    {  
        if(*p >= 0x80)       //有汉字  
        {  
            EnableU2S = ENABLE; //使能U2S编码模式  
            //SIM900_debug("需要发送的短信为PDU格式\r\n");  
            break;  
        }  
        p++;  
    }  
      
                  
    if(EnableU2S == ENABLE)         //使能了U2S编码模式  
    {  
        SMSLen = GSM_PDUStrRes(pSMS);   //限制PDU短信长度,计算短信长度  
    }  
    else                            //TEXT模式短信  
    {  
        SMSLen = strlen(pSMS);      //计算短信长度  
        if(SMSLen > 160)     //短信长度大于160个字符  
        {  
            pSMS[160] = 0;          //添加结束符,限制长度  
            SMSLen = 160;  
        }  
    }  
#if 0      //sim7100 发送短信不需要添加短信中心号码
    //计算短信中心号码长度,+91,+86,短信中心号码必须由86开头,并且要加上91(+),长度为每个数字半字节,不足补F  
    temp = (strlen(pServeNumber) + 2 + 1) /2;    
    GSM_HexToString(temp, (char *)(pPDU+OffsetCnt), 2); //短信中心号码长度转换为16进制样式字符串  
    OffsetCnt += 2;                                     //跳过短信中心号码长度字节  
      
    pPDU[OffsetCnt++] = '9';    //服务中心类型  
    pPDU[OffsetCnt++] = '1';  
    OffsetCnt += PhoneNumtoPDUChar((u8 *)pServeNumber,(char *)(pPDU+OffsetCnt),strlen(pServeNumber));   //短信中心号码  
    nSMSCenterLen = OffsetCnt / 2;  
#endif
	// SCA 基本固定为0
	pPDU[OffsetCnt++] = '0';  
    pPDU[OffsetCnt++] = '0';
    //! PDU  
    pPDU[OffsetCnt++] = '1';  
    pPDU[OffsetCnt++] = '1';  
    //! For MR  
    pPDU[OffsetCnt++] = '0';  
    pPDU[OffsetCnt++] = '0';  
    //! For DA  
      
    //计算电话号码长度,+86,发送短信的电话号码由86开头,电话号码长度为字符个数  
    GSM_HexToString(strlen(pPhoneNumber), (char *)(pPDU+OffsetCnt), 2); //手机号码长度转换为16进制样式字符串  
    OffsetCnt += 2; //跳过手机号码长度字节  
      
    pPDU[OffsetCnt++] = '9';    //服务中心类型  
    pPDU[OffsetCnt++] = '1';  
    OffsetCnt += PhoneNumtoPDUChar((u8 *)pPhoneNumber,(char *)(pPDU+OffsetCnt),strlen(pPhoneNumber));   //短信发送号码  
    //! For PID  
    pPDU[OffsetCnt++] = '0';  
    pPDU[OffsetCnt++] = '0';  
    //! For DCS  
    if(EnableU2S == ENABLE)         //U2S  
    {  
        pPDU[OffsetCnt++] = '0';  
        pPDU[OffsetCnt++] = '8';  
    }  
    else                            //7BIT  
    {  
        pPDU[OffsetCnt++] = '0';  
        pPDU[OffsetCnt++] = '0';  
    }  
      
    //! For VP  
    pPDU[OffsetCnt++] = 'A';  
    pPDU[OffsetCnt++] = 'A';  
      
    //! For UDL AND UD  
      //! 注意,此处先将用户数据长度设置为00,并  
      //! 记录此时的缓冲区位置,然后等编码完成,   
      //! 确定了用户数据长度后再修改为实际长度  
      cnt = OffsetCnt;  
      pPDU[OffsetCnt++] = '0';  
      pPDU[OffsetCnt++] = '0';    
      
    //短信内容  
    if(EnableU2S == ENABLE)         //U2S  
    {  
        temp = StringToUnicodeStr(pSMS,(char *)&pPDU[OffsetCnt], strlen(pSMS));//将短信数据转换为字符数据  
        OffsetCnt += temp;  
        GSM_HexToString(temp/2, (char *)&pPDU[cnt], 2);  // 填充PDU 短信长度
        //! PDU串的长度,后面AT+CMGS要用到此长度  
        nSMSPduLen = OffsetCnt / 2 -nSMSCenterLen;  
    }  
    else                            //7bit  
    {  
        u8 buff[140];               //TEXT短信缓冲区  
          
        temp = gsmEncode7bit(pSMS, buff);                       //将ASCII转换为7bit编码  
        GSM_HexToString(SMSLen, (char *)&pPDU[cnt], 2);  
          
        for(cnt = 0;cnt < temp;cnt ++)  
        {  
            GSM_HexToString(buff[cnt], (char *)&pPDU[OffsetCnt+cnt*2], 2);  //7bit编码转换为16进制格式字符串  
        }  
        OffsetCnt += (temp << 1);  
        //! PDU串的长度,后面AT+CMGS要用到此长度  
        nSMSPduLen = OffsetCnt / 2 -nSMSCenterLen;  
    }  
      
    //短信内容长度转换为16进制样式字符串,存储短信长度  
      
    //end  
    pPDU[OffsetCnt++] = 0x1A;  // CTRL+Z
    pPDU[OffsetCnt++] = 0x00;  // 结束符
   
    return TRUE;  
}
/************************************************************************************************************************* 
*函数         :   bool GSM_SendSMS(char *pSMS, char *pPhoneNumber) 
*功能         :   发送一条短信 
*参数         :   pSMS:短信内容缓冲区指针,内容为文本文档,并且字符串需要结束符 
                    pPhoneNumber:目标电话号码 
*返回         :   TRUE:短信发送成功;FALSE:短信发送失败 
*依赖         :   底层 
*作者         :   cp1300@139.com 
*时间         :   2013-10-25 
*最后修改时间 :   2013-10-25 
*说明         :   需要先调用SIM900_SetSMSServeNumber()设置短信中心号码 
                    需要使用全局的PDU数据缓冲区 
                    一定要添加结束符 
                    当短信长度超过单条短信长度限制后会发送多条短信 
*************************************************************************************************************************/  
#define SMS_MAX_LEN     2048                    //短信最大长度  
bool GSM_SendSMS(char *pSMS, char *pPhoneNumber)  
{  
    char SMSBuff[160+1];    //短信最大160Byte,加上一个结束符  
    u8 PDUBuff[512];        //短信PDU数据缓冲区  
    u16 SMSLen;             //短信长度  
    u16 SMSOffset;          //短信发送偏移位置,用于发送多条短信  
    u16 i,j;  
      
    SMSLen = strlen(pSMS);  		//获取要发送的短信长度  
    if(SMSLen > SMS_MAX_LEN) 
		SMSLen = SMS_MAX_LEN;   //限制短信最大长度,防止无限发送  
		
#if 0      
    if(strlen(SMSServeNumber.PhoneNumBuff) == 0)  
    {  
        //SIM900_debug("由于短信中心号码设置失败,导致短信无法发送!\r\n");  
        return FALSE;  
    }  
#endif

    SMSOffset = 0;          //起始偏移为0  
    while(1)  
    {  
        if((SMSLen-SMSOffset) > 160)  
            j = 160;  
        else   
            j = SMSLen-SMSOffset;  
        for(i = 0;i < j;i ++)  
        {  
            SMSBuff[i] = pSMS[SMSOffset + i];   //复制短信到发送缓冲区  
        }  
        SMSBuff[j] = 0; //添加结束符  
          
        if(GSM_CheckNotASCII(SMSBuff,j) == TRUE)    //分割的短信中含有非ASCII编码,那么只能使用U2S编码,只能发送70个字符(包括中英文)  
        {  
            SMSOffset += GSM_GetU2SCharOffset(SMSBuff,70);  //第一条短信限制70个字符,返回下一条分割的起始位置  
            SMSBuff[SMSOffset] = 0;  
        }  
        else  
        {  
            SMSOffset += j; //下一条分割的起始位置  
            SMSBuff[SMSOffset] = 0;  
        }  
          
        //SIM900_WaitSleep(1000);   //等待上一个操作完成  
        if(GSM_SendOneSMS(SMSBuff, PDUBuff, pPhoneNumber) == TRUE)  
        {  
			send_one_message(PDUBuff);
        }  
        else  
        {  
            //SIM900_debug("发送短信失败!\r\n");  
            return FALSE;  
        }  
        if(SMSOffset >= SMSLen) 
			break;   //短信发送完成,退出  
    }  
      
    return TRUE;  
      
}


/************************************************************************************************************************* 
* 函数    :   u8 GBK_UNICODE_Init(void) 
* 功能    :   初始化GBK,UNICODE编码表 
* 参数    :   无    
* 返回    :   0:初始化成功;其它:初始化失败 
* 依赖    :   底层读写函数 
* 作者    :   cp1300@139.com 
* 时间    :   2013-04-18 
* 最后修改时间 : 2013-04-18 
* 说明    :   无 
*************************************************************************************************************************/   
u8 GBK_UNICODE_Init(void)  
{    
    GtoU_File = fopen(GtoU, "r");
	if (GtoU_File == NULL)
	{
		printf("open GtoU failed\n");
		return 0;
	}
	UtoG_File = fopen(UtoG, "r");
	if (UtoG_File == NULL)
	{
		printf("open UtoG failed\n");
  		return 0;
	}
    return 1;  
} 
