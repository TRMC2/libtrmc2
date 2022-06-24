// SPDX-License-Identifier: LGPL-3.0-or-later
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
#include "TrmcBoardE.h"

static int first_E = 1;
static VAR_ADEF *var_adef_E;
static short int CalibMatrixV[_NB_RANGEV_E*_NB_MEASURE_CALIB_E] 
	= _CALIBMATRIXV_E;
static short int CalibMatrixI[(_NB_RANGEI_E+1)*_NB_MEASURE_CALIB_E] 
	= _CALIBMATRIXI_E;

long int InitBoard_E(ABOARD *Board)
{
	TABMEASURECAL TabMeasureCal_E = _TABMESCAL_E;
	TABRANGE TabRangeI = _TABRANGEI_E;
	TABRANGE TabRangeV = _TABRANGEV_E;
	int mask[_NBMAX_SUBADD] = _MASK_SUBADD_E,st;

	if (first_E)
	{
		first_E = 0;
		
		var_adef_E = (VAR_ADEF *)malloc(sizeof(VAR_ADEF));
		if (var_adef_E == 0)
			return _CANNOT_ALLOCATE_MEM;

		var_adef_E->VERYFIRST = 2;
		var_adef_E->BOARDTYPE = 'E';
		var_adef_E->ODDDATA = _ODDDATA_E;
		var_adef_E->EVENDATA = _EVENDATA_E;
		var_adef_E->NB_RANGEI = TabRangeI.NbAutoRange;
		var_adef_E->NB_RANGEV = TabRangeV.NbAutoRange;
		var_adef_E->NB_MEASURE_CALIB = _NB_MEASURE_CALIB_E;
		var_adef_E->NB_SUBADD = _NB_SUBADD_E;
		var_adef_E->INIT_BOARD_MODE = _INIT_BOARD_MODE_E;
		var_adef_E->INIT_PREAVERAGING = _INIT_PREAVERAGING_E;
		var_adef_E->WAIT_BEFORE_CALIBRATION = _WAIT_BEFORE_CALIBRATION_E;
		var_adef_E->WAIT_BEFORE_FIRST_CALIBRATION = _WAIT_BEFORE_FIRST_CALIBRATION_E;
		var_adef_E->MAX_OFFSET = _MAX_OFFSET_E;
		var_adef_E->FLOOR = _FLOOR_E;
		var_adef_E->CEIL = _CEIL_E;
		var_adef_E->MAX_NUMBER_OVERRANGE = _MAX_NUMBER_OVERRANGE_E;
		var_adef_E->MAX_NUMBER_UNDERRANGE = _MAX_NUMBER_UNDERRANGE_E;
		var_adef_E->WAIT_DECREASING_V_AFTER_INCREASING = _WAIT_DECREASING_V_AFTER_INCREASING_E;
		var_adef_E->WAIT_INCREASING_I_AFTER_DECREASING = _WAIT_INCREASING_I_AFTER_DECREASING_E;
		var_adef_E->ODDDATA = _ODDDATA_E;
		var_adef_E->EVENDATA = _EVENDATA_E;
		var_adef_E->FULLSCALE = _FULLSCALE_E;
		var_adef_E->NB_MEASURE_OFFSET = _NB_MEASURE_OFFSET_E;
		var_adef_E->ACCEPT_CALIBRATION = _ACCEPT_CALIBRATION_E;
		var_adef_E->TabMeasCal = TabMeasureCal_E;
		var_adef_E->TabRangeI = TabRangeI;
		var_adef_E->TabRangeV = TabRangeV;
		var_adef_E->CalibMatrixI = CalibMatrixI;
		var_adef_E->CalibMatrixV = CalibMatrixV;

		var_adef_E->MASK_SUBADD[0] = mask[0];
		var_adef_E->MASK_SUBADD[1] = mask[1];
		var_adef_E->MASK_SUBADD[2] = mask[2];
		var_adef_E->MASK_SUBADD[3] = mask[3];


	}

	st = InitBoard_ADEF(Board,var_adef_E);

	Board->OddData = _ODDDATA_E;
	Board->EvenData = _EVENDATA_E;

	return st;
}				// FIN InitBoard_E()
// ****************************************************************

long int CheckChannelBoard_E(ACHANNEL *Channel)
{
	return CheckChannelBoard_ADEF(Channel,var_adef_E);
}				// FIN CheckChannelBaord_E()
// ****************************************************************

long int CalcBoard_E(ABOARD *boardpt)
{
	return CalcBoard_ADEF(boardpt,var_adef_E);
}				// FIN CalcBoard_E()
// ************************************************************************

void OffsetCalulation_E(ABOARD *Board,VAR_ADEF *var_adef_E,int location) 
// compute and set in board and in calibration table the offset 
// from the first calibartion measures
{
	double x;
	int kI,kV,kc;

	(void) location; // unused
	kI = var_adef_E->TabMeasCal.rg[0].NumRangeI;
	kV = var_adef_E->TabMeasCal.rg[0].NumRangeV;
	kc =  var_adef_E->TabMeasCal.rg[0].Channel;
	// x has been measure with RangeV = kV and RangeI = kI on channel kc

	x = Board->parameter.CalibrationTable[0];
	x *= var_adef_E->TabMeasCal.rg[kc].Value;
	x *= var_adef_E->TabRangeI.rg[kI].Value/var_adef_E->TabRangeV.rg[kV].Value;
	x *= Board->FullScale;

	Board->Offset = (int)(x); 

	// to make easier to track offset, add in the calibrationtable
	Board->parameter.CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE-1]	= Board->Offset;
}				// FIN OffsetCalculation_E()
// ************************************************************************

