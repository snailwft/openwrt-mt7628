//File   : lrecord.c
//Author : Loon <sepnic@gmail.com>

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include "wav_parser.h"
#include "sndwav_common.h"

#define DEFAULT_CHANNELS         (2)
#define DEFAULT_SAMPLE_RATE      (8000)
#define DEFAULT_SAMPLE_LENGTH    (16)
#define DEFAULT_DURATION_TIME    (10)

int SNDWAV_PrepareWAVParams(WAVContainer_t *wav)
{
	assert(wav);

	uint16_t channels = DEFAULT_CHANNELS;
	uint16_t sample_rate = DEFAULT_SAMPLE_RATE;
	uint16_t sample_length = DEFAULT_SAMPLE_LENGTH;
	uint32_t duration_time = DEFAULT_DURATION_TIME;

	/* Const */
	wav->header.magic = WAV_RIFF;
	wav->header.type = WAV_WAVE;
	wav->format.magic = WAV_FMT;
	wav->format.fmt_size = LE_INT(16);
	wav->format.format = LE_SHORT(WAV_FMT_PCM);
	wav->chunk.type = WAV_DATA;

	/* User definition */
	wav->format.channels = LE_SHORT(channels);
	wav->format.sample_rate = LE_INT(sample_rate);
	wav->format.sample_length = LE_SHORT(sample_length);

	/* See format of wav file */
	wav->format.blocks_align = LE_SHORT(channels * sample_length / 8);
	wav->format.bytes_p_second = LE_INT((uint16_t)(wav->format.blocks_align) * sample_rate);
	
	wav->chunk.length = LE_INT(duration_time * (uint32_t)(wav->format.bytes_p_second));
	wav->header.length = LE_INT((uint32_t)(wav->chunk.length) +/
		sizeof(wav->chunk) + sizeof(wav->format) + sizeof(wav->header) - 8);

	return 0;
}

void SNDWAV_Record(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav, int fd)
{
	off64_t rest;
	size_t c, frame_size;
	
	if (WAV_WriteHeader(fd, wav) < 0) {
		exit(-1);
	}

	rest = wav->chunk.length;
	while (rest > 0) {
		c = (rest <= (off64_t)sndpcm->chunk_bytes) ? (size_t)rest : sndpcm->chunk_bytes;
		frame_size = c * 8 / sndpcm->bits_per_frame;
		if (SNDWAV_ReadPcm(sndpcm, frame_size) != frame_size)
			break;
		
		if (write(fd, sndpcm->data_buf, c) != c) {
			fprintf(stderr, "Error SNDWAV_Record[write]/n");
			exit(-1);
		}

		rest -= c;
	}
}

int main(int argc, char *argv[])
{
	char *filename;
	char *devicename = "default";
	int fd;
	WAVContainer_t wav;
	SNDPCMContainer_t record;
	
	if (argc != 2) {
		fprintf(stderr, "Usage: ./lrecord <FILENAME>/n");
		return -1;
	}
	
	memset(&record, 0x0, sizeof(record));

	filename = argv[1];
	remove(filename);
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0644)) == -1) {
		fprintf(stderr, "Error open: [%s]/n", filename);
		return -1;
	}

	if (snd_output_stdio_attach(&record.log, stderr, 0) < 0) {
		fprintf(stderr, "Error snd_output_stdio_attach/n");
		goto Err;
	}

	if (snd_pcm_open(&record.handle, devicename, SND_PCM_STREAM_CAPTURE, 0) < 0) {
		fprintf(stderr, "Error snd_pcm_open [ %s]/n", devicename);
		goto Err;
	}

	if (SNDWAV_PrepareWAVParams(&wav) < 0) {
		fprintf(stderr, "Error SNDWAV_PrepareWAVParams/n");
		goto Err;
	}

	if (SNDWAV_SetParams(&record, &wav) < 0) {
		fprintf(stderr, "Error set_snd_pcm_params/n");
		goto Err;
	}
	snd_pcm_dump(record.handle, record.log);

	SNDWAV_Record(&record, &wav, fd);

	snd_pcm_drain(record.handle);

	close(fd);
	free(record.data_buf);
	snd_output_close(record.log);
	snd_pcm_close(record.handle);
	return 0;

Err:
	close(fd);
	remove(filename);
	if (record.data_buf) free(record.data_buf);
	if (record.log) snd_output_close(record.log);
	if (record.handle) snd_pcm_close(record.handle);
	return -1;
}