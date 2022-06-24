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
#include "TrmcBoardB.h"

static TABRANGE TabRangeV_B = _TABRANGEV_B;
static TABRANGE TabRangeI_B = _TABRANGEI_B;

long int InitBoard_B(ABOARD *Board)
// Two purposes:
// 1/	Initialize the Board the first time with default values.
// 2/	Derive the correction coefficients from the calibration measures.
{
	PRIVATEMEMORY_B *pt = 0;
	int i;

	if (Board->PrivateMemory==0)		// First call = default values
	{
		// This a REAL initialization and NOT to translate the calibration
		// measures into correction coefficient

		// Allocate private memory
		pt = (PRIVATEMEMORY_B *)malloc(sizeof(PRIVATEMEMORY_B));
		if (pt==0)
			return _CANNOT_ALLOCATE_MEM;
		Board->PrivateMemory = pt;

		// set the private memory parameters
		pt->veryfirst = 1;
		pt->NumberOverRange=0;	// number of consecutive overrange
		pt->NumberUnderRange=0;  // number of consecutive underrange
		pt->LastIncrV=_THEEPOCH; // last call where an increase was done
		pt->LastDecrI=_THEEPOCH;	// last call where a decrease was done
		pt->CalibrationCpt=-1;	// Counter for calibratiom
		pt->Accumulator = 0;	// Accumulator for calibration
		pt->MeasDone = 0;	// to be equal to MeasToDo
		pt->MeasToDo = 0;
		pt->Hit = 0;
		pt->Accumulator = 0;
		pt->CalibrationSubCpt = 0;
		pt->CorrectionI[0] = 1.0;	// Nominal value

		// set number of calibration (2 for offset + 9 for calibration)
		Board->parameter.NumberofCalibrationMeasure = _NB_COEF_CALIBRATION_B;
		// the 9 first are for calibration and the two last for offset

		// set fullscale
		Board->FullScale = _FULLSCALE_B;
		Board->NumberofChannels = _NB_SUBADD_B; // Number of channel for this Board

		// Give the calibration measure nominal values
		for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++) 
			Board->parameter.CalibrationTable[i] = -2.0;
		for (i=0;i<Board->parameter.NumberofCalibrationMeasure;i++) 
			Board->parameter.CalibrationTable[i] = _REF_CALIBRATION_B;

		// Set the channel treated to zero since ChannelAlways present
		Board->ChannelTreated = 0;

		Board->Data = 0;
	}
	pt = Board->PrivateMemory ;
	

	Board->parameter.NumberofIRanges = TabRangeI_B.NbAutoRange;
	for(i=0;i<TabRangeI_B.NbRange;i++)
		Board->parameter.IRangesTable[i] = TabRangeI_B.rg[i].Value;

	Board->parameter.NumberofVRanges = TabRangeV_B.NbAutoRange;
	for(i=0;i<TabRangeV_B.NbRange;i++)
		Board->parameter.VRangesTable[i] = TabRangeV_B.rg[i].Value;

	Board->OddData = _ODDDATA_B;
	Board->EvenData = _EVENDATA_B;

	return _RETURN_OK;
}				// FIN InitBoard_B()
// ****************************************************************


