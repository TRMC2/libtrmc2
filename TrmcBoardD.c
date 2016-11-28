#ifdef powerc
#include "manip.h"
#else
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <malloc.h>
#include <math.h> 
#endif

#include "Trmc.h" 
#include "TrmcRunLib.h"

#include "TrmcDef.h"
#include "TrmcProto.h"
#include "TrmcBoard.h"
#include "TrmcBoardD.h"

static int first_D = 1;
static VAR_ADEF *var_adef_D;
static short int CalibMatrixV[200] 
	= _CALIBMATRIXV_D;
static short int CalibMatrixI[_NB_RANGEI_D*_NB_MEASURE_CALIB_D] 
	= _CALIBMATRIXI_D;

static TABMEASURECAL TabMeasureCal = _TABMESCAL_D;

long int InitBoard_D(ABOARD *Board)
{
	TABRANGE TabRangeI = _TABRANGEI_D;
	TABRANGE TabRangeV = _TABRANGEV_D;
	short int mask[_NBMAX_SUBADD] = _MASK_SUBADD_D;

	if (first_D)
	{
		first_D = 0;
		
		var_adef_D = (VAR_ADEF *)malloc(sizeof(VAR_ADEF));
		if (var_adef_D == 0)
			return _CANNOT_ALLOCATE_MEM;

		var_adef_D->VERYFIRST = 1;
		var_adef_D->BOARDTYPE = 'D';
		var_adef_D->ODDDATA = _ODDDATA_D;
		var_adef_D->EVENDATA = _EVENDATA_D;
		var_adef_D->NB_RANGEI = TabRangeI.NbAutoRange;
		var_adef_D->NB_RANGEV = TabRangeV.NbAutoRange;
		var_adef_D->NB_MEASURE_CALIB = _NB_MEASURE_CALIB_D;
		var_adef_D->NB_SUBADD = _NB_SUBADD_D;
		var_adef_D->INIT_BOARD_MODE = _INIT_BOARD_MODE_D;
		var_adef_D->INIT_PREAVERAGING = _INIT_PREAVERAGING_D;
		var_adef_D->WAIT_BEFORE_CALIBRATION = _WAIT_BEFORE_CALIBRATION_D;
		var_adef_D->WAIT_BEFORE_FIRST_CALIBRATION = _WAIT_BEFORE_FIRST_CALIBRATION_D;
		var_adef_D->MAX_OFFSET = _MAX_OFFSET_D;
		var_adef_D->FLOOR = _FLOOR_D;
		var_adef_D->CEIL = _CEIL_D;
		var_adef_D->MAX_NUMBER_OVERRANGE = _MAX_NUMBER_OVERRANGE_D;
		var_adef_D->MAX_NUMBER_UNDERRANGE = _MAX_NUMBER_UNDERRANGE_D;
		var_adef_D->WAIT_DECREASING_V_AFTER_INCREASING = _WAIT_DECREASING_V_AFTER_INCREASING_D;
		var_adef_D->WAIT_INCREASING_I_AFTER_DECREASING = _WAIT_INCREASING_I_AFTER_DECREASING_D;
		var_adef_D->ODDDATA = _ODDDATA_D;
		var_adef_D->EVENDATA = _EVENDATA_D;
		var_adef_D->FULLSCALE = _FULLSCALE_D;
		var_adef_D->NB_MEASURE_OFFSET = _NB_MEASURE_OFFSET_D;
		var_adef_D->ACCEPT_CALIBRATION = _ACCEPT_CALIBRATION_D;
		var_adef_D->TabMeasCal = TabMeasureCal;
		var_adef_D->TabRangeI = TabRangeI;
		var_adef_D->TabRangeV = TabRangeV;
		var_adef_D->CalibMatrixI = CalibMatrixI;
		var_adef_D->CalibMatrixV = CalibMatrixV;

		var_adef_D->MASK_SUBADD[0] = mask[0];

	}

	Board->OddData = _ODDDATA_D;
	Board->EvenData = _EVENDATA_D;

	return InitBoard_ADEF(Board,var_adef_D);
}				// FIN InitBoard_D()
// ****************************************************************

long int CheckChannelBoard_D(ACHANNEL *Channel)
{
	return CheckChannelBoard_ADEF(Channel,var_adef_D);
}				// FIN CheckChannelBaord_D()
// ****************************************************************

long int CalcBoard_D(ABOARD *boardpt)
{
	return CalcBoard_ADEF(boardpt,var_adef_D);
}				// FIN CalcBoard_D()
// ************************************************************************

void OffsetCalulation_D(ABOARD *board,VAR_ADEF *var_adef_D,int location) 
// compute and set in board and in calibration table the offset 
// from the first calibartion measures
{
	double x,y;
	int xi,xv,yi,yv;

	(void) location;	// unused
	xi = TabMeasureCal.rg[0].NumRangeI;
	xv = TabMeasureCal.rg[0].NumRangeV;
	yi = TabMeasureCal.rg[1].NumRangeI;
	yv = TabMeasureCal.rg[1].NumRangeV;
	// first initialize the offset:
	// The 2 last measures x and y in CalibrationTable are for offset
 	x = board->parameter.CalibrationTable[0]*var_adef_D->TabMeasCal.rg[0].Value;
	y = board->parameter.CalibrationTable[1]*var_adef_D->TabMeasCal.rg[1].Value;
	// x has been measure with RangeV = 3 and RangeI = 0 

	// inversion of:
	// x = r *  Channel->parameter.ValueRangeV/ Channel->parameter.ValueRangeI;
	// x = ((double)a ) / ((double)boardpt->FullScale);
	x *= var_adef_D->TabRangeI.rg[xi].Value / var_adef_D->TabRangeV.rg[xv].Value;
	x *= board->FullScale;
	// y has been measure with RangeV = yv RangeI = yi 
	y *= var_adef_D->TabRangeI.rg[yi].Value / var_adef_D->TabRangeV.rg[yv].Value;
	y *= board->FullScale;
	
	//  Offset = closest integer from ((800*X1-X2)/799) 
	board->Offset = (int)((800.0*x-y)/799.0);
	// to make easier to track offset, add in the calibrationtable
	board->parameter.CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE-1]
		= (double) board->Offset;
}			// FIN OffsetCalulation_D()
// *********************************************************************
