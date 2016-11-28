//			TrmcBoard.h

int SetRangeAndData (TABRANGE *Range,
					 short int flag,
					 short int *NumRange,
					 double *ValueRange,
					 int *DataRange);

double raisepower(double x,int k);

//-------------------------------------------------------- 
//    PRIVATE DEFINITIONS AND STRUCTURES FOR EACH BOARD    
//-------------------------------------------------------- 
#define _NB_RANGEV_MAX 10
#define _NB_RANGEI_MAX 32

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
}  PRIVATEMEMORY_ADEF;	

typedef struct
{
	char BOARDTYPE;
	int NB_RANGEI;
	int NB_RANGEV;
	int ODDDATA;
	int EVENDATA;
	int NB_MEASURE_CALIB;
	int NB_MEASURE_OFFSET;
	int NB_SUBADD;
	int INIT_BOARD_MODE;
	int INIT_PREAVERAGING;
	int WAIT_BEFORE_CALIBRATION;
	int WAIT_BEFORE_FIRST_CALIBRATION;
	int MAX_OFFSET;
	int FLOOR;
	int CEIL;
	int MAX_NUMBER_OVERRANGE;
	int MAX_NUMBER_UNDERRANGE;
	int WAIT_DECREASING_V_AFTER_INCREASING;
	int WAIT_INCREASING_I_AFTER_DECREASING;
	int FULLSCALE;
	double ACCEPT_CALIBRATION;
	TABMEASURECAL TabMeasCal;
	TABRANGE TabRangeI;
	TABRANGE TabRangeV;
	short int *CalibMatrixI;
	short int *CalibMatrixV;
	int MASK_SUBADD[_NBMAX_SUBADD];
	short VERYFIRST;
} VAR_ADEF;

#define _PRIVATEMEMORY PRIVATEMEMORY_ADEF
// +1 since an extra measure for testing quality

long int CheckChannelBoard_ADEF(ACHANNEL *Channel,VAR_ADEF *var_adef);
long int InitBoard_ADEF(ABOARD *board,VAR_ADEF *var_adef);
long int CalcBoard_ADEF(ABOARD *boardpt,VAR_ADEF *var_adef);

void OffsetCalulation_D(ABOARD *board,VAR_ADEF *var_adef_D,int location);
void OffsetCalulation_E(ABOARD *board,VAR_ADEF *var_adef_E,int location); 
void OffsetCalulation_F(ABOARD *board,VAR_ADEF *var_adef_F,int location);
void OffsetCalulation_G(ABOARD *board,VAR_ADEF *var_adef_G,int location); 
int Calibrate_ADEF(ABOARD *Board,VAR_ADEF *var_adef);

