#ifndef CryptFunH
#define CryptFunH

extern int crypt_des(unsigned char *Key, unsigned char *InBuff,
                    int InLen, unsigned char *OutBuff, char IsEncrypt);

#endif
