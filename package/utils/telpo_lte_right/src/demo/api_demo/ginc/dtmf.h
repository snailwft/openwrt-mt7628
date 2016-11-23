#ifndef _ASTERISK_DTMF_H
#define _ASTERISK_DTMF_H
#define DSP_FEATURE_DIGIT_DETECT	                (1 << 3)
#define DSP_FEATURE_BUSY_DETECT		(1 << 1)
#define DSP_DIGITMODE_MF			1	
#define DSP_DIGITMODE_NOQUELCH		(1 << 8)	
#define DSP_DIGITMODE_RELAXDTMF		(1 << 11)		/*!< "Radio" mode (relaxed DTMF) */
#define DSP_FEATURE_SILENCE_SUPPRESS	(1 << 0)
#define	MAX_DTMF_DIGITS		128

#define DEF_DTMF_HITS_TO_BEGIN	2
#define AST_FRAME_DTMF AST_FRAME_DTMF_END

/* How many successive misses needed to consider end of a digit
 * IE. Override with dtmf_misses_to_end=4 in dsp.conf
 */
#define DEF_DTMF_MISSES_TO_END	3
/*! Remember last 15 units */
#define DSP_HISTORY 		15
/* DTMF goertzel size */
#define DTMF_GSIZE		102

#define DTMF_THRESHOLD		8.0e7
#define DTMF_RELATIVE_PEAK_COL	6.3     /* 8dB */
#define DTMF_RELATIVE_PEAK_ROW	6.3     /* 8dB */
#define DTMF_TO_TOTAL_ENERGY	42.0
/* DTMF goertzel size */
#define DTMF_GSIZE		102
#define SAMPLE_RATE		8000
#define	DSP_DIGITMODE_DTMF			0				/*!< Detect DTMF digits */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEF_DTMF_NORMAL_TWIST		6.31	 /* 8.0dB */
#define DEF_DTMF_REVERSE_TWIST		2.51	 /* 4.01dB */

#define DEF_RELAX_DTMF_NORMAL_TWIST	6.31	 /* 8.0dB */
#define DEF_RELAX_DTMF_REVERSE_TWIST	3.98	 /* 6.0dB */
#define uint32 unsigned int
typedef unsigned int uint32_t;

enum ast_frame_type {
	/*! DTMF end event, subclass is the digit */
	AST_FRAME_DTMF_END = 1,
	/*! Voice data, subclass is AST_FORMAT_* */
	AST_FRAME_VOICE,
	/*! Video frame, maybe?? :) */
	AST_FRAME_VIDEO,
	/*! A control frame, subclass is AST_CONTROL_* */
	AST_FRAME_CONTROL,
	/*! An empty, useless frame */
	AST_FRAME_NULL,
	/*! Inter Asterisk Exchange private frame type */
	AST_FRAME_IAX,
	/*! Text messages */
	AST_FRAME_TEXT,
	/*! Image Frames */
	AST_FRAME_IMAGE,
	/*! HTML Frame */
	AST_FRAME_HTML,
	/*! Comfort Noise frame (subclass is level of CNG in -dBov), 
	    body may include zero or more 8-bit quantization coefficients */
	AST_FRAME_CNG,
	/*! Modem-over-IP data streams */
	AST_FRAME_MODEM,	
	/*! DTMF begin event, subclass is the digit */
	AST_FRAME_DTMF_BEGIN,
};

typedef struct {
	int v2;
	int v3;
	int chunky;
	int fac;
	int samples;
} goertzel_state_t;
typedef struct {
	int value;
	int power;
} goertzel_result_t;

typedef struct
{
	goertzel_state_t row_out[4];
	goertzel_state_t col_out[4];
	int hits;			/* How many successive hits we have seen already */
	int misses;			/* How many successive misses we have seen already */
	int lasthit;
	int current_hit;
	float energy;
	int current_sample;
	int mute_samples;
} dtmf_detect_state_t;


typedef struct
{
	char digits[MAX_DTMF_DIGITS + 1];
	int digitlen[MAX_DTMF_DIGITS + 1];
	int current_digits;
	int detected_digits;
	int lost_digits;
	dtmf_detect_state_t dtmf;
		
} digit_detect_state_t;

union ast_frame_subclass {
	int integer;
	long long codec;
};

struct ast_frame {
	/*! Kind of frame */
	enum ast_frame_type frametype;				
	/*! Subclass, frame dependent */
	union ast_frame_subclass subclass;
	/*! Length of data */
	int datalen;				
	/*! Number of samples in this frame */
	int samples;			
	/*! Pointer to actual data */
	union { void *ptr; char pad[8]; } data;
	long len;
};

typedef struct {
	int start;
	int end;
} fragment_t;

struct ast_dsp {
	struct ast_frame f;	
	int mute_fragments;
	int features;
	int digitmode;
	digit_detect_state_t digit_state;
	int dtmf_began;
	int historicnoise[DSP_HISTORY];
	int historicsilence[DSP_HISTORY];
	fragment_t mute_data[5];
};


static const float dtmf_row[] = {
	697.0,  770.0,  852.0,  941.0
};
static const float dtmf_col[] = {
	1209.0, 1336.0, 1477.0, 1633.0
};

static const char dtmf_positions[] = "123A" "456B" "789C" "*0#D";
static float relax_dtmf_reverse_twist;	/* AT&T = 6dB */
static float dtmf_reverse_twist;	/* AT&T = 4dB */
static float relax_dtmf_normal_twist;	/* AT&T = 8dB */
static float dtmf_normal_twist;	/* AT&T = 8dB */
static int dtmf_hits_to_begin;		/* How many successive hits needed to consider begin of a digit */
static int dtmf_misses_to_end;		/* How many successive misses needed to consider end of a digit */

#define ARRAY_LEN(a) (size_t) (sizeof(a) / sizeof(0[a]))
void dtmf_init(struct ast_dsp * dsp);
struct ast_frame *dsp_process(struct ast_dsp *dsp, struct ast_frame *af);

#endif 
