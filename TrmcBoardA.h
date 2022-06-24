// SPDX-License-Identifier: LGPL-3.0-or-later
// *****************************************************************
//                        BOARD TYPE A : RESISTOR LOW PRECISION
// *****************************************************************
//      General parameters
#define _NB_SUBADD_A				1
#define _ODDDATA_A					0x00000000
#define _EVENDATA_A					0x04000400
#define _MASK_SUBADD_A				{0}

//		Initialisation parameters
#define _INIT_PREAVERAGING_A		25 // a point/secund
#define _INIT_BOARD_MODE_A			_FIX_RANGE_MODE
#define _INIT_RANGEI_A				0
#define _INIT_RANGEV_A				0

//		Calibration parameters
#define _NB_MEASURE_CALIB_A			1
#define _ACCEPT_CALIBRATION_A		0.1    // |mesure/_REF_CAL - 1|<_ACCEPT_CALIBRATION_A
#define _MAX_OFFSET_A				1000
#define _FULLSCALE_A				59602	//
#define _WAIT_BEFORE_CALIBRATION_A  25*5		// to stabilize 5 secunds
#define _WAIT_BEFORE_FIRST_CALIBRATION_A 25*60*4 // 4 minutes: 25 for _WAIT_BEFORE_FIRST_CALIBRATIONsecund, 60 for minute 
#define _NB_MEASURE_OFFSET_A		0
#define	_TABMESCAL_A	{ _NB_MEASURE_CALIB_A+_NB_MEASURE_OFFSET_A , \
/*	Range V	Range I		Channel	Time		Value*/\
	{ { 0,		2		,	0	,	25*5	,	1000 } } }
#define   _CALIBMATRIXI_A {1,1,1,1,1,1} 
#define   _CALIBMATRIXV_A {1}

//		Autoscale parameters
#define _FLOOR_A 2500 // if abs(value) < _FLOOR increase the senisibility
#define _CEIL_A 29789  // if abs(value) > _CEIL decrease the sensibility
#define  _WAIT_INCREASING_I_AFTER_DECREASING_A	20
#define _WAIT_DECREASING_V_AFTER_INCREASING_A 20 // unusefull for board A
#define _MAX_NUMBER_OVERRANGE_A		10
#define _MAX_NUMBER_UNDERRANGE_A	10
#define _NB_RANGEV_A				1 
#define _NB_RANGEI_A				6


//		RANGE PARAMETERs FOR THE DELIVERED CURRENT 
#define _TABRANGEI_A {\
	/*NbAutoRange*/_NB_RANGEI_A,/*NbRange*/_NB_RANGEI_A,\
	/*DefaultRange=*/_INIT_RANGEI_A,	/*bitmasque*/0x00700070,\
		{{1.e-7,	0x00500050},\
		{1.e-6,	0x00400040},\
		{1.e-5,	0x00300030},\
		{1.e-4,	0x00200020},\
		{1.e-3,	0x00100010},\
		{1.e-2,	0x00000000}}}


//		RANGE PARAMETERS FOR READING THE VOLTAGE
#define _TABRANGEV_A {/*NbAutoRange*/_NB_RANGEV_A,/*NbRange*/_NB_RANGEV_A, \
				/*DefaultRange=*/_INIT_RANGEV_A,	/*bitmasque*/0x0000,\
		{{1.e-2,	0x0000}}}
 

