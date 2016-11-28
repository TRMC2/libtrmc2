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
#include "TrmcBoardA.h"

static int first_A = 1;
static VAR_ADEF *var_adef_A;
static short int CalibMatrixI_A[_NB_RANGEI_A*_NB_MEASURE_CALIB_A] 
	= _CALIBMATRIXI_A;
static short int CalibMatrixV_A[_NB_RANGEV_A*_NB_MEASURE_CALIB_A] 
	= _CALIBMATRIXV_A;
 
long int InitBoard_A(ABOARD *Board)
{
	TABMEASURECAL TabMeasureCal_A = _TABMESCAL_A;
	TABRANGE TabRangeI_A = _TABRANGEI_A;
	TABRANGE TabRangeV_A = _TABRANGEV_A;

	if (first_A)
	{
		first_A = 0;
		
		var_adef_A = (VAR_ADEF *)malloc(sizeof(VAR_ADEF));
		if (var_adef_A == 0)
			return _CANNOT_ALLOCATE_MEM;

		var_adef_A->VERYFIRST = 1;
		var_adef_A->BOARDTYPE = 'A';
		var_adef_A->ODDDATA = _ODDDATA_A;
		var_adef_A->EVENDATA = _EVENDATA_A;
		var_adef_A->NB_RANGEI = TabRangeI_A.NbAutoRange;
		var_adef_A->NB_RANGEV = TabRangeV_A.NbAutoRange;
		var_adef_A->NB_MEASURE_CALIB = _NB_MEASURE_CALIB_A;
		var_adef_A->NB_SUBADD = _NB_SUBADD_A;
		var_adef_A->INIT_BOARD_MODE = _INIT_BOARD_MODE_A;
		var_adef_A->INIT_PREAVERAGING = _INIT_PREAVERAGING_A;
		var_adef_A->WAIT_BEFORE_CALIBRATION = _WAIT_BEFORE_CALIBRATION_A;
		var_adef_A->WAIT_BEFORE_FIRST_CALIBRATION = _WAIT_BEFORE_FIRST_CALIBRATION_A;
		var_adef_A->MAX_OFFSET = _MAX_OFFSET_A;
		var_adef_A->FLOOR = _FLOOR_A;
		var_adef_A->CEIL = _CEIL_A;
		var_adef_A->MAX_NUMBER_OVERRANGE = _MAX_NUMBER_OVERRANGE_A;
		var_adef_A->MAX_NUMBER_UNDERRANGE = _MAX_NUMBER_UNDERRANGE_A;
		var_adef_A->WAIT_DECREASING_V_AFTER_INCREASING = _WAIT_DECREASING_V_AFTER_INCREASING_A;
		var_adef_A->WAIT_INCREASING_I_AFTER_DECREASING = _WAIT_INCREASING_I_AFTER_DECREASING_A;
		var_adef_A->ODDDATA = _ODDDATA_A;
		var_adef_A->EVENDATA = _EVENDATA_A;
		var_adef_A->FULLSCALE = _FULLSCALE_A;
		var_adef_A->NB_MEASURE_OFFSET = _NB_MEASURE_OFFSET_A;
		var_adef_A->ACCEPT_CALIBRATION = _ACCEPT_CALIBRATION_A;
		var_adef_A->TabMeasCal = TabMeasureCal_A;
		var_adef_A->TabRangeI = TabRangeI_A;
		var_adef_A->TabRangeV = TabRangeV_A;
		var_adef_A->CalibMatrixI = CalibMatrixI_A;
		var_adef_A->CalibMatrixV = CalibMatrixV_A;

		var_adef_A->MASK_SUBADD[0] = 0;

	}
	
	Board->OddData = _ODDDATA_A;
	Board->EvenData = _EVENDATA_A;

	return InitBoard_ADEF(Board,var_adef_A);
}				// FIN InitBoard_A()
// ****************************************************************

long int CheckChannelBoard_A(ACHANNEL *Channel)
{	
		return CheckChannelBoard_ADEF(Channel,var_adef_A);
}				// FIN CheckChannelBaord_A()
// ****************************************************************

long int CalcBoard_A(ABOARD *boardpt)
{
	return CalcBoard_ADEF(boardpt,var_adef_A);
}				// FIN CalcBoard_A()
// ************************************************************************


