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
#include "TrmcBoardC.h"

long int InitBoard_C(ABOARD *Board)
{
	PRIVATEMEMORY_C *pt = 0;    // =0 en plus
	double Z1,Z2;
	int i;

	if (Board->PrivateMemory==0)	// First call = default values
	{
		pt = (PRIVATEMEMORY_C *)malloc(sizeof(PRIVATEMEMORY_C));
		if (pt==0)
			return _CANNOT_ALLOCATE_MEM;

		Board->PrivateMemory = pt;

		// set number of calibration
		Board->parameter.NumberofCalibrationMeasure = _NB_COEF_CALIBRATION_C;

		// initialize coefficicient with 1.0
		for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++)
		{
			Board->parameter.CalibrationTable[i] = -2.0;
		}
		Board->parameter.CalibrationTable[0] = 1;// close +1
		Board->parameter.CalibrationTable[1] = -1;// close to -1


		// Set the channel treated to zero since always present
		Board->ChannelTreated = 0;

	}
		
	pt = Board->PrivateMemory;

	Z1 = Board->parameter.CalibrationTable[0];// close to +1 V=0.5
	Z2 = Board->parameter.CalibrationTable[1];// close to -1 V=2

	pt->CoefCst = (0.5*Z2 - 2*Z1)/(Z2-Z1);
	pt->Slope = (2.0-0.5)/(Z2-Z1);

	Board->parameter.CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE-1]	= pt->Slope;
	Board->parameter.CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE-2]	= pt->CoefCst;

	Board->parameter.NumberofIRanges = _NUMBER_RANGEI_C;
	Board->parameter.IRangesTable[0] = _VALUERANGE_I_C;
		
	Board->parameter.NumberofVRanges = _NUMBER_RANGEV_C;
	Board->parameter.VRangesTable[0] = _VALUERANGE_V_C;

	Board->NumberofChannels = _NB_SUBADD_C; // Number of channel for this Board

	Board->OddData = _ODDDATA_C;
	Board->EvenData = _EVENDATA_C;

	return _RETURN_OK;
}				// FIN InitBoard_C()
// ****************************************************************

long int CheckChannelBoard_C(ACHANNEL *Channel)
{
	int somethingchanged=0;

	//				Check sub_address
	if (Channel->parameter.SubAddress >= _NB_SUBADD_C)
		return _INVALID_SUBADDRESS;

	//				Check mode
	// all modes in case are ok, default is not;
	switch (Channel->parameter.Mode)
	{
	case _INIT_MODE:
		Channel->NumRangeI = _INIT_RANGEI_C;
		Channel->NumRangeV = _INIT_RANGEV_C;
		Channel->parameter.Mode = _INIT_BOARD_MODE_C;
		Channel->parameter.PreAveraging = _INIT_PREAVERAGING_C;
		Channel->parameter.ValueRangeI = _VALUERANGE_I_C;
		Channel->parameter.ValueRangeV = _VALUERANGE_V_C;
	case _NOT_USED_MODE:
	case _FIX_RANGE_MODE:
	case _CALIBRATION_MODE:
		break;
	default:
		// choose between this 2 possibilities
		return _INVALID_MODE; 
	}

	if (Channel->parameter.ValueRangeI != _VALUERANGE_I_C)
		return _RANGE_CHANGE_NOT_POSSIBLE;
	if (Channel->parameter.ValueRangeV != _VALUERANGE_V_C)
		return _RANGE_CHANGE_NOT_POSSIBLE;

	
	if (somethingchanged) 
		return _CHANNEL_HAS_BEEN_MODIFIED;  // This NOT an error since <0

	return _RETURN_OK ;
}				// FIN CheckChannelBoard_C()
// ****************************************************************

void Calibration_C(ABOARD *Board)
{
	PRIVATEMEMORY_C *pvmm = Board->PrivateMemory; 
	double x;
	int ok;

	if (Board->Time%2)
	{
		Board->Data = _ODDDATA_C;
		Board->Data |= Board->parameter.AddressofBoard-1;
		Board->Data |= (Board->parameter.AddressofBoard-1)<<16;

		return ;
	}

	//*************************************************************
	//				Things to be done only for EVEN ticks
	//*************************************************************
		// mesure de la ref ou du signal
	Board->Data = _EVENDATA_C;
	Board->Data |= Board->parameter.AddressofBoard-1;
	Board->Data |= (Board->parameter.AddressofBoard-1)<<16;

	// Calculation of the result
	x = ((double)Board->OldOddMeasure)/((double)Board->Measure);
	
	ok = 1;
	if ( Board->OldEvenMeasure == -32768) 
		ok = 0;
	if (Board->OldEvenMeasure == 32767)
		ok = 0;
	if (Board->OldOddMeasure == -32768) 
		ok = 0;
	if (Board->OldOddMeasure  == 32767) 
		ok = 0;
	if ((Board->Over) & (_BITOVERLOAD))
		ok = 0;
	if 	((fabs(x-1) > _ACCEPT_CALIBRATION_C) && (fabs(x+1) > _ACCEPT_CALIBRATION_C))
		ok = 0;

	if (!ok)
		Board->parameter.CalibrationStatus = _CALIBRATION_FAILED;

	pvmm->Accumulator += x;
	pvmm->Hit ++;

	if (pvmm->Hit < _CALIBRATION_TIME_C)
		return;

	// ************************************************************
	//					CALIBRATION COMPLETED
	// ************************************************************
	x = pvmm->Accumulator/pvmm->Hit;

	Board->parameter.CalibrationStatus = _NORMAL_MODE;
	if (fabs(x-1) < _ACCEPT_CALIBRATION_C)
		Board->parameter.CalibrationTable[0] = x;// close +1
	else if (fabs(x+1) < _ACCEPT_CALIBRATION_C)
		Board->parameter.CalibrationTable[1] = x;// close -1
	else
		Board->parameter.CalibrationStatus = _CALIBRATION_FAILED;

	// now translate measure into coeff:
	InitBoard_C(Board);
	
}				// FIN Calibration_C()
// ****************************************************************

