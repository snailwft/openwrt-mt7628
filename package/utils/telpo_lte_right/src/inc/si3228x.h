/*
** Copyright (c) 2015 by Silicon Laboratories
**
** $Id: si3228x.h 4878 2015-04-17 21:36:59Z nizajerk $
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.     
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#ifndef SI3228XH_H
#define SI3228XH_H

#include "proslic.h"


#define SI3228X_CHAN_PER_DEVICE     2

/*
** SI3228X DataTypes/Function Definitions 
*/

#define NUMIRQ 4


typedef ProSLIC_DCfeed_Cfg Si3228x_DCfeed_Cfg;
typedef Si3228x_DCfeed_Cfg* Si3228x_DCfeed_Cfg_ptr;
/*
** Defines structure for general configuration and the dcdc converter
*/
typedef struct {
    uInt8 device_key;    /* Used to prevent loaded coeffs for another device */
    bomOptionsType bomOpt;
    batRailType  batType;
    ramData bat_hyst;
    ramData vbatr_expect; /* default - this is overwritten by ring preset */
    ramData vbath_expect;  /* default - this is overwritten by dc feed preset */
    ramData pwrsave_timer;
    ramData offhook_thresh; 
    ramData vbat_track_min;
    ramData vbat_track_min_rng;
    ramData pwrsave_dbi;
    ramData dcdc_ana_scale;
    ramData vov_bat_pwrsave_min;
    ramData vov_bat_pwrsave_min_rng;
    ramData therm_dbi;
    ramData cpump_dbi;
    ramData dcdc_verr;
    ramData dcdc_verr_hyst;
    ramData dcdc_oithresh_lo;
    ramData dcdc_oithresh_hi;
    ramData pd_uvlo; 
    ramData pd_ovlo;
    ramData pd_oclo;
    ramData pd_swdrv;
    ramData dcdc_uvhyst; 
    ramData dcdc_uvthresh; 
    ramData dcdc_ovthresh; 
    ramData dcdc_oithresh; 
    ramData dcdc_swdrv_pol; 
    ramData dcdc_uvpol;
    ramData dcdc_vref_man;
    ramData dcdc_vref_ctrl;
    ramData dcdc_rngtype; 
    ramData dcdc_ana_vref;
    ramData dcdc_ana_gain; 
    ramData dcdc_ana_toff; 
    ramData dcdc_ana_tonmin; 
    ramData dcdc_ana_tonmax; 
    ramData dcdc_ana_dshift; 
    ramData dcdc_ana_lpoly; 
    ramData dcdc_aux_invert;
    ramData dcdc_cpump_lp;
    ramData dcdc_cpump_pulldown;
    ramData dcdc_lift_en;
    ramData coef_p_hvic;
    ramData p_th_hvic;
    uInt8 vdc_range;
    uInt8 autoRegister;
    uInt8 irqen1;
    uInt8 irqen2;
    uInt8 irqen3;
    uInt8 irqen4;
    uInt8 enhance;
    ramData scale_kaudio;
    uInt8 zcal_en;
    ramData lkg_ofhk_offset;
    ramData lkg_lb_offset;
    ramData vbath_delta;
    ramData uvthresh_max;
    ramData uvthresh_scale;
    ramData uvthresh_bias;
} Si3228x_General_Cfg;


/*
** Defines structure for configuring pcm
*/
typedef struct {
    uInt8 pcmFormat;
    uInt8 widebandEn;
    uInt8 pcm_tri;
    uInt8 tx_edge;
    uInt8 alaw_inv;
} Si3228x_PCM_Cfg;

/*
** Defines structure for configuring pulse metering
*/
typedef struct {
    ramData pm_amp_thresh;
    uInt8 pm_freq;
    uInt8 pm_auto;
    ramData pm_active;
    ramData pm_inactive;
} Si3228x_PulseMeter_Cfg;

/*
** Defines structure for configuring FSK generation
*/
typedef struct {
    ramData fsk01;
    ramData fsk10;
    ramData fskamp0;
    ramData fskamp1;
    ramData fskfreq0;
    ramData fskfreq1;
    uInt8 eightBit;
    uInt8 fskdepth;
} Si3228x_FSK_Cfg;

/*
** Defines structure for configuring dtmf decode
*/
typedef struct {
    ramData dtmfdtf_b0_1;
    ramData dtmfdtf_b1_1;
    ramData dtmfdtf_b2_1;
    ramData dtmfdtf_a1_1;
    ramData dtmfdtf_a2_1;
    ramData dtmfdtf_b0_2;
    ramData dtmfdtf_b1_2;
    ramData dtmfdtf_b2_2;
    ramData dtmfdtf_a1_2;
    ramData dtmfdtf_a2_2;
    ramData dtmfdtf_b0_3;
    ramData dtmfdtf_b1_3;
    ramData dtmfdtf_b2_3;
    ramData dtmfdtf_a1_3;
    ramData dtmfdtf_a2_3;
} Si3228x_DTMFDec_Cfg;

/*
** Defines structure for configuring impedance synthesis
*/
typedef struct {
    ramData zsynth_b0;
    ramData zsynth_b1;
    ramData zsynth_b2;
    ramData zsynth_a1;
    ramData zsynth_a2;
    uInt8 ra;
} Si3228x_Zsynth_Cfg;

