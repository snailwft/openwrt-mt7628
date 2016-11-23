/* tinyplay.c
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

#include "asoundlib.h"
#include "gsm.h"
#include "user_handler.h"
#include "dtmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

int gsm_pcm_recv_success = 0;

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

static int close1 = 0;

void play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count);

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close1 = 1;
}



//int main(int argc, char **argv)
int tiny_play()
{
    FILE *file;
#if 0
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
	int more_chunks = 1;
#endif
    unsigned int device = 0;
    unsigned int card = 0;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
   
#if 0
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav [-D card] [-d device] [-p period_size]"
                " [-n n_periods] \n", argv[0]);
        return 1;
    }
#endif

    file = fopen(TINY_PLAY_FILE, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", TINY_PLAY_FILE);
        return 1;
    }
#if 0
    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
        fclose(file);
        return 1;
    }

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);
#endif
#if 0
    /* parse command line arguments */
    argv += 2;
    while (*argv) 
	{
        if (strcmp(*argv, "-d") == 0) 
		{
            argv++;
            if (*argv)
                device = atoi(*argv);
        }
        if (strcmp(*argv, "-p") == 0) 
		{
            argv++;
            if (*argv)
                period_size = atoi(*argv);
        }
        if (strcmp(*argv, "-n") == 0) 
		{
            argv++;
            if (*argv)
                period_count = atoi(*argv);
        }
        if (strcmp(*argv, "-D") == 0) 
		{
            argv++;
            if (*argv)
                card = atoi(*argv);
        }
        if (*argv)
            argv++;
    }
#endif
	#if 0
    play_sample(file, card, device, chunk_fmt.num_channels, chunk_fmt.sample_rate,
                chunk_fmt.bits_per_sample, period_size, period_count);
	#else
	 //play_sample(file, card, device, chunk_fmt.num_channels, SAMPLE_RATE,
        //        chunk_fmt.bits_per_sample, period_size, period_count);
	 play_sample(file, card, device,2, SAMPLE_RATE,
                16, period_size, period_count);
	#endif
    fclose(file);

    return 0;
}

int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
                 char *param_name, char *param_unit)
{
    unsigned int min;
    unsigned int max;
    int is_within_bounds = 1;

    min = pcm_params_get_min(params, param);
    if (value < min) {
        fprintf(stderr, "%s is %u%s, device only supports >= %u%s\n", param_name, value,
                param_unit, min, param_unit);
        is_within_bounds = 0;
    }

    max = pcm_params_get_max(params, param);
    if (value > max) {
        fprintf(stderr, "%s is %u%s, device only supports <= %u%s\n", param_name, value,
                param_unit, max, param_unit);
        is_within_bounds = 0;
    }

    return is_within_bounds;
}

int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
                        unsigned int rate, unsigned int bits, unsigned int period_size,
                        unsigned int period_count)
{
    struct pcm_params *params;
    int can_play;

    params = pcm_params_get(card, device, PCM_OUT);
    if (params == NULL) {
        fprintf(stderr, "Unable to open PCM device %u.\n", device);
        return 0;
    }

    can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", "Hz");
    can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", "Hz");

    pcm_params_free(params);

    return can_play;
}

struct pcm *get_pcm_out()
{
#if 1
	unsigned int card = 0;
	unsigned int device = 0;
	struct pcm_config config;
	struct pcm *pcm;
	char *buffer;
	int size;
	int num_read;
	int bits = 16;
	config.channels = 1;
	config.rate = SAMPLE_RATE;
	config.period_size = 160;
	config.period_count = 32;
	if (bits == 32)
	{
		config.format = PCM_FORMAT_S32_LE;
	}
	else if (bits == 16)
	{
		config.format = PCM_FORMAT_S16_LE;
	}
	config.start_threshold = 0;
	config.stop_threshold = 0;
	config.silence_threshold = 0;

	if (!sample_is_playable(card, device, config.channels, config.rate, bits, config.period_size, config.period_count)) 
	{
		return;
	}

