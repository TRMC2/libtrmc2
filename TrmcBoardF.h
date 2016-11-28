// *****************************************************************
//                        BOARD TYPE F
// *****************************************************************

//      General parameters
#define _NB_SUBADD_F				4
#define _ODDDATA_F					0x00000000
#define _EVENDATA_F					0x08000800
#define _MASK_SUBADD_F				{0x00000000,0x00000100,0x00000200,0x00000300}

//		Initialisation parameters
#define _INIT_PREAVERAGING_F		25 // a point/secund
#define _INIT_BOARD_MODE_F			_FIX_RANGE_MODE
#define _INIT_RANGEI_F				0
#define _INIT_RANGEV_F				8

//		Calibration parameters
#define _CORRECTION_TO_ZERO_RESISTOR 0.79   // the resistor for offset is not STRICTLY 0 (10ohm)
#define _C_F						25		// secunds
#define	_NB_RANGEV_F				10		// Voltage Range Coef only 9 in autorange
#define	_NB_RANGEI_F				32		// Intensity Range Coef
#define _MAX_OFFSET_F				3000
#define _ACCEPT_CALIBRATION_F		0.1		// each measure has to be closer to actual than 10% 
#define _WAIT_BEFORE_CALIBRATION_F	10*_C_F	// in tics units 2 secondes
#define _WAIT_BEFORE_FIRST_CALIBRATION_F 25*60*4 // 4 minutes: 25 for _WAIT_BEFORE_FIRST_CALIBRATIONsecund, 60 for minute 
#define _FULLSCALE_F				63018	//
#define _NB_MEASURE_CALIB_F			14		//  measures to evaluate calib. coeff.
#define _NB_MEASURE_OFFSET_F		1		//  measures for offset 
#define	_TABMESCAL_F	{ _NB_MEASURE_CALIB_F+_NB_MEASURE_OFFSET_F , \
/*	Range V	Range I		Channel	Time		Value*/\
	{{8,		0		,	0	,	15*_C_F	,	10}	,/* To compute the offset*/\
	{8,		10		,	2	,	25*_C_F	,	2e3}	,\
	{8,		11		,	1	,	15*_C_F	,	1e3}	,\
	{7,		10		,	1	,	20*_C_F	,	1e3}	,\
	{7,		9		,	2	,	20*_C_F	,	2e3}	,\
	{6,		9		,	1	,	30*_C_F	,	1e3}	,\
	{6,		8		,	2	,	50*_C_F	,	2e3}	,\
	{5,		8		,	1	,	100*_C_F,	1e3}	,\
	{8,		7		,	3	,	50*_C_F	,	10e3}	,\
	{4,		7		,	1	,	130*_C_F,	1e3}	,\
	{4,		15		,	0	,	20*_C_F	,	10}		,\
	{4,		31		,	0	,	20*_C_F	,	10}		,\
	{8,		19		,	0	,	15*_C_F,	10}  	,\
	{9,		6		,	1	,	100*_C_F,	1e3}	,\
	{5,		31		,	0	,	20*_C_F	,	10}}}	

#define _CALIBMATRIXV_F {\
/*	0	1	2	3	4	5	6	7	8	9	10	11	12	13  */\
	0,-1,	0,	0,	0,	0,	0,	2,	-1,	0,	0,	0,	-1,	0,	 /* 0 */\
	0,	0,	-1,	1,	-1,	1,	-1,	1,	-1,	0,	0,	0,	0,	0,	 /* 1 */\
	0,	0,	-1,	1,	-1,	0,	0,	1,	-1,	0,	0,	0,	0,	0,	 /* 2 */\
	0,	0,	-1,	0,	0,	0,	0,	1,	-1,	0,	0,	0,	0,	0,	 /* 3 */\
	-1,	0,	0,	0,	0,	0,	0,	1,	-1,	0,	0,	0,	0,	0,	 /* 4 */\
	0,	0,	-1,	1,	-1,	1,	-1,	0,	0,	0,	0,	0,	0,	0,	 /* 5 */\
	0,	0,	-1,	1,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /* 6 */\
	0,	0,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /* 7 */\
	-1,	0,	0 ,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	} /* 8 */

