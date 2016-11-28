// *****************************************************************
//                        BOARD TYPE G
// *****************************************************************
//      General parameters
#define _NB_SUBADD_G				4
#define _ODDDATA_G					0x08000800
#define _EVENDATA_G					0x00000000
#define _MASK_SUBADD_G				{0x00000000,0x00000100,0x00000200,0x00000300}



//		Initialisation parameters
#define _INIT_PREAVERAGING_G		25 // a point/secund
#define _INIT_BOARD_MODE_G			_FIX_RANGE_MODE
#define _INIT_RANGEI_G				0
#define _INIT_RANGEV_G				3

//		Calibration parameters
#define _C_G						10						// _C secunds
#define	_NB_RANGEV_G				4						// Voltage Range Coef
#define	_NB_RANGEI_G				24						// Intensity Range Coef
#define _WAIT_BEFORE_CALIBRATION_G	25*_C_G					// in tics unit
#define _WAIT_BEFORE_FIRST_CALIBRATION_G 25*60*4 // 4 minutes: 25 for _WAIT_BEFORE_FIRST_CALIBRATIONsecund, 60 for minute 
#define _FULLSCALE_G				52429	//
#define _MAX_OFFSET_G				5000	// largest offset allowed
#define _ACCEPT_CALIBRATION_G		0.1    // |mesure/_REF_C_DAL - 1|<_ACCEPT_C_DALIBRATION
#define _NB_MEASURE_CALIB_G			10		// 10 measures to evaluate calib. coeff.
#define _NB_MEASURE_OFFSET_G		1		// 1 measures for offset 
#define  _TABMESCAL_G	{ _NB_MEASURE_CALIB_G + _NB_MEASURE_OFFSET_G, \
/*	Range V	Range I		Channel	Time	Value*/\
	{{3,	0		,	0	,	20*_C_G	,	1e1}	,	/* measure for offset calculation*/\
	{2,		10		,	1	,	30*_C_G	,	1e3}	,\
	{2,		9		,	2	,	20*_C_G	,	2e3}	,\
	{1,		9		,	1	,	20*_C_G	,	1e3}	,\
	{1,		8		,	2	,	20*_C_G	,	2e3}	,\
	{3,		10		,	2	,	20*_C_G	,	2e3}	,\
	{3,		11		,	1	,	20*_C_G	,	1e3}	,\
	{3,		7		,	3	,	20*_C_G	,	1e4}	,\
	{0,		7		,	1	,	40*_C_G	,	1e3}	,\
	{0,		3		,	3	,	40*_C_G	,	1e4}	,\
	{2,		18		,	0	,	20*_C_G	,	1e1}}	}

#define _CALIBMATRIXV_G {\
/*0		1		2		3		4		5		6		7		8		9	*/\
1	,	0	,	0	,	0	,	-1	,	0	,	1	,	-1	,	0	,	0	,	/* RANGE V 0*/\
0	,	1	,	-1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	/* RANGE V 1*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	/* RANGE V 2*/\
1	,	0	,	0	,	0	,	-1	,	0	,	0	,	0	,	0	,	0	}/* RANGE V 3*/



