// *****************************************************************
//                        BOARD TYPE D
// *****************************************************************
//      General parameters
#define _NB_SUBADD_D				1
#define _ODDDATA_D					0x08000800
#define _EVENDATA_D					0x00000000
#define _MASK_SUBADD_D				{0}


//		Initialisation parameters
#define _INIT_PREAVERAGING_D		25 // a point/secund
#define _INIT_BOARD_MODE_D			_FIX_RANGE_MODE
#define _INIT_RANGEI_D				0
#define _INIT_RANGEV_D				3

//		Calibration parameters
#define _C_D						10						// _C secunds
#define	_NB_RANGEV_D				4						// Voltage Range Coef
#define	_NB_RANGEI_D				24						// Intensity Range Coef
#define _WAIT_BEFORE_CALIBRATION_D	25*_C_D					// in tics unit
#define _WAIT_BEFORE_FIRST_CALIBRATION_D 25*60*4 // 4 minutes: 25 for _WAIT_BEFORE_FIRST_CALIBRATIONsecund, 60 for minute 
#define _FULLSCALE_D				58385	//
#define _MAX_OFFSET_D				5000	// largest offset allowed
#define _ACCEPT_CALIBRATION_D		0.1    // |mesure/_REF_C_DAL - 1|<_ACCEPT_C_DALIBRATION
#define _NB_MEASURE_CALIB_D			9		// 9 measures to evaluate calib. coeff.
#define _NB_MEASURE_OFFSET_D		2		// 2 measures for offset 
#define  _TABMESCAL_D	{ _NB_MEASURE_CALIB_D + _NB_MEASURE_OFFSET_D, \
/*	Range V	Range I		Channel	Time	Value*/\
	{{3,		0		,	0	,	25*_C_D	,	1e3}	,	/* first measure for offset calculation*/\
	{3,		11		,	0	,	25*_C_D	,	1e3}	,	/* second measure for offset calculation*/\
	{2,		10		,	0	,	25*_C_D	,	1e3}	,	\
	{2,		9		,	0	,	25*_C_D	,	1e3}	,	\
	{1,		9		,	0	,	50*_C_D	,	1e3}	,	\
	{1,		8		,	0	,	50*_C_D	,	1e3}	,	\
	{0,		8		,	0	,	75*_C_D	,	1e3}	,	\
	{3,		10		,	0	,	25*_C_D	,	1e3}	,	\
	{3,		11		,	0	,	25*_C_D	,	1e3}	,	\
	{1,		7		,	0	,	25*_C_D	,	1e3}	,	\
	{0,		3		,	0	,	75*_C_D	,	1e3	}}}
#define _CALIBMATRIXV_D {\
-1	,	1	,	-1	,	1	,	-1	,	0	,	0	,	0	,	0,	/* RANGE V 0*/\
-1	,	1	,	-1	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE V 1*/\
-1	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE V 2*/\
0	,	0	,	0	,	0	,	0	,	-1	,	0	,	0	,	0}/* RANGE V 3*/
/*1		2		3		4		5		6		7		8		9*/\

#define _CALIBMATRIXI_D {\
-2	,	2	,	-2	,	2	,	-1	,	1	,	-1	,	0	,	1,	/* RANGE I 0*/\
-2	,	2	,	-1	,	1	,	-1	,	1	,	-1	,	0	,	1,	/* RANGE I 1*/\
-1	,	1	,	-1	,	1	,	-1	,	1	,	-1	,	0	,	1,	/* RANGE I 2*/\
-1	,	1	,	-1	,	1	,	-1	,	0	,	0	,	0	,	1,	/* RANGE I 3*/\
\
-2	,	2	,	-2	,	1	,	0	,	1	,	-1	,	1	,	0,	/* RANGE I 4*/\
-2	,	2	,	-1	,	0	,	0	,	1	,	-1	,	1	,	0,	/* RANGE I 5*/\
-1	,	1	,	-1	,	0	,	0	,	1	,	-1	,	1	,	0,	/* RANGE I 6*/\
-1	,	1	,	-1	,	0	,	0	,	0	,	0	,	1	,	0,	/* RANGE I 7*/\
\
-1	,	1	,	-1	,	1	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 8*/\
-1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 9*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 10*/\
0	,	0	,	0	,	0	,	0	,	-1	,	1	,	0	,	0,	/* RANGE I 11*/\
\
-1	,	1	,	-1	,	1	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 12*/\
-1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 13*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 14*/\
0	,	0	,	0	,	0	,	0	,	-1	,	1	,	0	,	0,	/* RANGE I 15*/\
\
-1	,	1	,	-1	,	1	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 16*/\
-1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 17*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 18*/\
0	,	0	,	0	,	0	,	0	,	-1	,	1	,	0	,	0,	/* RANGE I 19*/\
\
-1	,	1	,	-1	,	1	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 20*/\
-1	,	1	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 21*/\
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0,	/* RANGE I 22*/\
0	,	0	,	0	,	0	,	0	,	-1	,	1	,	0	,	0	/* RANGE I 23*/\
}

//		Autoscale parameters
#define _FLOOR_D 12000 // if abs(value) < _FLOOR increase the senisibility
#define _CEIL_D 29789  // if abs(value) > _CEIL decrease the sensibility
#define _WAIT_INCREASING_I_AFTER_DECREASING_D	50
#define _WAIT_DECREASING_V_AFTER_INCREASING_D	50
#define _MAX_NUMBER_OVERRANGE_D 50
#define _MAX_NUMBER_UNDERRANGE_D 50

//		RANGE PARAMETERs FOR THE DELIVERED CURRENT 
#define _TABRANGEI_D {\
	/*NbAutoRange*/_NB_RANGEI_D,/*NbRange*/_NB_RANGEI_D,\
	/*DefaultRange=*/_INIT_RANGEI_D,	/*bitmasque*/0x07300730,\
	{{1.25e-8,	0x05000500}, {2.5e-8,	0x05100510},\
	{5e-8,		0x05200520}, {1e-7,	0x05300530},	\
	{1.25e-7,	0x04000400}, {2.5e-7,	0x04100410},	\
	{5e-7,		0x04200420}, {1e-6,	0x04300430},	\
	{1.25e-6,	0x03000300}, {2.5e-6,	0x03100310},	\
	{5e-6,		0x03200320}, {1e-5,	0x03300330},	\
	{1.25e-5,	0x02000200}, {2.5e-5,	0x02100210},	\
	{5e-5,		0x02200220}, {1e-4,	0x02300230},	\
	{1.25e-4,	0x01000100}, {2.5e-4,	0x01100110},	\
	{5e-4,		0x01200120}, {1e-3,	0x01300130},	\
	{1.25e-3,	0x00000000}, {2.5e-3,	0x00100010},	\
	{5e-3,		0x00200020}, {1e-2,	0x00300030}}}	

//		RANGE PARAMETERS FOR READING THE VOLTAGE
#define _TABRANGEV_D {\
	/*NbAutoRange*/_NB_RANGEV_D,/*NbRange*/_NB_RANGEV_D, \
	/*DefaultRange=*/_INIT_RANGEV_D,	/*bitmasque*/0x00c000c0,\
		{{1.25e-3,	0x00c000c0},\
		{2.5e-3,	0x00800080},\
		{5e-3,		0x00400040},\
		{1e-2,		0x00000000}}}
