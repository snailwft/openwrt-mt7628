#include <stdlib.h>
#include "dtmf.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>

static inline void goertzel_init(goertzel_state_t *s, float freq, int samples)
{
	s->v2 = s->v3 = s->chunky = 0.0;
	s->fac = (int)(32768.0 * 2.0 * cos(2.0 * M_PI * freq / SAMPLE_RATE));
	s->samples = samples;
}

static void dtmf_detect_init (dtmf_detect_state_t *s)
{
	int i;

	s->lasthit = 0;
	s->current_hit = 0;
	for (i = 0;  i < 4;  i++) {
		goertzel_init(&s->row_out[i], dtmf_row[i], DTMF_GSIZE);
		goertzel_init(&s->col_out[i], dtmf_col[i], DTMF_GSIZE);
		s->energy = 0.0;
	}
	s->current_sample = 0;
	s->hits = 0;
	s->misses = 0;
}

static void digit_detect_init(digit_detect_state_t *s)
{
	s->current_digits = 0;
	s->detected_digits = 0;
	s->lost_digits = 0;
	s->digits[0] = '\0';	
	dtmf_detect_init(&s->dtmf);
}

void dtmf_init(struct ast_dsp * dsp)
{
	relax_dtmf_normal_twist = DEF_RELAX_DTMF_NORMAL_TWIST;
	relax_dtmf_reverse_twist = DEF_RELAX_DTMF_REVERSE_TWIST;
	dtmf_normal_twist = DEF_DTMF_NORMAL_TWIST;
	dtmf_reverse_twist = DEF_DTMF_REVERSE_TWIST;
	dsp->features = DSP_FEATURE_DIGIT_DETECT;
	dsp->digitmode = DSP_DIGITMODE_DTMF;
	dtmf_hits_to_begin = DEF_DTMF_HITS_TO_BEGIN;
	dtmf_misses_to_end = DEF_DTMF_MISSES_TO_END;
	//dsp->dtmf_began = 0;
	digit_detect_init(&dsp->digit_state);
}

static void store_digit(digit_detect_state_t *s, char digit)
{
	s->detected_digits++;
	if (s->current_digits < MAX_DTMF_DIGITS) {
		s->digitlen[s->current_digits] = 0;
		s->digits[s->current_digits++] = digit;
		s->digits[s->current_digits] = '\0';
	} else {
		printf("Digit lost due to full buffer\n");
		s->lost_digits++;
	}
}


static inline void goertzel_sample(goertzel_state_t *s, short sample)
{
	int v1;
	
	v1 = s->v2;
	s->v2 = s->v3;
	
	s->v3 = (s->fac * s->v2) >> 15;
	s->v3 = s->v3 - v1 + (sample >> s->chunky);
	if (abs(s->v3) > 32768) {
		s->chunky++;
		s->v3 = s->v3 >> 1;
		s->v2 = s->v2 >> 1;
	}
}
static inline float goertzel_result(goertzel_state_t *s)
{
	goertzel_result_t r;
	r.value = (s->v3 * s->v3) + (s->v2 * s->v2);
	r.value -= ((s->v2 * s->v3) >> 15) * s->fac;
	r.power = s->chunky * 2;
	return (float)r.value * (float)(1 << r.power);
}

static inline void goertzel_reset(goertzel_state_t *s)
{
	s->v2 = s->v3 = s->chunky = 0.0;
}

static void mute_fragment(struct ast_dsp *dsp, fragment_t *fragment)
{
	if (dsp->mute_fragments >= ARRAY_LEN(dsp->mute_data)) {
		printf("Too many fragments to mute. Ignoring\n");
		return;
	}

	dsp->mute_data[dsp->mute_fragments++] = *fragment;
}