#define _CALIBMATRIXI_G {\
0	,	1	,	-1	,	1	,	0	,	-1	,	1	,	-1	,	1	,	0	,/* RANGE I 0*/\
0	,	1	,	0	,	0	,	0	,	-1	,	1	,	-1	,	1	,	0	,/* RANGE I 1*/\
1	,	0	,	0	,	0	,	0	,	-1	,	1	,	-1	,	1	,	0	,/* RANGE I 2*/\
1	,	0	,	0	,	0	,	-1	,	0	,	1	,	-1	,	1	,	0	,/* RANGE I 3*/\
0	,	1	,	-1	,	1	,	0	,	-1	,	1	,	0	,	0	,	0	,/* RANGE I 4*/\
0	,	1	,	0	,	0	,	0	,	-1	,	1	,	0	,	0	,	0	,/* RANGE I 5*/\
1	,	0	,	0	,	0	,	0	,	-1	,	1	,	0	,	0	,	0	,/* RANGE I 6*/\
1	,	0	,	0	,	0	,	-1	,	0	,	1	,	0	,	0	,	0	,/* RANGE I 7*/\
\
0	,	1	,	-1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,/* RANGE I 8*/\
0	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,/* RANGE I 9*/\
1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,/* RANGE I 10*/\
1	,	0	,	0	,	0	,	-1	,	1	,	0	,	0	,	0	,	0	,/* RANGE I 11*/\
\
0	,	1	,	-1	,	1	,	0	,	0	,	0	,	1	,	-1	,	0	,/* RANGE I 12*/\
0	,	1	,	0	,	0	,	0	,	0	,	0	,	1	,	-1	,	0	,/* RANGE I 13*/\
1	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	-1	,	0	,/* RANGE I 14*/\
1	,	0	,	0	,	0	,	-1	,	1	,	0	,	1	,	-1	,	0	,/* RANGE I 15*/\
\
-1	,	1	,	-1	,	1	,	0	,	0	,	0	,	0	,	0	,	1	,/* RANGE I 16*/\
-1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,/* RANGE I 17*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,/* RANGE I 18*/\
0	,	0	,	0	,	0	,	-1	,	1	,	0	,	0	,	0	,	1	,/* RANGE I 19*/\
\
-1	,	1	,	-1	,	1	,	0	,	0	,	0	,	1	,	-1	,	1	,/* RANGE I 20*/\
-1	,	1	,	0	,	0	,	0	,	0	,	0	,	1	,	-1	,	1	,/* RANGE I 21*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	1	,	-1	,	1	,/* RANGE I 22*/\
0	,	0	,	0	,	0	,	-1	,	1	,	0	,	1	,	-1	,	1	}/* RANGE I 23*/\


//		Autoscale parameters
#define _FLOOR_G 11800 // if abs(value) < _FLOOR increase the senisibility
#define _CEIL_G 28800  // if abs(value) > _CEIL decrease the sensibility
#define _WAIT_INCREASING_I_AFTER_DECREASING_G	50
#define _WAIT_DECREASING_V_AFTER_INCREASING_G	50
#define _MAX_NUMBER_OVERRANGE_G 50
#define _MAX_NUMBER_UNDERRANGE_G 50

//		RANGE PARAMETERs FOR THE DELIVERED CURRENT 
#define _TABRANGEI_G {\
	/*NbAutoRange*/_NB_RANGEI_G,/*NbRange*/_NB_RANGEI_G,\
	/*DefaultRange=*/_INIT_RANGEI_G,	/*bitmasque*/0x07700000,\
	{{1.25e-8,	0x05100000}, {2.5e-8,	0x05300000},\
	{5e-8,		0x05500000}, {1e-7,		0x05700000},	\
	{1.25e-7,	0x04100000}, {2.5e-7,	0x04300000},	\
	{5e-7,		0x04500000}, {1e-6,		0x04700000},	\
	{1.25e-6,	0x03100000}, {2.5e-6,	0x03300000},	\
	{5e-6,		0x03500000}, {1e-5,		0x03700000},	\
	{1.25e-5,	0x02100000}, {2.5e-5,	0x02300000},	\
	{5e-5,		0x02500000}, {1e-4,		0x02700000},	\
	{1.25e-4,	0x01100000}, {2.5e-4,	0x01300000},	\
	{5e-4,		0x01500000}, {1e-3,		0x01700000},	\
	{1.25e-3,	0x00100000}, {2.5e-3,	0x00300000},	\
	{5e-3,		0x00500000}, {1e-2,		0x00700000}}}	

//		RANGE PARAMETERS FOR READING THE VOLTAGE
#define _TABRANGEV_G {\
	/*NbAutoRange*/_NB_RANGEV_G,/*NbRange*/_NB_RANGEV_G, \
	/*DefaultRange=*/_INIT_RANGEV_G,/*bitmasque*/0x00000060,\
		{{1.25e-3,	0x00000060},\
		{2.5e-3,	0x00000040},\
		{5e-3,		0x00000020},\
		{1e-2,		0x00000000}}}

