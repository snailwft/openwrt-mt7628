#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define WATCHDOG_ENABLE 1
#define WATCHDOG_LOW 2
#define WATCHDOG_HIGET 3

int main()
{
	pid_t pid;
	int fd;
	fd = open("/dev/watchdog_dev",O_RDWR);
	if (fd < 0)
	{
		printf("open watchdog_dev failed\n");
		system("reboot");
		exit(1);
	}
	ioctl(fd, WATCHDOG_ENABLE);// 使能看门狗
	pid = fork();
	if (pid < 0)
	{
		printf("fork failed\n");
		exit(1);
	}
	else if (pid == 0)
	{
		if (execl("/etc/run_start.sh","run_start.sh",(char *)0) < 0)
		{
			printf ("execl failed\n");
			exit(1);
		}
	}
	while (waitpid(pid, NULL, WNOHANG) == 0)
	{
		//printf ("waitpid %d\n", pid);
		//sleep(10);
		ioctl(fd, WATCHDOG_HIGET);
		usleep(100000);
		ioctl(fd, WATCHDOG_LOW);
		usleep(100000);
	}
	exit(0);
}

