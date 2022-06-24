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
#include "TrmcBoardG.h"

static int first_G = 1;
static VAR_ADEF *var_adef_G;
static short int CalibMatrixV[_NB_RANGEV_G*_NB_MEASURE_CALIB_G] = _CALIBMATRIXV_G;
static short int CalibMatrixI[(_NB_RANGEI_G+1)*_NB_MEASURE_CALIB_G] = _CALIBMATRIXI_G;

static TABMEASURECAL TabMeasureCal = _TABMESCAL_G;

long int InitBoard_G(ABOARD *Board)
{
	TABRANGE TabRangeI = _TABRANGEI_G;
	TABRANGE TabRangeV = _TABRANGEV_G;
	short int mask[_NBMAX_SUBADD] = _MASK_SUBADD_G;

	if (first_G)
	{
		first_G = 0;
		
		var_adef_G = (VAR_ADEF *)malloc(sizeof(VAR_ADEF));
		if (var_adef_G == 0)
			return _CANNOT_ALLOCATE_MEM;

		var_adef_G->VERYFIRST = 1;
		var_adef_G->BOARDTYPE = 'G';
		var_adef_G->ODDDATA = _ODDDATA_G;
		var_adef_G->EVENDATA = _EVENDATA_G;
		var_adef_G->NB_RANGEI = TabRangeI.NbAutoRange;
		var_adef_G->NB_RANGEV = TabRangeV.NbAutoRange;
		var_adef_G->NB_MEASURE_CALIB = _NB_MEASURE_CALIB_G;
		var_adef_G->NB_SUBADD = _NB_SUBADD_G;
		var_adef_G->INIT_BOARD_MODE = _INIT_BOARD_MODE_G;
		var_adef_G->INIT_PREAVERAGING = _INIT_PREAVERAGING_G;
		var_adef_G->WAIT_BEFORE_CALIBRATION = _WAIT_BEFORE_CALIBRATION_G;
		var_adef_G->WAIT_BEFORE_FIRST_CALIBRATION = _WAIT_BEFORE_FIRST_CALIBRATION_G;
		var_adef_G->MAX_OFFSET = _MAX_OFFSET_G;
		var_adef_G->FLOOR = _FLOOR_G;
		var_adef_G->CEIL = _CEIL_G;
		var_adef_G->MAX_NUMBER_OVERRANGE = _MAX_NUMBER_OVERRANGE_G;
		var_adef_G->MAX_NUMBER_UNDERRANGE = _MAX_NUMBER_UNDERRANGE_G;
		var_adef_G->WAIT_DECREASING_V_AFTER_INCREASING = _WAIT_DECREASING_V_AFTER_INCREASING_G;
		var_adef_G->WAIT_INCREASING_I_AFTER_DECREASING = _WAIT_INCREASING_I_AFTER_DECREASING_G;
		var_adef_G->ODDDATA = _ODDDATA_G;
		var_adef_G->EVENDATA = _EVENDATA_G;
		var_adef_G->FULLSCALE = _FULLSCALE_G;
		var_adef_G->NB_MEASURE_OFFSET = _NB_MEASURE_OFFSET_G;
		var_adef_G->ACCEPT_CALIBRATION = _ACCEPT_CALIBRATION_G;
		var_adef_G->TabMeasCal = TabMeasureCal;
		var_adef_G->TabRangeI = TabRangeI;
		var_adef_G->TabRangeV = TabRangeV;
		var_adef_G->CalibMatrixI = CalibMatrixI;
		var_adef_G->CalibMatrixV = CalibMatrixV;

		var_adef_G->MASK_SUBADD[0] = mask[0];
		var_adef_G->MASK_SUBADD[1] = mask[1];
		var_adef_G->MASK_SUBADD[2] = mask[2];
		var_adef_G->MASK_SUBADD[3] = mask[3];


	}

	Board->OddData = _ODDDATA_G;
	Board->EvenData = _EVENDATA_G;

	return InitBoard_ADEF(Board,var_adef_G);
}				// FIN InitBoard_G()
// ****************************************************************

long int CheckChannelBoard_G(ACHANNEL *Channel)
{
	return CheckChannelBoard_ADEF(Channel,var_adef_G);
}				// FIN CheckChannelBaord_G()
// ****************************************************************

long int CalcBoard_G(ABOARD *boardpt)
{
	return CalcBoard_ADEF(boardpt,var_adef_G);
}				// FIN CalcBoard_G()
// ************************************************************************

void OffsetCalulation_G(ABOARD *Board,VAR_ADEF *var_adef_G,int location) 
// compute and set in board and in calibration table the offset 
// from the first calibartion measures
{
	double x;
	int kI,kV,kc;

	(void) location; // unused
	kI = var_adef_G->TabMeasCal.rg[0].NumRangeI;
	kV = var_adef_G->TabMeasCal.rg[0].NumRangeV;
	kc =  var_adef_G->TabMeasCal.rg[0].Channel;
	// x has been measure with RangeV = kV and RangeI = kI on channel kc

	x = Board->parameter.CalibrationTable[0];
	x *= var_adef_G->TabMeasCal.rg[kc].Value;
	x *= var_adef_G->TabRangeI.rg[kI].Value/var_adef_G->TabRangeV.rg[kV].Value;
	x *= Board->FullScale;

	Board->Offset = (int)(x-0.655); 
}			// FIN OffsetCalulation_G()
// *********************************************************************
