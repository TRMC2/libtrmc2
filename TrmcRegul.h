// SPDX-License-Identifier: LGPL-3.0-or-later
// --------------------   BOARD TYPE REGUL    ---------------------
#define _NB_SUBADD_REGUL 1

#define _INIT_PREAVERAGING_REGUL 25*2 // every two second
#define _INIT_VALUERANGE_REGUL	0
#define _INIT_MODE_REGUL		_FIX_RANGE_MODE
#define _INIT_RANGEI_REGUL		4

#define _ALPHA_MAX_REGUL 0.999 // maximum acceptable value for alpha and beta

#define _DERIVATIVE_INTENSITY 7 // Enter in the formula for calculating the two filters

//		Calibration parameters
#define _WAIT_BEFORE_CALIB_REGUL 12 // in Tics unit
#define _START_VALUE_CALIB_MAX 1024  // SHOULD BE A POWER OF TWO
#define _START_VALUE_CALIB_MIN -_START_VALUE_CALIB_MAX  // SHOULD BE A POWER OF TWO
#define  _NB_MEASURE_CALIB_REGUL 1

#define _WATCHDOG_REGUL_VALUE (25*30) // in tics unit

#define _NB_RANGEI_REGUL 6
#define  _FLOOR_REGUL 2500
#define  _CEIL_REGUL 30000


#define  _BOOSTER_RANGE 4
#define _VMAX_RETURN0_WITHOUT_BOOSTER 13
#define _VMAX_RETURN15_WITHOUT_BOOSTER 28
#define _VMAX_WITH_BOOSTER 50


// IL FAUT METTRE LES VALEURS DS LE POIDS FORT
#define _TABRANGEI_REGUL {\
	/*NbAutoRange*/_NB_RANGEI_REGUL-1,/*NbRange*/_NB_RANGEI_REGUL,\
	/*DefaultRange=*/_INIT_RANGEI_REGUL,	/*bitmasque*/0x00070000,\
	{{5e-4,0x00010000},\
	{5e-3,0x00020000},\
	{5e-2,0x00030000},\
	{5e-1,0x00040000},\
	{3,   0x00050000},\
	{0,   0x00000000}}  /* never used in auto*/\
};

double Heat(REGULPARAMETER *Regulation,double dx,double *acc);
void Calibrate_Regul(ABOARD *Board);
int Check_Regul(AREGUL *Regul);
void 	MakeReguls(void);
void 	SynchroRegulPointeur(void);
// double 	Watt2Ampere(double x,AREGUL *Regul); // Linux does not like static in .h file 

typedef struct
{
	int DataToSend;	// For calibration
	int xa;			// For calibration
	int ya;			// For calibration
	int xi;			// For calibration
	int yi;			// For calibration
	int x;			// For calibration
	int y;			// For calibration
	int Cpt;		// For calibration
	
	double Accumulator;
	double LastPdx;

	double Filter1Dx; // usefull for intermediate calculation in the filtered dx
	double Filter2Dx; // usefull for intermediate calculation in the filtered dx

	double alpha;	// for first filter
	double beta;	// for second filter

	int WatchDog;
	REGULPARAMETER RegulSaved;
	CHANNELPARAMETER ChannelSaved;
	double TabVMax[_NB_RANGEI_REGUL];
}  PRIVATEMEMORY_REGUL;	

#define	_MAXBOARDCHANNEL_REGUL	1

#define	_MAX_VOLTAGE_DACB 10.0
#define	_MAX_CURRENT_DACB 5e-3
// SOME PROTOTYPE SOME PROTOTYPE SOME PROTOTYPE SOME PROTOTYPE
// int Check_Regul_Interne(AREGUL *Regul);// Linux does not like static in a .h