long int CheckChannelBoard_B(ACHANNEL *Channel)
{
	int	k,somethingchanged = 0;

	//				Check sub_address
	if (Channel->parameter.SubAddress >= _NB_SUBADD_B)
		return _INVALID_SUBADDRESS;

	//				Check mode
	// all modes in case are ok, default is not;
	switch (Channel->parameter.Mode)
	{
	case _INIT_MODE:
		Channel->NumRangeI = TabRangeI_B.DefaultRange;
		Channel->NumRangeV = TabRangeV_B.DefaultRange;
		Channel->parameter.Mode = _INIT_BOARD_MODE_B;
		Channel->parameter.PreAveraging = _INIT_PREAVERAGING_B;
		Channel->parameter.ValueRangeI = 
			TabRangeI_B.rg[TabRangeI_B.DefaultRange].Value;
		Channel->parameter.ValueRangeV = 
			TabRangeV_B.rg[TabRangeV_B.DefaultRange].Value;
	case _NOT_USED_MODE:
	case _FIX_RANGE_MODE:
	case _FIX_VOLTAGE_MODE:
	case _FIX_CURRENT_MODE:
	case _PRIORITY_VOLTAGE_MODE:
	case _PRIORITY_CURRENT_MODE:
		break;
	default:
		return _INVALID_MODE; 
	}

	k = SetRangeAndData(&TabRangeV_B,_RANGEBYVALUE,
		&Channel->NumRangeV,&Channel->parameter.ValueRangeV,0);
	if (k<0) 
		return _RANGE_CHANGE_NOT_POSSIBLE;
	if (k>0)
		somethingchanged++;

	k = SetRangeAndData(&TabRangeI_B,_RANGEBYVALUE,
		&Channel->NumRangeI,&Channel->parameter.ValueRangeI,0);
	if (k<0) 
		return _RANGE_CHANGE_NOT_POSSIBLE;
	if (k>0)
		somethingchanged++;

	if (somethingchanged)
		return _CHANNEL_HAS_BEEN_MODIFIED;
	else
		return _RETURN_OK;
}				// FIN CheckChannelBoard_B()
// ****************************************************************


#define _COPY_DATA 	Board->NumRangeI = Channel->NumRangeI;\
		Board->NumRangeV = Channel->NumRangeV;\
		pvmm->LastData = Board->Data & (~_EVENDATA_B);

#define _SCALE_X(x,calibre,numrangeI,numrangeV) x =\
	(_FULLSCALE_REF_B*(double)Board->OldOddMeasure)/(_FULLSCALE_B*(double)Board->Measure);\
	x /= TabRangeI_B.rg[numrangeI].Value;\
	x *= TabRangeV_B.rg[numrangeV].Value;\
	if (!calibre)\
		x *= pvmm->CorrectionI[0];\
	if (fabs(x)<1e-10)\
		x = 1e-10;\
	x = _IMPEDANCE_CONVERSION/x;

//*************************************************************
//						CALIBRATION MODE (B)
//*************************************************************

int Calibrate_B(ABOARD *Board)
{
	PRIVATEMEMORY_B *pvmm = Board->PrivateMemory; 
	ACHANNEL *Channel = Board->Channels[0];

	double x,y;

	MeasuredValue(Board);  //Only to perfrom the shifet between the 3 lsat values

	//*************************************************************
	//				ODD ticks : that's it return
	//*************************************************************
	if (Board->Time%2)
	{
		Board->Data = _ODDDATA_B;
		Board->Data |= Board->parameter.AddressofBoard-1;
		Board->Data |= (Board->parameter.AddressofBoard-1)<<16;
		Board->Data |= pvmm->LastData;

		return _RETURN_OK;
	}
	Board->Data = _EVENDATA_B;
	Board->Data |= Board->parameter.AddressofBoard-1;
	Board->Data |= (Board->parameter.AddressofBoard-1)<<16;

	y = _RANGEI_CALIBRATION_B;
	SetRangeAndData(&TabRangeI_B,_RANGEBYVALUE,&Channel->NumRangeI,&y,&Board->Data);
	x = _RANGEV_CALIBRATION_B;
	SetRangeAndData(&TabRangeV_B,_RANGEBYVALUE,&Channel->NumRangeV,&x,&Board->Data);
	_COPY_DATA;

	_SCALE_X(x,1,Channel->NumRangeI,Channel->NumRangeV);
	
	if (pvmm->CalibrationCpt++ <= _WAIT_BEFORE_CALIBRATION_B)
		return _RETURN_OK;

	pvmm->Accumulator += x;
	if (pvmm->Hit++ <= _CALIBRATION_TIME_B)
		return  _RETURN_OK; 
	
	// Here the calibration has been completed
	pvmm->Accumulator /= pvmm->Hit;
	x = pvmm->Accumulator/_REF_CALIBRATION_B;
	if (fabs(x-1)<_ACCEPT_CALIBRATION_B)
	{	
		Board->parameter.CalibrationTable[0] = x;
		pvmm->CorrectionI[0] = Board->parameter.CalibrationTable[0];
		Board->parameter.CalibrationStatus = _NORMAL_MODE;
	}
	else
		Board->parameter.CalibrationStatus = _CALIBRATION_FAILED;

	// return since calibration completed
	return _RETURN_OK;
}//				FIN Calibrate_B()
// ************************************************************************