/*
** Defines structure for configuring hybrid
*/
typedef struct {
    ramData ecfir_c2;
    ramData ecfir_c3;
    ramData ecfir_c4;
    ramData ecfir_c5;
    ramData ecfir_c6;
    ramData ecfir_c7;
    ramData ecfir_c8;
    ramData ecfir_c9;
    ramData ecfir_b0;
    ramData ecfir_b1;
    ramData ecfir_a1;
    ramData ecfir_a2;
} Si3228x_hybrid_Cfg;

/*
** Defines structure for configuring modem tone detect
*/
typedef struct {
    ramData rxmodpwr;
    ramData rxpwr;
    ramData modem_gain;
    ramData txmodpwr;
    ramData txpwr;
} Si3228x_modemDet_Cfg;

/*
** Defines structure for configuring audio eq
*/

typedef struct {
    ramData txaceq_c0;
    ramData txaceq_c1;
    ramData txaceq_c2;
    ramData txaceq_c3;

    ramData rxaceq_c0;
    ramData rxaceq_c1;
    ramData rxaceq_c2;
    ramData rxaceq_c3;
} Si3228x_audioEQ_Cfg;



/*
** Defines structure for configuring audio gain
*/

typedef ProSLIC_audioGain_Cfg Si3228x_audioGain_Cfg;


typedef struct {
    Si3228x_audioEQ_Cfg audioEQ;
    Si3228x_hybrid_Cfg hybrid;
    Si3228x_Zsynth_Cfg zsynth;
    ramData txgain;
    ramData rxgain;
    ramData rxachpf_b0_1;
    ramData  rxachpf_b1_1;
    ramData  rxachpf_a1_1;
    int16 txgain_db; /*overall gain associated with this configuration*/
    int16 rxgain_db;
} Si3228x_Impedance_Cfg;



/*
** Defines structure for configuring tone generator
*/
typedef struct {
    Oscillator_Cfg osc1;
    Oscillator_Cfg osc2;
    uInt8 omode;
} Si3228x_Tone_Cfg; 

/*
** Defines structure for configuring ring generator
*/
typedef struct {
    ramData rtper;
    ramData freq;
    ramData amp;
    ramData phas;
    ramData offset;
    ramData slope_ring;
    ramData iring_lim;
    ramData rtacth;
    ramData rtdcth;
    ramData rtacdb;
    ramData rtdcdb;
    ramData vov_ring_bat;
    ramData vov_ring_gnd;
    ramData vbatr_expect;
    uInt8 talo;
    uInt8 tahi;
    uInt8 tilo;
    uInt8 tihi;
    ramData adap_ring_min_i;
    ramData counter_iring_val;
    ramData counter_vtr_val;
    ramData ar_const28;
    ramData ar_const32;
    ramData ar_const38;
    ramData ar_const46;
    ramData rrd_delay;
    ramData rrd_delay2;
    ramData vbat_track_min_rng;
    uInt8 ringcon;
    uInt8 userstat;
    ramData vcm_ring;
    ramData vcm_ring_fixed;
    ramData delta_vcm;
    ramData dcdc_rngtype;
    ramData vov_dcdc_slope;
    ramData vov_dcdc_os;
    ramData vov_ring_bat_max;
    ramData efpi_period;
    ramData efpi_phase;
} Si3228x_Ring_Cfg;



/*
** This defines names for the interrupts in the ProSLIC
*/
typedef enum {
/* IRQ1 */
IRQ_OSC1_T1_SI3228X = 0,   
IRQ_OSC1_T2_SI3228X,
IRQ_OSC2_T1_SI3228X,
IRQ_OSC2_T2_SI3228X,
IRQ_RING_T1_SI3228X,
IRQ_RING_T2_SI3228X,
IRQ_FSKBUF_AVAIL_SI3228X,
IRQ_VBAT_SI3228X,
/* IRQ2 */
IRQ_RING_TRIP_SI3228X = 8,
IRQ_LOOP_STAT_SI3228X,
IRQ_LONG_STAT_SI3228X,
IRQ_VOC_TRACK_SI3228X,
IRQ_DTMF_SI3228X,
IRQ_INDIRECT_SI3228X,
IRQ_TXMDM_SI3228X,
IRQ_RXMDM_SI3228X,
/* IRQ3 */
IRQ_P_HVIC_SI3228X = 16,  
IRQ_P_THERM_SI3228X,
IRQ_PQ3_SI3228X,
IRQ_PQ4_SI3228X,
IRQ_PQ5_SI3228X,
IRQ_PQ6_SI3228X,
IRQ_DSP_SI3228X,
IRQ_MADC_FS_SI3228X,
/* IRQ4 */
IRQ_USER_0_SI3228X = 24, 
IRQ_USER_1_SI3228X,
IRQ_USER_2_SI3228X,
IRQ_USER_3_SI3228X,
IRQ_USER_4_SI3228X,
IRQ_USER_5_SI3228X,
IRQ_USER_6_SI3228X,
IRQ_USER_7_SI3228X
}Si3228xProslicInt;

#endif
