// *****************************************************************
//                        BOARD TYPE B : CAPACITOR
// *****************************************************************
//      General parameters
#define _NB_SUBADD_B				1
#define _ODDDATA_B					0x01000100
#define _EVENDATA_B					0x00000000
#define _FULLSCALE_REF_B			28985.0
#define _FULLSCALE_B				27846	
#define _IMPEDANCE_CONVERSION		159.12e-6
#define	_NB_RANGEV_B				4						// Voltage Range Coef
#define	_NB_RANGEI_B				2						// Intensity Range Coef

//		Initialisation parameters
#define _INIT_PREAVERAGING_B		25 // a point/secund
#define _INIT_BOARD_MODE_B			_FIX_RANGE_MODE
#define _INIT_RANGEI_B				0
#define _INIT_RANGEV_B				0

//		Calibration parameters
// Only one measure for calibration
#define _NB_COEF_CALIBRATION_B		1
#define _REF_CALIBRATION_B			26e-9   // 26 nanofarad
#define _RANGEI_CALIBRATION_B		2.40e-7 // 250 nanoA
#define _RANGEV_CALIBRATION_B		7.64e-3    //  7.64 millivolts
#define _CALIBRATION_TIME_B			125	// 10 seconds
#define _WAIT_BEFORE_CALIBRATION_B	25		// in tics unit
#define _ACCEPT_CALIBRATION_B		0.1		// |mesure/_REF_CAL - 1|<_ACCEPT_CALIBRATION_B


//		Autoscale parameters
#define _FLOOR_B 2240 // if abs(value) < _FLOOR increase the senisibility
#define _CEIL_B  28000 // if abs(value) > _CEIL decrease the sensibility
#define _WAIT_DECREASING_V_AFTER_INCREASING_B 25 // a point every TWO tics
#define _WAIT_INCREASING_I_AFTER_DECREASING_B 25 // a point every TWO tics
#define _MAX_NUMBER_OVERRANGE_B 10
#define _MAX_NUMBER_UNDERRANGE_B 10

//		RANGE PARAMETERS FOR READING THE VOLTAGE
#define _TABRANGEV_B {\
	/*NbAutoRange*/_NB_RANGEV_B,/*NbRange*/_NB_RANGEV_B, \
	/*DefaultRange=*/_INIT_RANGEV_B,	/*bitmasque*/0x00300030,\
		{{7.641e-6,	0x00300030},	\
		{7.641e-5,	0x00200020},\
		{7.641e-4,	0x00100010},\
		{7.641e-3,	0x00000000 }}};

//		RANGE PARAMETERs FOR THE DELIVERED CURRENT 
#define _TABRANGEI_B {\
	/*NbAutoRange*/_NB_RANGEI_B,/*NbRange*/_NB_RANGEI_B,\
	/*DefaultRange=*/_INIT_RANGEI_B,	/*bitmasque*/0x00c000c0,\
		{{2.4e-8,	0x00800080},		\
		{2.4e-7,	0x00400040}}};	

typedef struct
{
	int NumberOverRange;	// number of consecutive overrange
	int NumberUnderRange;   // number of consecutive underrange
	int LastIncrV;			// last call where an increase was done
	int LastDecrI;			// last call where a decrease was done
	int CalibrationCpt;		// Counter for calibratiom
	int CalibrationSubCpt;	// Counter for calibratiom
	int CalibrationIndex;	// Index for calibratiom
	int CalibrationNextEvent;	// time of the next begin or end of a calibration measure
	double Accumulator;		// Accumulator for calibration
	int Hit;				// Accumulator for calibration
	int MeasDone;	// ususefull for scanninf subaddress
	int MeasToDo;	// ususefull for scanninf subaddress
	int CurrentChannel;		// channel scrutated
	double CorrectionI[_NB_RANGEI_MAX];	// Coefficients I themselves
	double CorrectionV[_NB_RANGEV_MAX];	// Coefficients V themselves
	int veryfirst;
	int LastData;
}  PRIVATEMEMORY_B;	

