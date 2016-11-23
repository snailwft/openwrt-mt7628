#include "dbug_log.h"
#include "gsm.h"

int dbug_close_logfile(FILE *dbug_logfile)
{
	if ((dbug_logfile == stderr) || (dbug_logfile == stdout))
		return(0);
	return(fclose(dbug_logfile));
}

long dbug_get_size_of_logfile()
{
	return(50000);
}


int dbug_get_name_of_logfile(char *name_of_logfile)
{
	sprintf(name_of_logfile,DBUG_LOG,NULL);
	return(0);
}

FILE *dbug_open_logfile()
{
	char filename[512];
	FILE *dbug_logfile;
	
	memset(filename,0,512);
	if (dbug_get_name_of_logfile(filename) < 0 )
		return(stderr);
	//printf("filename = [%s]\n",filename);
	
	if ((dbug_logfile = fopen(filename,"a")) == NULL)
		return(stderr);

	fseek(dbug_logfile, 0, SEEK_END);
	if (ftell(dbug_logfile) >= dbug_get_size_of_logfile())
	{
		dbug_close_logfile(dbug_logfile);
		if ((dbug_logfile = fopen(filename,"w")) == NULL)
			return(stderr);
	}

	return(dbug_logfile);
}

unsigned long dbug_get_logtime( char *cur_time )
{
    time_t  tb;
    struct tm *tmptm= NULL;
    int  tmpyear;
	
    time( &tb );

    tmptm = (struct tm *)localtime(&tb);
    /*
      if (tmptm->tm_year > 90)
      tmpyear = 1900 + tmptm->tm_year%1900;
      else
      tmpyear = 2000 + tmptm->tm_year;
    */
	

    sprintf(cur_time,"%02d%02d%02d%02d%02d",tmptm->tm_mon+1,tmptm->tm_mday,tmptm->tm_hour,tmptm->tm_min,tmptm->tm_sec);

    return tb; 
}


void dbug_log(char *file, int line, char *fmt,...)
{	
	char		tmp_buf[40];
	FILE		*log_file_ptr;	
	
	log_file_ptr = dbug_open_logfile();
	memset(tmp_buf,0,40);
	dbug_get_logtime(tmp_buf);
	fprintf(log_file_ptr,"[%s__%d][%10s][Debug][%05d]:\t",file,line,tmp_buf,getpid());
	va_list 	args;
	va_start(args,fmt);
	vfprintf(log_file_ptr,fmt,args);
	va_end(args);
	fflush(log_file_ptr);
	dbug_close_logfile(log_file_ptr);
}

