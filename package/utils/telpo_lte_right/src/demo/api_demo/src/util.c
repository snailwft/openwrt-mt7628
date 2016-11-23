/*
** Copyright (c) 2013-2015 by Silicon Laboratories, Inc.
**
** $Id: util.c 4722 2015-02-09 22:38:05Z nizajerk $
**
** This file contains proprietary information.
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
**
** Non-API utilities
**
*/

#ifdef __GNUC__
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
#endif

/*************************************/
int selectUserInt(const char *msg)
{
    char val[8] = {0};
    printf("\n\n");
    printf(msg);
    gets(val);
    return atoi(val);
}

/*************************************/
int selectUserStr(const char *msg, char *user_input)
{
    printf("\n\n");
    printf(msg);
    gets(user_input);
    return strlen(user_input);
}
