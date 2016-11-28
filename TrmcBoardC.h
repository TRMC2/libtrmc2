// *****************************************************************
//                        BOARD TYPE C : DIOD
// *****************************************************************
//      General parameters
#define _NB_SUBADD_C				1
#define _EVENDATA_C					0x00000000
#define _ODDDATA_C					0x00100010
#define _DEFAULT_MODE_C				_FIX_RANGE_MODE
#define _FLOOR_C					3000
#define _CEIL_C						30000
#define _NB_SUBADD_C				1

//		Calibration parameters
#define _NB_COEF_CALIBRATION_C		2
#define _REF_CALIBRATION0_C			0.5   // 1/2 volt
#define _REF_CALIBRATION1_C			2   // 2 volt
#define _CALIBRATION_TIME_C			(25*5) // 40*25*2 musecond = 2 seconds
#define _ACCEPT_CALIBRATION_C		0.05    // |1-coeff|<_ACCEPT_CALIBRATION_C
#define _FULLSCALE_C				58982	// 
#define _RANGEV_C					1
/*
le bit de lecture est A4
A4=1: lecture de la ref
a4=0 lecture de mesure
*/

//		Default values
#define  _NUMBER_RANGEI_C			1     // just to inform user NOT used in any calculation
#define  _NUMBER_RANGEV_C			1     // just to inform user NOT used in any calculation
#define  _VALUERANGE_I_C			10e-6 // just to inform user NOT used in any calculation
#define  _VALUERANGE_V_C			2	  // just to inform user NOT used in any calculation
#define _INIT_PREAVERAGING_C		25	// a point/secund
#define _INIT_BOARD_MODE_C			_FIX_RANGE_MODE
#define _INIT_RANGEI_C				0
#define _INIT_RANGEV_C				0

//			Private memory
typedef struct
{
	int Hit;				
	double Accumulator;       // to re-copy from odd time to even time
	double CoefCst;			// for calibration, constant terme for linear interpolatiom
	double Slope;			// for calibration, slope for linear interpolatiom
}  PRIVATEMEMORY_C;	