// There is an extra line in the CalibMatrixI which is used to compute
// the ratio of two correction coefficients of ranges=(4,23) reached by 
// two different paths. This value will be stored in the BOARDPARAMETER
// as a fake last measure.
#define _CALIBMATRIXI_F {\
/*	0	1	2	3	4	5	6	7	8	9	10	11	12	13 */\
	0,	-1,	-1,	1,	-1,	1,	0,	2,	-1,	0,	1,	-1,	0,	0,	 /*0 */\
	0,	-1,	-1,	1,	0,	0,	0,	2,	-1,	0,	1,	-1,	0,	0,	 /*1 */\
	0,	-1,	0,	0,	0,	0,	0,	2,	-1,	0,	1,	-1,	0,	0,	 /*2 */\
	-1,	0,	0,	0,	0,	0,	0,	2,	-1,	0,	1,	-1,	0,	0,	 /*3 */\
	0,	-1,	-1,	1,	-1,	1,	0,	1,	0,	0,	0,	0,	0,	0,	 /*4 */\
	0,	-1,	-1,	1,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	 /*5 */\
	0,	-1,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	 /*6 */\
	-1,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	0,	 /*7 */\
	0,	0,	-1,	1,	-1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	 /*8 */\
	0,	0,	-1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /*9 */\
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /*10 */\
	-1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /*11 */\
	0,	-1,	-1,	1,	-1,	1,	0,	1,	-1,	1,	0,	0,	0,	0,	 /*12 */\
	0,	-1,	-1,	1,	0,	0,	0,	1,	-1,	1,	0,	0,	0,	0,	 /*13 */\
	0,	-1,	0,	0,	0,	0,	0,	1,	-1,	1,	0,	0,	0,	0,	 /*14 */\
	-1,	0,	0,	0,	0,	0,	0,	1,	-1,	1,	0,	0,	0,	0,	 /*15 */\
	0,	-1,	-1,	1,	-1,	1,	0,	0,	0,	0,	0,	1,	0,	0,	 /*16 */\
	0,	-1,	-1,	1,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	 /*17 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	 /*18 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	 /*19 */\
	0,	0,	-1,	1,	-1,	1,	0,	-1,	0,	0,	0,	1,	0,	0,	 /*20 */\
	0,	0,	-1,	1,	0,	0,	0,	-1,	0,	0,	0,	1,	0,	0,	 /*21 */\
	0,	0,	0,	0,	0,	0,	0,	-1,	0,	0,	0,	1,	0,	0,	 /*22 */\
	-1,	1,	0,	0,	0,	0,	0,	-1,	0,	0,	0,	1,	0,	0,	 /*23 */\
	0,	-1,	-1,	1,	-1,	1,	0,	0,	-1,	1,	0,	1,	0,	0,	 /*24 */\
	0,	-1,	-1,	1,	0,	0,	0,	0,	-1,	1,	0,	1,	0,	0,	 /*25 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	-1,	1,	0,	1,	0,	0,	 /*26 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	-1,	1,	0,	1,	0,	0,	 /*27 */\
	0,	-1,	-1,	1,	-1,	1,	0,	1,	-1,	0,	1,	0,	0,	0,	 /*28 */\
	0,	-1,	-1,	1,	0,	0,	0,	1,	-1,	0,	1,	0,	0,	0,	 /*29 */\
	0,	-1,	0,	0,	0,	0,	0,	1,	-1,	0,	1,	0,	0,	0,	 /*30 */\
	-1,	0,	0,	0,	0,	0,	0,	1,	-1,	0,	1,	0,	0,	0,	 /*31 */\
	1,	0,	-1,	1,	-1,	1,	-1,	-1,	1,	0,	-1,	0,	0,	1}	 /*32 */
//	0	1	2	3	4	5	6	7	8	9	10	11	12	13

//		Autoscale parameters
#define _FLOOR_F 13107 // if abs(value) < _FLOOR increase the senisibility (32768/2)*0.8
#define _CEIL_F 31562  // if abs(value) > _CEIL decrease the sensibility
#define _WAIT_INCREASING_I_AFTER_DECREASING_F	50
#define _WAIT_DECREASING_V_AFTER_INCREASING_F	50
#define _MAX_NUMBER_OVERRANGE_F 50
#define _MAX_NUMBER_UNDERRANGE_F 50

// le WORD envoye avec A4=0 est le pds faible et contient gamme I scrutateur et gamme V
// le word envoye avec A4=1 est le poids fort et conrient gain I gain V et gamme V
// Les 16 bits de poids fort se refere a [GainI GainV Gammes V]
// Les 16 bits de poids faible se refere a [GammeI Gamme I Scrutateur]
//		RANGE PARAMETERS FOR READING THE VOLTAGE
#define _TABRANGEV_F {\
	/*NbAutoRange*/_NB_RANGEV_F-1,/*NbRange*/_NB_RANGEV_F, \
	/*DefaultRange=*/_INIT_RANGEV_F,	/*bitmasque*/0x07900400,\
		{{5e-6,	0x02100000}, /*0*/\
		{1e-5,	0x01900000}, /*1*/\
		{2e-5,	0x01100000}, /*2*/\
		{4e-5,	0x00900000}, /*3*/\
		{8e-5,	0x00100000}, /*4*/\
		{1e-4,	0x05900400}, /*5*/\
		{2e-4,	0x05100400}, /*6*/\
		{4e-4,	0x04900400}, /*7*/\
		{8e-4,	0x04100000}, /*8*/\
		{5e-5,	0x06100400}}  /*9*/\
	}


//		RANGE PARAMETERs FOR THE DELIVERED CURRENT 
#define _TABRANGEI_F {\
	/*NbAutoRange*/_NB_RANGEI_F-4,/*NbRange*/_NB_RANGEI_F,\
	/*DefaultRange=*/_INIT_RANGEI_F,	/*bitmasque*/0x6000E0,\
/*0*/	{{1e-9,0x000060}, {2e-9,0x200060}, {4e-9,	0x400060}, {8e-9,	0x600060},\
/*4*/	{1e-8,0x000040}, {2e-8,	0x200040}, {4e-8,	0x400040}, {8e-8,	0x600040},\
/*8*/	{1e-7,0x000020}, {2e-7,	0x200020}, {4e-7,	0x400020}, {8e-7,	0x600020},\
/*12*/	{1e-6,0x000000}, {2e-6,	0x200000}, {4e-6,	0x400000}, {8e-6,	0x600000},\
/*16*/	{1e-5,0x0000C0}, {2e-5,	0x2000C0}, {4e-5,	0x4000C0}, {8e-5,	0x6000C0},\
/*20*/	{1e-4,0x0000A0}, {2e-4,	0x2000A0}, {4e-4,	0x4000A0}, {8e-4,	0x6000A0},\
/*24*/	{1e-3,0x000080}, {2e-3,	0x200080}, {4e-3,	0x400080}, {8e-3,	0x600080},\
/*28*/	{1e-6,0x0000E0}, {2e-6,	0x2000E0}, {4e-6,	0x4000E0}, {8e-6,	0x6000E0}}}