	pcm = pcm_open(card, device, PCM_OUT, &config);
	if (!pcm || !pcm_is_ready(pcm)) 
	{
		fprintf(stderr, "Unable to open PCM device %u (%s)\n",
		device, pcm_get_error(pcm));
		return;
	}
	return pcm;

#else
	
	unsigned int card = 0;
	unsigned int device = 0;
	struct pcm_config config;
	struct pcm *pcm_out;
	pcm_init_config(&config);
	
	if (!sample_is_playable(card, device, config.channels, config.rate, 16, config.period_size, config.period_count)) 
	{
        return;
    }
	pcm_out = pcm_open(card, device, PCM_OUT, &config);
	if (!pcm_out || !pcm_is_ready(pcm_out)) 
	{
		printf("Unable to open PCM device %u (%s)\n",
		device, pcm_get_error(pcm_out));
		return NULL;
	}
	return pcm_out;
#endif

}

void play_sample(FILE *file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count)
{
#if 0
    struct pcm_config config;
    struct pcm *pcm;
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    if (!sample_is_playable(card, device, channels, rate, bits, period_size, period_count)) {
        return;
    }

    pcm = pcm_open(card, device, PCM_OUT, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm));
        return;
    }
#endif
	int size, size2;
	char *buffer, *buffer2;
	int num_read, num_read2;
    size = pcm_frames_to_bytes(pcm_out, pcm_get_buffer_size(pcm_out));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm_out);
        return;
    }

    printf("Playing sample: %u ch, %u hz, %u bit\n", channels, rate, bits);

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

#if 1
	size2 = pcm_frames_to_bytes(pcm_in, pcm_get_buffer_size(pcm_in));
	buffer2 = malloc(size2);
//#ifdef _DBUG_
	printf("buffer = %d, buffer2 = %d\n", size, size2);
//#endif
	if (!buffer2) 
	{
		fprintf(stderr, "Unable to allocate %d bytes\n", size2);
		free(buffer2);
		pcm_close(pcm_in);
		return 0;
	}
	while (!close1 && !pcm_read(pcm_in, buffer2, size2)) 
	{
		#if 0
		if (fwrite(buffer2, 1, size2, file) != size) 
		{
			fprintf(stderr,"Error capturing sample\n");
			break;
		}
		bytes_read += size;
		#endif
		if (pcm_write(pcm_out, buffer2, size2)) 
		{
			fprintf(stderr, "Error playing sample\n");
			break;
		}
	}
#endif
#if 0
    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if (pcm_write(pcm_out, buffer, num_read)) 
			{
                fprintf(stderr, "Error playing sample\n");
                break;
            }
        }
    } while (!close && num_read > 0);

    free(buffer);
    pcm_close(pcm_out);
#endif
}

void *gsm_audio_write_thread(void *arg)
{
	int len = 0, i = 0, read_times = 0, write_flag = 0;
	char *buffer = NULL;
	buffer = malloc(BUFFER_SIZE);
	if (!buffer) 
	{
		dbug_log(__FILE__,__LINE__,"Unable to allocate %d bytes\n", BUFFER_SIZE);
		free(buffer);
		exit(1);
	}
#ifdef _TINYPLAY_
	FILE *file;
	file = fopen(TINY_PLAY_FILE, "w");
	if (!file) 
	{
		fprintf(stderr, "Unable to create file '%s'\n", TINY_CAP_FILE);
	}
	else
	{
		printf("TINY_PLAY_FILE open success\n");
	}
#endif
	while (1)
	{
		memset(buffer, 0x0, BUFFER_SIZE);
		//sem_wait(&gsm_pcm_sem2);
		if (pcm_flag == 1) // sim pcm通道打开
		{
			len = read(fd2, buffer, BUFFER_SIZE);				
			if (len > 0)
			{
				gsm_pcm_recv_success = 1;
#ifndef _TINYPLAY_
				pcm_write(pcm_out, buffer, len); //(fwrite(buffer, 1, len, file) < 0) // 不为0就是错误
				//soft_dtmf_decode(af, &dsp, &buffer);
#else
				fwrite(buffer, 1, len, file);
#ifndef DTMF_DECODE
				soft_dtmf_decode(af, &dsp, &buffer);
#endif

#endif
			}								
		}
		if (open_flag == 0) //表示挂机完全
		{
			open_flag = 1;		
			gsm_pcm_recv_success = 0;
			pcm_stop(pcm_out);
			pcm_stop(pcm_in);
			tcflush(fd2,TCIFLUSH); 
			tcflush(fd2,TCOFLUSH);
		}	
		//sem_post(&gsm_pcm_sem);		
	}
	dbug_log(__FILE__,__LINE__,"gsm_audio_read_thread exit\n");
	free(buffer);
	pthread_exit(NULL);
}