static int dtmf_detect(struct ast_dsp *dsp, digit_detect_state_t *s, int16_t amp[], int samples, int squelch, int relax)
{
	float row_energy[4];
	float col_energy[4];
	int i;
	int j;
	int sample;
	short samp;
	int best_row;
	int best_col;
	int hit;
	int limit;
	static int nn = 0;
	fragment_t mute = {0, 0};
		
	if (squelch && s->dtmf.mute_samples > 0) {
		mute.end = (s->dtmf.mute_samples < samples) ? s->dtmf.mute_samples : samples;
		s->dtmf.mute_samples -= mute.end;
	}

	hit = 0;
	for (sample = 0; sample < samples; sample = limit) {
		/* DTMF_GSIZE is optimised to meet the DTMF specs. */
		if ((samples - sample) >= (DTMF_GSIZE - s->dtmf.current_sample)) {
			limit = sample + (DTMF_GSIZE - s->dtmf.current_sample);
		} else {
			limit = samples;
		}
		/* The following unrolled loop takes only 35% (rough estimate) of the 
		   time of a rolled loop on the machine on which it was developed */
		for (j = sample; j < limit; j++) {
			samp = amp[j];			
			s->dtmf.energy += (int) samp * (int) samp;
			/* With GCC 2.95, the following unrolled code seems to take about 35%
			   (rough estimate) as long as a neat little 0-3 loop */
			goertzel_sample(s->dtmf.row_out, samp);
			goertzel_sample(s->dtmf.col_out, samp);
			goertzel_sample(s->dtmf.row_out + 1, samp);
			goertzel_sample(s->dtmf.col_out + 1, samp);
			goertzel_sample(s->dtmf.row_out + 2, samp);
			goertzel_sample(s->dtmf.col_out + 2, samp);
			goertzel_sample(s->dtmf.row_out + 3, samp);
			goertzel_sample(s->dtmf.col_out + 3, samp);		
		}
		
		s->dtmf.current_sample += (limit - sample);
		if (s->dtmf.current_sample < DTMF_GSIZE) {
			continue;
		}
		/* We are at the end of a DTMF detection block */
		/* Find the peak row and the peak column */
		row_energy[0] = goertzel_result (&s->dtmf.row_out[0]);
		col_energy[0] = goertzel_result (&s->dtmf.col_out[0]);

		for (best_row = best_col = 0, i = 1;  i < 4;  i++) {
			row_energy[i] = goertzel_result (&s->dtmf.row_out[i]);
			if (row_energy[i] > row_energy[best_row]) {
				best_row = i;
			}
			col_energy[i] = goertzel_result (&s->dtmf.col_out[i]);
			if (col_energy[i] > col_energy[best_col]) {
				best_col = i;
			}
		}
		hit = 0;
	
		/* Basic signal level test and the twist test */
		if (row_energy[best_row] >= DTMF_THRESHOLD && 
		    col_energy[best_col] >= DTMF_THRESHOLD &&
		    col_energy[best_col] < row_energy[best_row] * (relax ? relax_dtmf_reverse_twist : dtmf_reverse_twist) &&
		    row_energy[best_row] < col_energy[best_col] * (relax ? relax_dtmf_normal_twist : dtmf_normal_twist)) {
			
			/* Relative peak test */
			for (i = 0;  i < 4;  i++) {
				if ((i != best_col &&
				    col_energy[i] * DTMF_RELATIVE_PEAK_COL > col_energy[best_col]) ||
				    (i != best_row 
				     && row_energy[i] * DTMF_RELATIVE_PEAK_ROW > row_energy[best_row])) {
					break;
				}
			}
			/* ... and fraction of total energy test */
			if (i >= 4 &&
			    (row_energy[best_row] + col_energy[best_col]) > DTMF_TO_TOTAL_ENERGY * s->dtmf.energy) {				
				hit = dtmf_positions[(best_row << 2) + best_col];		
			}
		} 
/*
 * Adapted from ETSI ES 201 235-3 V1.3.1 (2006-03)
 * (40ms reference is tunable with hits_to_begin and misses_to_end)
 * each hit/miss is 12.75ms with DTMF_GSIZE at 102
 *
 * Character recognition: When not DRC *(1) and then
 *      Shall exist VSC > 40 ms (hits_to_begin)
 *      May exist 20 ms <= VSC <= 40 ms
 *      Shall not exist VSC < 20 ms
 *
 * Character recognition: When DRC and then
 *      Shall cease Not VSC > 40 ms (misses_to_end)
 *      May cease 20 ms >= Not VSC >= 40 ms
 *      Shall not cease Not VSC < 20 ms
 *
 * *(1) or optionally a different digit recognition condition
 *
 * Legend: VSC The continuous existence of a valid signal condition.
 *      Not VSC The continuous non-existence of valid signal condition.
 *      DRC The existence of digit recognition condition.
 *      Not DRC The non-existence of digit recognition condition.
 */

/*
 * Example: hits_to_begin=2 misses_to_end=3
 * -------A last_hit=A hits=0&1
 * ------AA hits=2 current_hit=A misses=0       BEGIN A
 * -----AA- misses=1 last_hit=' ' hits=0
 * ----AA-- misses=2
 * ---AA--- misses=3 current_hit=' '            END A
 * --AA---B last_hit=B hits=0&1
 * -AA---BC last_hit=C hits=0&1
 * AA---BCC hits=2 current_hit=C misses=0       BEGIN C
 * A---BCC- misses=1 last_hit=' ' hits=0
 * ---BCC-C misses=0 last_hit=C hits=0&1
 * --BCC-CC misses=0
 *
 * Example: hits_to_begin=3 misses_to_end=2
 * -------A last_hit=A hits=0&1
 * ------AA hits=2
 * -----AAA hits=3 current_hit=A misses=0       BEGIN A
 * ----AAAB misses=1 last_hit=B hits=0&1
 * ---AAABB misses=2 current_hit=' ' hits=2     END A
 * --AAABBB hits=3 current_hit=B misses=0       BEGIN B
 * -AAABBBB misses=0
 *
 * Example: hits_to_begin=2 misses_to_end=2
 * -------A last_hit=A hits=0&1
 * ------AA hits=2 current_hit=A misses=0       BEGIN A
 * -----AAB misses=1 hits=0&1
 * ----AABB misses=2 current_hit=' ' hits=2 current_hit=B misses=0 BEGIN B
 * ---AABBB misses=0
 */

		if (s->dtmf.current_hit) {
			/* We are in the middle of a digit already */
			if (hit != s->dtmf.current_hit) {
				s->dtmf.misses++;
				if (s->dtmf.misses == dtmf_misses_to_end) {
					/* There were enough misses to consider digit ended */
					s->dtmf.current_hit = 0;
				}
			} else {
				s->dtmf.misses = 0;
				/* Current hit was same as last, so increment digit duration (of last digit) */
				s->digitlen[s->current_digits - 1] += DTMF_GSIZE;
			}
		}

		/* Look for a start of a new digit no matter if we are already in the middle of some
		   digit or not. This is because hits_to_begin may be smaller than misses_to_end
		   and we may find begin of new digit before we consider last one ended. */

		if (hit != s->dtmf.lasthit) {
			s->dtmf.lasthit = hit;
			s->dtmf.hits = 0;
		}
		if (hit && hit != s->dtmf.current_hit) {
			s->dtmf.hits++;
			if (s->dtmf.hits == dtmf_hits_to_begin) {
				store_digit(s, hit);
				s->digitlen[s->current_digits - 1] = dtmf_hits_to_begin * DTMF_GSIZE;
				s->dtmf.current_hit = hit;
				s->dtmf.misses = 0;
			}
		}

		/* If we had a hit in this block, include it into mute fragment */
		if (squelch && hit) {
			if (mute.end < sample - DTMF_GSIZE) {
				/* There is a gap between fragments */
				mute_fragment(dsp, &mute);
				mute.start = (sample > DTMF_GSIZE) ? (sample - DTMF_GSIZE) : 0;
			}
			mute.end = limit + DTMF_GSIZE;
		}

		/* Reinitialise the detector for the next block */
		for (i = 0; i < 4; i++) {
			goertzel_reset(&s->dtmf.row_out[i]);
			goertzel_reset(&s->dtmf.col_out[i]);
		}
		s->dtmf.energy = 0.0;
		s->dtmf.current_sample = 0;
	}

	if (squelch && mute.end) {
		if (mute.end > samples) {
			s->dtmf.mute_samples = mute.end - samples;
			mute.end = samples;
		}
		mute_fragment(dsp, &mute);
	}

	return (s->dtmf.current_hit);	/* return the debounced hit */
}