long int CalcBoard_C(ABOARD *Board)
{
	int flag;
	double x;
	ACHANNEL *Channel;
	PRIVATEMEMORY_C *pvmm = Board->PrivateMemory; 

	Channel=Board->Channels[0];	// board C has only one channel

	if (Board->parameter.CalibrationStatus == _CALIBRATION_MODE)
	{
		Calibration_C(Board);
		return _RETURN_OK;
	}

	if (Channel->parameter.Mode == _NOT_USED_MODE)
	{
		Board->ChannelTreated = -1; // Ie no chanel treated
		return _RETURN_OK;
	}
	Board->ChannelTreated = 0;

	//*************************************************************
	//				Things to be done for EVEN AND ODD ticks
	//*************************************************************
	MeasuredValue(Board);  //Only to perfrom the shifet between the 3 lsat values
	// Include the address and subadress in the board

	//*************************************************************
	//				ODD ticks : that's it return
	//*************************************************************
	if(Board->Time%2)
	{
		Board->Data = _ODDDATA_C;
		Board->Data |= Board->parameter.AddressofBoard-1;
		Board->Data |= (Board->parameter.AddressofBoard-1)<<16;
		if (Channel->fifopt->TimerForgetChange >= 2)
			Channel->fifopt->TimerForgetChange --;

		return _RETURN_OK;
	}

	//*************************************************************
	//				Things to be done only for EVEN ticks
	//*************************************************************
		// mesure de la ref ou du signal
	Board->Data = _EVENDATA_C;
	Board->Data |= Board->parameter.AddressofBoard-1;
	Board->Data |= (Board->parameter.AddressofBoard-1)<<16;

	// Calculation of the result
	x = ((double)Board->OldOddMeasure)/((double)Board->Measure);	
	x = pvmm->CoefCst + pvmm->Slope*x; // Corrected value

	// update Channel
	Channel->NumRangeIOld = Channel->NumRangeI;
	Channel->NumRangeVOld = Channel->NumRangeV;
	Channel->OldMeasureTime = Channel->mes.Time+1; // +1 since go here every other tic
	Channel->mes.Time = Board->Time;
	Channel->mes.MeasureRaw = x;
	// if this channel is used to regulate then convert
	if (Channel->ConversionDone)
	{
		Channel->ConversionDone &= 0xfffffff5; // bit 2 and 4 cleared
		if (Channel->parameter.Etalon)
		{
			if ((Channel->parameter.Etalon)(&x))
				Channel->mes.Status |= _BIT_CONVERSION_ERROR;
		}
	}
	Channel->mes.Measure = x;

	if ((Board->Over) & (_BITOVERLOAD))
		Board->Status |= _BIT_STATUS_OVERLOAD;

	Channel->mes.Status = Board->Over | Board->Status;

	//*************************************************************
	//						 FILL THE FIFO
	//*************************************************************
	// here flag is set to force finishing an averaging 
	if (Board->parameter.CalibrationStatus == _START_CALIBRATION_MODE)
	{
		flag = _FORCE_STOP_AVERAGING;
		Board->parameter.CalibrationStatus = _CALIBRATION_MODE;
		pvmm->Accumulator = 0;
		pvmm->Hit = 0;
	}
	else
	{
		flag = !(_FORCE_STOP_AVERAGING);
	}
	flag = WriteFifoMeasure(Channel,flag);


	//*************************************************************
	//						Prepare the status
	//*************************************************************
	Board->Status = 0;
	flag = _OK;
	if ((_ABS(Board->OldOddMeasure) < _FLOOR_C) || (_ABS(Board->OldEvenMeasure) < _FLOOR_C))
		Board->Status = _BIT_TOOSMALL;
	if ((_ABS(Board->OldOddMeasure)>_CEIL_C) || (_ABS(Board->OldEvenMeasure)>_CEIL_C))
		Board->Status = _BIT_TOOBIG;

	// test if the value in the dac was the largest or smallest possible value
	if ((Board->OldEvenMeasure == -32768)  || (Board->OldEvenMeasure==32767))
		Board->Status |= _BIT_DAC_AT_MAX;
	if ((Board->OldOddMeasure == -32768)  || (Board->OldOddMeasure==32767))
		Board->Status |= _BIT_DAC_AT_MAX;

	return _RETURN_OK ;
}				// FIN CalcBoard_C()
// ****************************************************************