int soft_dtmf_decode(struct ast_frame *af, struct ast_dsp **dsp, char **buffer)
{
	unsigned char ch;

	af  = (struct ast_frame *)malloc(sizeof(struct ast_frame));
	if (NULL == af)
	{
		printf("dtmf dsp malloc failed\n");  
		exit(1);
	}
	af->frametype = AST_FRAME_VOICE;
	af->data.ptr = *buffer;
	af->datalen = BUFFER_SIZE;
	af = dsp_process(*dsp, af);                 		
	if (af->frametype == AST_FRAME_DTMF && af) 
	{
		printf("DTMF Detected :%c\n", (*dsp)->f.subclass.integer);
		ch = int_to_char((*dsp)->f.subclass.integer);
		pthread_mutex_lock(&dtmf_mutex);
		recv_dtmf(ch);
		pthread_mutex_unlock(&dtmf_mutex);			
		return 1;
	}	
	return 0;
}

void *gsm_audio_read_thread(void *arg)
{
	int size, fd;
	char *buffer;
	int num_write = 0, num_read = 0;
    buffer = malloc(BUFFER_SIZE);
#ifdef DTMF_DECODE
	unsigned char ch;
	struct ast_dsp *dsp = NULL;
	struct ast_frame *af;
	
	dsp = (struct ast_dsp *)malloc(sizeof(struct ast_dsp));	
	if (NULL == dsp )
	{
		printf("dtmf __dsp malloc failed\n");  
		exit(1);
	}
	dtmf_init(dsp);	
#endif
    if (!buffer) 
	{
		dbug_log(__FILE__,__LINE__,"Unable to allocate %d bytes\n", BUFFER_SIZE);
		free(buffer);
		exit(1);
    }
#ifdef _TINYPLAY_
	FILE *file;
	file = fopen(TINY_CAP_FILE, "w");
	if (!file) 
	{
		fprintf(stderr, "Unable to create file '%s'\n", TINY_CAP_FILE);
	}
	else
	{
		printf("tinycap open success\n");
	}
#endif

	while (1)
	{
		memset(buffer, 0x0, BUFFER_SIZE);
		//sem_wait(&gsm_pcm_sem);
#ifdef DTMF_DECODE
		if (slic_pcm_read_open && !pcm_read(pcm_in, buffer, BUFFER_SIZE))
#endif
		{
			if (pcm_flag == 1 && answer_begin == 1 && gsm_pcm_recv_success == 1)
			{	
#ifndef DTMF_DECODE
				if (!pcm_read(pcm_in, buffer, BUFFER_SIZE))
#endif
				{		
#ifdef DTMF_DECODE			
					soft_dtmf_decode(af, &dsp, &buffer);
#endif
					num_write = write(fd2, buffer, BUFFER_SIZE);
					//fwrite(buffer, 1, BUFFER_SIZE, file);//保存pcm数据到文件				
				}
			}	
			else
			{
#ifdef DTMF_DECODE
				soft_dtmf_decode(af, &dsp, &buffer);
#endif
			}
		}
		//sem_post(&gsm_pcm_sem2);
	}
#ifdef _DBUG_
	printf("gsm_audio_read_thread exit\n");
	dbug_log(__FILE__,__LINE__,"gsm_audio_read_thread exit\n");
#endif
	free(buffer);
	pthread_exit(NULL);	
}