struct ast_frame *dsp_process(struct ast_dsp *dsp, struct ast_frame *af)
{	
	int digit = 0;
	short *shortdata;	
	int len;
	struct ast_frame *outf = NULL;
	int x;

	if (!af) {
		return NULL;
	}
	if (af->frametype != AST_FRAME_VOICE) {
		return af;
	}		
        
	shortdata = af->data.ptr;
	len = af->datalen / 2;
		

	/* Initially we do not want to mute anything */
	dsp->mute_fragments = 0;

	/* Need to run the silence detection stuff for silence suppression and busy detection */
	
	if (dsp->features & (DSP_FEATURE_DIGIT_DETECT | DSP_FEATURE_BUSY_DETECT)) {
		//if (dsp->digitmode & DSP_DIGITMODE_MF)
			;//digit = mf_detect(dsp, &dsp->digit_state, shortdata, len, (dsp->digitmode & DSP_DIGITMODE_NOQUELCH) == 0, (dsp->digitmode & DSP_DIGITMODE_RELAXDTMF));
		//else			
			digit = dtmf_detect(dsp, &dsp->digit_state, shortdata, len, (dsp->digitmode & DSP_DIGITMODE_NOQUELCH) == 0, (dsp->digitmode & DSP_DIGITMODE_RELAXDTMF));

		if (dsp->digit_state.current_digits) {
			int event = 0, event_len = 0;
			char event_digit = 0;

			if (!dsp->dtmf_began) {
				/* We have not reported DTMF_BEGIN for anything yet */

				if (dsp->features & DSP_FEATURE_DIGIT_DETECT) {
					event = AST_FRAME_DTMF_BEGIN;
					event_digit = dsp->digit_state.digits[0];
				}
				dsp->dtmf_began = 1;

			} else if (dsp->digit_state.current_digits > 1 || digit != dsp->digit_state.digits[0]) {
				/* Digit changed. This means digit we have reported with DTMF_BEGIN ended */
				if (dsp->features & DSP_FEATURE_DIGIT_DETECT) {
					event = AST_FRAME_DTMF_END;
					event_digit = dsp->digit_state.digits[0];
					event_len = dsp->digit_state.digitlen[0] * 1000 / SAMPLE_RATE;
				}
				memmove(&dsp->digit_state.digits[0], &dsp->digit_state.digits[1], dsp->digit_state.current_digits);
				memmove(&dsp->digit_state.digitlen[0], &dsp->digit_state.digitlen[1], dsp->digit_state.current_digits * sizeof(dsp->digit_state.digitlen[0]));
				dsp->digit_state.current_digits--;
				dsp->dtmf_began = 0;

				if (dsp->features & DSP_FEATURE_BUSY_DETECT) {
					/* Reset Busy Detector as we have some confirmed activity */ 
					memset(dsp->historicsilence, 0, sizeof(dsp->historicsilence));
					memset(dsp->historicnoise, 0, sizeof(dsp->historicnoise));
					printf("DTMF Detected - Reset busydetector\n");
				}
			}

			if (event) {
				memset(&dsp->f, 0, sizeof(dsp->f));
				dsp->f.frametype = event;
				dsp->f.subclass.integer = event_digit;
				dsp->f.len = event_len;
				outf = &dsp->f;	
				//printf("DTMF Detected -:%c\n", event_digit);			
			}
		}
	}
	
	//for (x = 0; x < dsp->mute_fragments; x++) {
		//memset(shortdata + dsp->mute_data[x].start, 0, sizeof(int16_t) * (dsp->mute_data[x].end - dsp->mute_data[x].start));
	//}
	
	if (outf) {
		if(af)
		{
			free(af);
			af = NULL;
		}
		return outf;		
	}

	return af;
}
