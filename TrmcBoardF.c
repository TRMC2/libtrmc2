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
#include "TrmcBoardF.h"

static int first_F = 1;
static VAR_ADEF *var_adef_F;
static short int CalibMatrixV[_NB_RANGEV_F*_NB_MEASURE_CALIB_F] 
	= _CALIBMATRIXV_F;
static short int CalibMatrixI[(_NB_RANGEI_F+1)*_NB_MEASURE_CALIB_F] 
	= _CALIBMATRIXI_F;

long int InitBoard_F(ABOARD *Board)
{
	TABMEASURECAL TabMeasureCal_F = _TABMESCAL_F;
	TABRANGE TabRangeI = _TABRANGEI_F;
	TABRANGE TabRangeV = _TABRANGEV_F;
	int mask[_NBMAX_SUBADD] = _MASK_SUBADD_F;

	if (first_F)
	{
		first_F = 0;
		
		var_adef_F = (VAR_ADEF *)malloc(sizeof(VAR_ADEF));
		if (var_adef_F == 0)
			return _CANNOT_ALLOCATE_MEM;

		var_adef_F->VERYFIRST = 2;
		var_adef_F->BOARDTYPE = 'F';
		var_adef_F->ODDDATA = _ODDDATA_F;
		var_adef_F->EVENDATA = _EVENDATA_F;
		var_adef_F->NB_RANGEI = TabRangeI.NbAutoRange;
		var_adef_F->NB_RANGEV = TabRangeV.NbAutoRange;
		var_adef_F->NB_MEASURE_CALIB = _NB_MEASURE_CALIB_F;
		var_adef_F->NB_SUBADD = _NB_SUBADD_F;
		var_adef_F->INIT_BOARD_MODE = _INIT_BOARD_MODE_F;
		var_adef_F->INIT_PREAVERAGING = _INIT_PREAVERAGING_F;
		var_adef_F->WAIT_BEFORE_CALIBRATION = _WAIT_BEFORE_CALIBRATION_F;
		var_adef_F->WAIT_BEFORE_FIRST_CALIBRATION = _WAIT_BEFORE_FIRST_CALIBRATION_F;
		var_adef_F->MAX_OFFSET = _MAX_OFFSET_F;
		var_adef_F->FLOOR = _FLOOR_F;
		var_adef_F->CEIL = _CEIL_F;
		var_adef_F->MAX_NUMBER_OVERRANGE = _MAX_NUMBER_OVERRANGE_F;
		var_adef_F->MAX_NUMBER_UNDERRANGE = _MAX_NUMBER_UNDERRANGE_F;
		var_adef_F->WAIT_DECREASING_V_AFTER_INCREASING = _WAIT_DECREASING_V_AFTER_INCREASING_F;
		var_adef_F->WAIT_INCREASING_I_AFTER_DECREASING = _WAIT_INCREASING_I_AFTER_DECREASING_F;
		var_adef_F->ODDDATA = _ODDDATA_F;
		var_adef_F->EVENDATA = _EVENDATA_F;
		var_adef_F->FULLSCALE = _FULLSCALE_F;
		var_adef_F->NB_MEASURE_OFFSET = _NB_MEASURE_OFFSET_F;
		var_adef_F->ACCEPT_CALIBRATION = _ACCEPT_CALIBRATION_F;
		var_adef_F->TabMeasCal = TabMeasureCal_F;
		var_adef_F->TabRangeI = TabRangeI;
		var_adef_F->TabRangeV = TabRangeV;
		var_adef_F->CalibMatrixI = CalibMatrixI;
		var_adef_F->CalibMatrixV = CalibMatrixV;

		var_adef_F->MASK_SUBADD[0] = mask[0];
		var_adef_F->MASK_SUBADD[1] = mask[1];
		var_adef_F->MASK_SUBADD[2] = mask[2];
		var_adef_F->MASK_SUBADD[3] = mask[3];
		}

	Board->OddData = _ODDDATA_F;
	Board->EvenData = _EVENDATA_F;

	return InitBoard_ADEF(Board,var_adef_F);
}				// FIN InitBoard_F()
// ****************************************************************

long int CheckChannelBoard_F(ACHANNEL *Channel)
{
	return CheckChannelBoard_ADEF(Channel,var_adef_F);
}				// FIN CheckChannelBaord_F()
// ****************************************************************

long int CalcBoard_F(ABOARD *boardpt)
{
	return CalcBoard_ADEF(boardpt,var_adef_F);
}				// FIN CalcBoard_F()
// ************************************************************************

void OffsetCalulation_F(ABOARD *Board,VAR_ADEF *var_adef_F,int location) 
// compute and set in board and in calibration table the offset 
// from the first calibartion measures
{
	double x;
	int kI,kV,kc;

	(void) location; // unused
	kI = var_adef_F->TabMeasCal.rg[0].NumRangeI;
	kV = var_adef_F->TabMeasCal.rg[0].NumRangeV;
	kc =  var_adef_F->TabMeasCal.rg[0].Channel;
	// x has been measure with RangeV = kV and RangeI = kI on channel kc

	x = Board->parameter.CalibrationTable[0];
	x *= var_adef_F->TabMeasCal.rg[kc].Value;
	x *= var_adef_F->TabRangeI.rg[kI].Value/var_adef_F->TabRangeV.rg[kV].Value;
	x *= Board->FullScale;

	Board->Offset = (int)(x - _CORRECTION_TO_ZERO_RESISTOR) ; 

	// to make easier to track offset, add in the calibrationtable
	Board->parameter.CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE-1]	= Board->Offset;
}				// FIN OffsetCalculation_F()
// ************************************************************************