long int CalcBoard_B(ABOARD *Board)
{
	short int oldodd,oldeven,flag;
	double x;
	ACHANNEL *Channel;
	int flagchangerange;
	int i;
	PRIVATEMEMORY_B *pvmm = Board->PrivateMemory; 

	if (Board->parameter.CalibrationStatus == _CALIBRATION_MODE)
	{
		Calibrate_B(Board);
		return _RETURN_OK;
	}

	Channel = Board->Channels[Board->ChannelTreatedOld];

	// the two first call need a special treatement
	if (pvmm->veryfirst)
	{
		flag = 0;
		pvmm->veryfirst--;
		goto next;
	}

	//*************************************************************
	//				FIRST CALCULATE THE CORRECTED DATA
	//*************************************************************
	MeasuredValue(Board);  //Only to perfrom the shifet between the 3 lsat values

	//*************************************************************
	//				ODD ticks : that's it return
	//*************************************************************
	if (Board->Time%2)
	{
		Board->Data = _ODDDATA_B;
		Board->Data |= Board->parameter.AddressofBoard-1;
		Board->Data |= (Board->parameter.AddressofBoard-1)<<16;
		Board->Data |= pvmm->LastData;
		if (Channel->fifopt->TimerForgetChange >= 2)
			Channel->fifopt->TimerForgetChange --;

		return _RETURN_OK;
	}

	//*************************************************************
	//				Things to be done only for EVEN ticks
	//*************************************************************

	// compute the value taking ranges and corrections into account
		// Calculation of the result
	_SCALE_X(x,0,Board->NumRangeIOld,Board->NumRangeVOld);

	//*************************************************************
	//						SECOND FILL THE FIFO
	//*************************************************************
	// To fill the fifo, prepare a local channel 
	// The channel number _MAXCHANNELPERBOARD is reseverd for calibration
	Channel->NumRangeIOld = Channel->NumRangeI;
	Channel->NumRangeVOld = Channel->NumRangeV;
	Channel->OldMeasureTime = Channel->mes.Time+1; // +1 since go here every other tic
	Channel->mes.Time = Board->Time;
	Channel->mes.MeasureRaw = x;
	// if this channel is used to regulate then convert
	if (Channel->ConversionDone)
	{
		Channel->ConversionDone &= 0xfffffff5;; // bit 2 and 4 cleared
		if (Channel->parameter.Etalon)
		{
			if ((Channel->parameter.Etalon)(&x))
				Channel->mes.Status |= _BIT_CONVERSION_ERROR;
		}
	}
	Channel->mes.Measure = x;
	// SINCE BOARD E&F&G IS NOT CORRECTLY PROGRAMMED SATOUT IS INVERSED, TO CORRECT
//	if ((var_adef->BOARDTYPE == 'E')||(var_adef->BOARDTYPE == 'F')||(var_adef->BOARDTYPE == 'G'))
//		Board->Over = Board->Over ^ _BITOVERLOAD;
	// FIN SINCE BOARD E&F IS NOT CORRECTLY PROGRAMMED SATOUT IS INVERSED, TO CORRECT
	if ((Board->Over) & (_BITOVERLOAD))
		Board->Status |= _BIT_STATUS_OVERLOAD;

	Channel->mes.Status = Board->Over | Board->Status;
	// since only the first bit is significant for Board
	if (Board->parameter.CalibrationStatus == _START_CALIBRATION_MODE)
	{
		flag = _FORCE_STOP_AVERAGING;
		
		pvmm->CalibrationCpt = 0;
		pvmm->Accumulator = 0;
		pvmm->Hit = 0;

		Board->parameter.CalibrationStatus = _CALIBRATION_MODE;
	}
	else
	{
		flag = !(_FORCE_STOP_AVERAGING);
	}
	// here flag is set to force finishing an averaging 
	flag = WriteFifoMeasure(Channel,flag);
	// flag is set if an averaging has been completed

	// save the values of Ranges and ChannelTreated used for the last send
next:
	Board->NumRangeIOld = Board->NumRangeI;
	Board->NumRangeVOld = Board->NumRangeV;
	Board->ChannelTreatedOld = Board->ChannelTreated;

	//*************************************************************
	//				THIRD DETERMINE NEXT CHANNEL
	//*************************************************************
	Board->ChannelTreated = 0;  // only one channel for board B
	Board->ChannelTreatedOld = 0;  // only one channel for board B
	Channel = Board->Channels[Board->ChannelTreated];

	//*************************************************************
	//				FOURTH DETERMINE NEW DATA
	//*************************************************************
	Board->Data = _EVENDATA_B;

	// Include the address and subadress in the board
	// No sub address for board B
	Board->Data |= Board->parameter.AddressofBoard-1;
	Board->Data |= (Board->parameter.AddressofBoard-1)<<16;

	//*************************************************************
	//				FOURTH DETERMINE NEW RANGES AND STATUS
	//*************************************************************
	// start with status:
	Board->Status = 0;
	flagchangerange = _NOCHANGE;	
	oldodd = Board->OldOddMeasure;
	oldeven = Board->OldEvenMeasure;
	flag = _OK;
	if ((_ABS(oldodd) < _FLOOR_B) )
		flag = _TOOSMALL;
	if ((_ABS(oldodd)>_CEIL_B) )
		flag = _TOOBIG;
	if (flag == _TOOBIG)
		Board->Status = _BIT_TOOBIG;
	else if (flag == _TOOSMALL)
		Board->Status = _BIT_TOOSMALL;

	// test if the value in the dac was the largest or smallest possible value
	if ((oldodd == -32768)  || (oldodd==32767))
		Board->Status |= _BIT_DAC_AT_MAX;
	if ((oldeven == -32768)  || (oldeven==32767))
		Board->Status |= _BIT_DAC_AT_MAX;

	//------------------------------------------------------------*
	//						FIXED MODE
	//------------------------------------------------------------*
	if (Channel->parameter.Mode == _FIX_RANGE_MODE)
	{
		SetRangeAndData(&TabRangeI_B,_RANGEBYVALUE,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);

		SetRangeAndData(&TabRangeV_B,_RANGEBYVALUE,
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
		_COPY_DATA;

		return _RETURN_OK;
	}

	//------------------------------------------------------------*
	//						AUTOSCALE MODES
	//------------------------------------------------------------*
	switch (flag)
	{
	case _TOOBIG: // Possibly _RANGEPLUS on I or _RANGEMINUS on V
		pvmm->NumberUnderRange = 0;
		if (pvmm->NumberOverRange++ >= _MAX_NUMBER_OVERRANGE_B)
		{
			pvmm->NumberOverRange = 0;
			flagchangerange = _TOOBIG;
		}
		break;
	case _TOOSMALL:// Possibly _RANGEMINUS on I or _RANGEPLUS on V
		pvmm->NumberOverRange = 0;
		if (pvmm->NumberUnderRange++ >= _MAX_NUMBER_UNDERRANGE_B)
		{// Enough Underrange in a row
			pvmm->NumberUnderRange = 0;
			flagchangerange = _TOOSMALL;
		}
		break;
	default: 
		pvmm->NumberOverRange = 0;
		pvmm->NumberUnderRange = 0;
	}
// ************************** HAVE TO BE CHECKED ****************************** 	
// RANGEMINUS on V
//	if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING) break;
// RANGEPLUS on I 
//	if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING) break;

// ************************** HAVE TO BE NOTED ****************************** 	
// RANGEMINUS on I
// pvmm->LastDecrI = Board->Time;
// RANGEPLUS on V
// pvmm->LastIncrV =  Board->Time;

	switch (flagchangerange)
	{
	case _TOOBIG:		//  case overrange
		switch (Channel->parameter.Mode )
		{
		case _FIX_CURRENT_MODE:		// and too big
			// RANGEPLUS on V
			pvmm->LastIncrV =  Board->Time;
			SetRangeAndData(&TabRangeV_B,
					_RANGEPLUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&TabRangeI_B,
					_RANGEBYVALUE,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);			
			_COPY_DATA;
			return _RETURN_OK;

		case _FIX_VOLTAGE_MODE:		// and too big
			// RANGEMINUS on I
			pvmm->LastDecrI = Board->Time;
			SetRangeAndData(&TabRangeV_B,
					_RANGEBYVALUE,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&TabRangeI_B,
					_RANGEMINUS,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_CURRENT_MODE:		// and too big
			// RANGEMINUS on V
			if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING_B)
				break;
			i = SetRangeAndData(&TabRangeV_B,
					_RANGEPLUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			if (i==_RANGE_CHANGE_NOT_POSSIBLE)
			{
				// RANGEMINUS on I
				pvmm->LastDecrI = Board->Time;
				flag = _RANGEMINUS;
			}
			else
			{
				flag = _RANGEBYVALUE;
			}
			SetRangeAndData(&TabRangeI_B,
					flag,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data); 
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_VOLTAGE_MODE:		// and too big
			// RANGEMINUS on I
			pvmm->LastDecrI = Board->Time;
			i = SetRangeAndData(&TabRangeI_B,
					_RANGEMINUS,	
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			if (i==_RANGE_CHANGE_NOT_POSSIBLE)
			{
				// RANGEPLUS on V
				pvmm->LastIncrV =  Board->Time;
				flag = _RANGEPLUS;
			}
			else
				flag = _RANGEBYVALUE;
			SetRangeAndData(&TabRangeV_B,
					flag,
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data); 
			_COPY_DATA;
			return _RETURN_OK;

		default: 
			break;
		}			// END switch (Channel->parameter.Mode)
		break;      // END case _TOOBIG
	case _TOOSMALL:
		switch (Channel->parameter.Mode )		
		{
		case _FIX_CURRENT_MODE:		// and too small 
			// RANGEMINUS on V
			if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING_B)
				break;
			SetRangeAndData(&TabRangeV_B,
					_RANGEMINUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&TabRangeI_B,
					_RANGEBYVALUE,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _FIX_VOLTAGE_MODE:		// and too small
			// RANGEPLUS on I 
			if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING_B)
				break;
			SetRangeAndData(&TabRangeV_B,
					_RANGEBYVALUE,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&TabRangeI_B,
					_RANGEPLUS,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_CURRENT_MODE:		// and too small
			// RANGEMINUS on V
			if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING_B)
				break;
			i = SetRangeAndData(&TabRangeV_B,
					_RANGEMINUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			if (i==_RANGE_CHANGE_NOT_POSSIBLE)
			{
				// RANGEPLUS on I 
				if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING_B) 
					break;
				flag = _RANGEPLUS;
			}
			else
				flag = _RANGEBYVALUE;
			SetRangeAndData(&TabRangeI_B,
					flag,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_VOLTAGE_MODE:		// and too small
			// RANGEPLUS on I 
			if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING_B) 
				break;
			i = SetRangeAndData(&TabRangeI_B,
					_RANGEPLUS,	
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			if (i==_RANGE_CHANGE_NOT_POSSIBLE)
			{
				// RANGEMINUS on V
				if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING_B) 
					break;
				flag = _RANGEMINUS;
			}
			else
				flag = _RANGEBYVALUE;
			SetRangeAndData(&TabRangeV_B,
					flag,
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;
		
		default: 
			break;
		}			// END switch (Channel->parameter.Mode)
	break;      // END case _TOOSMALL

	default: 
		break;// NOT CHANGE OF RANGE
	}	// END switch (flagchangerange)

	flag = _RANGEBYVALUE;
	SetRangeAndData(&TabRangeI_B,flag, //flag depends on caibration or not
				&Channel->NumRangeI,
				&Channel->parameter.ValueRangeI,
				&Board->Data);

	SetRangeAndData(&TabRangeV_B,flag,//flag depends on caibration or not
				&Channel->NumRangeV,
				&Channel->parameter.ValueRangeV,
				&Board->Data);
	_COPY_DATA;
	return _RETURN_OK;

}				// FIN CalcBoard_ADEF()
// ****************************************************************

