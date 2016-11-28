// *****************************************************************
//                        BOARD TYPE E
// *****************************************************************

//      General parameters
#define _NB_SUBADD_E				4
#define _ODDDATA_E					0x00000000
#define _EVENDATA_E					0x08000800
#define _MASK_SUBADD_E				{0x00000000,0x00000100,0x00000200,0x00000300}

//		Initialisation parameters
#define _INIT_PREAVERAGING_E		25 // a point/secund
#define _INIT_BOARD_MODE_E			_FIX_RANGE_MODE
#define _INIT_RANGEI_E				0
#define _INIT_RANGEV_E				8

//		Calibration parameters
/*
Pour la mesure de l'offset a faire AVANT le calcul de coeff.
Il y a 1 mesures  faire:
1/ RangeV = 8 RangeI = 0 le resultat ==> X1 entier signe entre -65536 et +65536  
l'offset est DIRECTEMENT la valeur entiere
*/
#define _C_E						25				// secunds
#define	_NB_RANGEV_E				10				// Voltage Range Coef only 9 in autorange
#define	_NB_RANGEI_E				32				// Intensity Range Coef
#define _MAX_OFFSET_E				3000
#define _ACCEPT_CALIBRATION_E		0.1
#define _WAIT_BEFORE_FIRST_CALIBRATION_E (4*60*25) //(25*60*4) // 4 minutes: 25 for _WAIT_BEFORE_FIRST_CALIBRATIONsecund, 60 for minute 
#define _WAIT_BEFORE_CALIBRATION_E	10*_C_E   // in tics units 2 secondes
#define _FULLSCALE_E				63124	//
#define _NB_MEASURE_CALIB_E			14		//  measures to evaluate calib. coeff.
#define _NB_MEASURE_OFFSET_E		1		//  measures for offset 
#define	_TABMESCAL_E	{ _NB_MEASURE_CALIB_E+_NB_MEASURE_OFFSET_E , \
/*	Range V	Range I		Channel	Time		Value*/\
{	{8,		0		,	0	,	15*_C_E	,	10}	,/* To compute the offset*/\
	{7,		18		,	2	,	20*_C_E	,	2e3} ,\
	{7,		19		,	1	,	15*_C_E	,	1e3} ,\
	{8,		19		,	2	,	15*_C_E	,	2e3} ,\
	{6,		18		,	1	,	25*_C_E	,	1e3} ,\
	{6,		17		,	2	,	25*_C_E	,	2e3} ,\
	{5,		17		,	1	,	25*_C_E	,	1e3} ,\
	{5,		16		,	2	,	30*_C_E	,	2e3} ,\
	{4,		16		,	1	,	40*_C_E	,	1e3} ,\
	{7,		27		,	0	,	60*_C_E	,	1e1} ,\
	{7,		15		,	3	,	15*_C_E	,	1e4} ,\
	{7,		31		,	3	,	25*_C_E	,	1e4} ,\
	{3,		15		,	1	,	100*_C_E,	1e3} ,\
	{3,		23		,	0	,	40*_C_E	,	1e1} ,\
	{4,		23		,	0	,	30*_C_E	,	1e1}}}	

#define _CALIBMATRIXV_E {\
/*	0	1	2	3	4	5	6	7	8	9	10	11	12	13  */\
	0,	0,	0,	-1,	1,	-1,	1,	-1,	0,	1,	0,	-1,	0,	0,	 /* 0 */\
	0,	0,	0,	-1,	1,	-1,	0,	0,	0,	1,	0,	-1,	0,	0,	 /* 1 */\
	0,	0,	0,	-1,	0,	0,	0,	0,	0,	1,	0,	-1,	0,	0,	 /* 2 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	-1,	0,	0,	 /* 3 */\
	0,	0,	0,	-1,	1,	-1,	1,	-1,	0,	0,	0,	0,	0,	0,	 /* 4 */\
	0,	0,	0,	-1,	1,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	 /* 5 */\
	0,	0,	0,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /* 6 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /* 7 */\
	-1,	1,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0	} /* 8 */

