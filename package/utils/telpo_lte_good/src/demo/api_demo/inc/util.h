#ifndef UTIL_H
#define UTIL_H

#ifdef __GNUC__
int kbit(void);
#endif

int selectUserInt(const char *msg);

int selectUserStr(const char *msg,char *user_input);

#endif