// There is an extra line in the CalibMatrixI which is use to compute
// the ratio of two correction coefficients of ranges=(4,23) reached by 
// two different paths. This value will be stored in the BOARDPARAMETER
// as a fake last measure.
#define _CALIBMATRIXI_E {\
/*	0	1	2	3	4	5	6	7	8	9	10	11	12	13 */\
	0,	-1,	0,	-1,	1,	-1,	1,	0,	-1,	1,	1,	0,	0,	0,	 /*0 */\
	0,	-1,	0,	-1,	1,	0,	0,	0,	-1,	1,	1,	0,	0,	0,	 /*1 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	-1,	1,	1,	0,	0,	0,	 /*2 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	-1,	1,	1,	0,	0,	0,	 /*3 */\
	0,	0,	0,	-1,	1,	-1,	1,	0,	-1,	1,	0,	0,	0,	0,	 /*4 */\
	0,	0,	0,	-1,	1,	0,	0,	0,	-1,	1,	0,	0,	0,	0,	 /*5 */\
	0,	0,	0,	0,	0,	0,	0,	0,	-1,	1,	0,	0,	0,	0,	 /*6 */\
	-1,	1,	0,	0,	0,	0,	0,	0,	-1,	1,	0,	0,	0,	0,	 /*7 */\
	0,	-1,	0,	-1,	1,	-1,	1,	0,	-1,	2,	0,	-1,	1,	0,	 /*8 */\
	0,	-1,	0,	-1,	1,	0,	0,	0,	-1,	2,	0,	-1,	1,	0,	 /*9 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	-1,	2,	0,	-1,	1,	0,	 /*10 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	-1,	2,	0,	-1,	1,	0,	 /*11 */\
	0,	-1,	0,	-1,	1,	-1,	1,	0,	0,	1,	0,	0,	0,	0,	 /*12 */\
	0,	-1,	0,	-1,	1,	0,	0,	0,	0,	1,	0,	0,	0,	0,	 /*13 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	 /*14 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	 /*15 */\
	0,	0,	0,	-1,	1,	-1,	1,	0,	0,	0,	0,	0,	0,	0,	 /*16 */\
	0,	0,	0,	-1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /*17 */\
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /*18 */\
	-1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 /*19 */\
	0,	-1,	0,	-1,	1,	-1,	1,	0,	0,	1,	0,	-1,	1,	0,	 /*20 */\
	0,	-1,	0,	-1,	1,	0,	0,	0,	0,	1,	0,	-1,	1,	0,	 /*21 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	0,	1,	0,	-1,	1,	0,	 /*22 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	-1,	1,	0,	 /*23 */\
	0,	-1,	0,	-1,	1,	-1,	1,	0,	1,	0,	0,	0,	0,	0,	 /*24 */\
	0,	-1,	0,	-1,	1,	0,	0,	0,	1,	0,	0,	0,	0,	0,	 /*25 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	 /*26 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	0,	0,	 /*27 */\
	0,	-1,	0,	-1,	1,	-1,	1,	0,	0,	0,	1,	0,	0,	0,	 /*28 */\
	0,	-1,	0,	-1,	1,	0,	0,	0,	0,	0,	1,	0,	0,	0,	 /*29 */\
	0,	-1,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	 /*30 */\
	-1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0,	0,	0,	 /*31 */\
	-1,	0,	0,	1,	-1,	1,	-1,	1,	0,	1,	0,	-1,	1,	-1}	 /*32 */
//	0	1	2	3	4	5	6	7	8	9	10	11	12	13

//		Autoscale parameters
#define _FLOOR_E 13107 // if abs(value) < _FLOOR increase the senisibility (32768/2)*0.8
#define _CEIL_E 31562  // if abs(value) > _CEIL decrease the sensibility
#define _WAIT_INCREASING_I_AFTER_DECREASING_E	50
#define _WAIT_DECREASING_V_AFTER_INCREASING_E	50
#define _MAX_NUMBER_OVERRANGE_E 50
#define _MAX_NUMBER_UNDERRANGE_E 50

// le WORD envoye avec A4=0 est le pds faible et contient gamme I scrutateur et gamme V
// le word envoye avec A4=1 est le poids fort et conrient gain I gain V et gamme V
// Les 16 bits de poids fort se refere a [GainI GainV Gammes V]
// Les 16 bits de poids faible se refere a [GammeI Gamme I Scrutateur]
//		RANGE PARAMETERS FOR READING THE VOLTAGE
#define _TABRANGEV_E {\
	/*NbAutoRange*/_NB_RANGEV_E-1,/*NbRange*/_NB_RANGEV_E, \
	/*DefaultRange=*/_INIT_RANGEV_E,	/*bitmasque*/0x07900400,\
		{{1e-5,	0x02100000}, /*0*/\
		{2e-5,	0x01900000}, /*1*/\
		{4e-5,	0x01100000}, /*2*/\
		{8e-5,	0x00900000}, /*3*/\
		{1e-4,	0x06100400}, /*4*/\
		{2e-4,	0x05900400}, /*5*/\
		{4e-4,	0x05100400}, /*6*/\
		{8e-4,	0x04900400}, /*7*/\
		{1.6e-3,	0x04100000}, /*8*/\
		{1.6e-5,	0x00100000}}  /*9*/\
	}

//		RANGE PARAMETERs FOR THE DELIVERED CURRENT 
#define _TABRANGEI_E {\
	/*NbAutoRange*/_NB_RANGEI_E-4,/*NbRange*/_NB_RANGEI_E,\
	/*DefaultRange=*/_INIT_RANGEI_E,	/*bitmasque*/0x6000E0,\
/*0*/ {{1e-11,	0x0000E0}, {2e-11,	0x2000E0}, {4e-11,	0x4000E0}, {8e-11,	0x6000E0},\
/*4*/ {1e-10,	0x0000C0}, {2e-10,	0x2000C0}, {4e-10,	0x4000C0}, {8e-10,	0x6000C0},\
/*8*/ {1e-9,	0x0000A0}, {2e-9,	0x2000A0}, {4e-9,	0x4000A0}, {8e-9,	0x6000A0},\
/*12*/{1e-8,	0x000080}, {2e-8,	0x200080}, {4e-8,	0x400080}, {8e-8,	0x600080},\
/*16*/{1e-7,	0x000040}, {2e-7,	0x200040}, {4e-7,	0x400040}, {8e-7,	0x600040},\
/*20*/{1e-6,	0x000020}, {2e-6,	0x200020}, {4e-6,	0x400020}, {8e-6,	0x600020},\
/*24*/{1e-5,	0x000000}, {2e-5,	0x200000}, {4e-5,	0x400000}, {8e-5,	0x600000},\
/*28*/{1e-8,	0x000060}, {2e-8,	0x200060}, {4e-8,	0x400060}, {8e-8,	0x600060}}}
