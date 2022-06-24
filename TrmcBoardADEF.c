// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef powerc
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <malloc.h>
#include <math.h> 
#else
#include "manip.h"
#endif

#include "Trmc.h" 
#include "TrmcRunLib.h"
#include "TrmcDef.h"
#include "TrmcProto.h"
#include "TrmcBoard.h"

#define _COPY_DATA 	Board->NumRangeI = Channel->NumRangeI;\
		Board->NumRangeV = Channel->NumRangeV


#define _NB_RANGEI (var_adef->NB_RANGEI)
#define _NB_RANGEV (var_adef->NB_RANGEV)
#define _NB_MEASURE_CALIB (var_adef->NB_MEASURE_CALIB)
#define _NB_SUBADD (var_adef->NB_SUBADD)
#define _INIT_BOARD_MODE (var_adef->INIT_BOARD_MODE)
#define _INIT_PREAVERAGING (var_adef->INIT_PREAVERAGING)
#define _ACCEPT_CALIBRATION (var_adef->ACCEPT_CALIBRATION)
#define _WAIT_BEFORE_CALIBRATION (var_adef->WAIT_BEFORE_CALIBRATION)
#define _WAIT_BEFORE_FIRST_CALIBRATION (var_adef->WAIT_BEFORE_FIRST_CALIBRATION)
#define _MAX_OFFSET (var_adef->MAX_OFFSET)
#define _FLOOR (var_adef->FLOOR)
#define _CEIL (var_adef->CEIL)
#define _MAX_NUMBER_OVERRANGE (var_adef->MAX_NUMBER_OVERRANGE)
#define _MAX_NUMBER_UNDERRANGE (var_adef->MAX_NUMBER_UNDERRANGE)
#define _WAIT_DECREASING_V_AFTER_INCREASING (var_adef->WAIT_DECREASING_V_AFTER_INCREASING)
#define _WAIT_INCREASING_I_AFTER_DECREASING (var_adef->WAIT_INCREASING_I_AFTER_DECREASING)
#define _ODDDATA (var_adef->ODDDATA)
#define _EVENDATA (var_adef->EVENDATA)
#define _MASK_SUBADD (var_adef->MASK)
#define _FULLSCALE  (var_adef->FULLSCALE)
#define _NB_MEASURE_OFFSET (var_adef->NB_MEASURE_OFFSET)

#define _TABMEASCAL (var_adef->TabMeasCal)


#define _PRIVATEMEMORY PRIVATEMEMORY_ADEF



//*************************************************************
//						CALIBRATION MODE 
//*************************************************************

int Calibrate_ADEF(ABOARD *Board,VAR_ADEF *var_adef)
/*
x is the last calculated value, could be retreived from Board but cubersome..

     w                t0+2w                    t0+t1+3w 
|----|--------------|---|--------------------|---|--...........
0                 t0+w                    t0+t1+2w

  between T+2*k*w < t <=  T+(2*k+1)*w : waiting ie not measuring
  between T+(2*k+1)*w < t <=  T+(2*k+2)*w : performing measure k

  w is a constant

  pvmm->pvmm->CalibrationSubCpt = counter incrementead each tic set at beg. of per 
  pvmm->pvmm->CalibrationCpt = counter incrementead each tic -1 the very first time
  pvmm->CalibrationNextEvent = next change of regime
  pvmm->CalibrationIndex = number of the period in progress
		if even ==> waiting before period number pvmm->CalibrationIndex/2
		if odd ==>  making period number pvmm->CalibrationIndex/2
 */
{
	int i,ok,WaitingTime;
	ACHANNEL pt;
	ACHANNEL *Channel=&pt;
	double x;
	_PRIVATEMEMORY *pvmm = Board->PrivateMemory; 

	// Treat the very first time
	if (pvmm->CalibrationCpt==-1) 
	{					// STARTING THE CALIBRATION
		// save the old mesaures if calibration unsuccessfull
		for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++)
			Board->CalibrationTableSave[i] = Board->parameter.CalibrationTable[i];

		Board->Offset = 0;

		pvmm->CalibrationCpt = 0;
		pvmm->CalibrationSubCpt = 0;
		pvmm->Accumulator = 0;
		pvmm->Hit = 0;
		pvmm->CalibrationNextEvent = 
			_TABMEASCAL.rg[0].Time+_WAIT_BEFORE_CALIBRATION; 
		pvmm->CalibrationIndex = 0; // set the index to determine the measure
	}

	// prepare the data to send
	if (Board->Time%2)
		Board->Data = _ODDDATA;
	else
		Board->Data = _EVENDATA;

	// Include the address and subadress in the board
	Board->Data |= var_adef->MASK_SUBADD[Board->ChannelTreated];
	Board->Data |= Board->parameter.AddressofBoard-1;
	Board->Data |= (Board->parameter.AddressofBoard-1)<<16;


	Channel->NumRangeI = _TABMEASCAL.rg[pvmm->CalibrationIndex].NumRangeI;
	Channel->NumRangeV = _TABMEASCAL.rg[pvmm->CalibrationIndex].NumRangeV;
	Board->ChannelTreated = _TABMEASCAL.rg[pvmm->CalibrationIndex].Channel;
	SetRangeAndData(&(var_adef->TabRangeI),_RANGEBYNUMBER,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
	SetRangeAndData(&(var_adef->TabRangeV),_RANGEBYNUMBER,
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
	_COPY_DATA;

	// Scale the value x
	x = MeasuredValue(Board);  // |r| cannot exceed 1 by a large amount
	x *= var_adef->TabRangeV.rg[Channel->NumRangeV].Value/
		var_adef->TabRangeI.rg[Channel->NumRangeI].Value;

	// Set the 	_BIT_RECENT_CHANGE of 	Board->Status  if an overange of dac full occurs
	// if will be tested only if accumulating
	if ((var_adef->BOARDTYPE == 'E') || (var_adef->BOARDTYPE == 'F')|| (var_adef->BOARDTYPE == 'G'))
		Board->Over = Board->Over ^ _BITOVERLOAD;
	// FIN SINCE BOARD E&F&G IS NOT CORRECTLY PROGRAMMED SATOUT IS INVERSED, TO CORRECT

	// Handle the status : only Overload (from the board) or dac full tested
	Board->Status = 0;
	if ((Board->Over) & (_BITOVERLOAD))
		Board->Status = (unsigned short) _BIT_RECENT_CHANGE;
	if ((Board->Measure == -32768)||(Board->Measure == 32767))
		Board->Status = (unsigned short) _BIT_RECENT_CHANGE;

	// Accumulate after delay
	if (pvmm->CalibrationIndex <2) // the first two ranges neccessitate a longer waiting time
	{
		// IN THESE CASES (2 first step) one introduce an extra waiting time
		WaitingTime = _WAIT_BEFORE_FIRST_CALIBRATION;
		pvmm->CalibrationNextEvent = 
			_TABMEASCAL.rg[pvmm->CalibrationIndex].Time-_WAIT_BEFORE_CALIBRATION+WaitingTime; 
	}
	else 
		WaitingTime = _WAIT_BEFORE_CALIBRATION;

	if (pvmm->CalibrationSubCpt++ < WaitingTime)
	{				// clear the cpts
		pvmm->Accumulator = 0;
		pvmm->Hit = 0;
	}
	else
	{				// accumulate and update fifo
		pvmm->Accumulator += x;
		pvmm->Hit++;

		// No recent change is allowed during calibration
		if (Board->Status & _BIT_RECENT_CHANGE) 
			goto OnError;
	}
	// leave only if it is not the end of a period
	if (pvmm->CalibrationSubCpt < pvmm->CalibrationNextEvent)
		return _RETURN_OK; // Period k still running

	//*************************************************************
	// Here it is the END of a MEASURING PERIOD k
	//*************************************************************
	
	// perform the average and copy the result in Board
	pvmm->Accumulator /= pvmm->Hit;
	Board->parameter.CalibrationTable[pvmm->CalibrationIndex] = 
		pvmm->Accumulator /_TABMEASCAL.rg[pvmm->CalibrationIndex].Value;

	// a new period
	pvmm->CalibrationIndex++;
	pvmm->CalibrationSubCpt = 0;
	pvmm->CalibrationNextEvent = 
		_TABMEASCAL.rg[pvmm->CalibrationIndex].Time+_WAIT_BEFORE_CALIBRATION; 

		// Test if offset is to be calculated
	if (pvmm->CalibrationIndex == _NB_MEASURE_OFFSET)
	{
		switch (Board->parameter.TypeofBoard)
		{
		case _TYPED: 
			InitBoard_D(Board);break;
		case _TYPEE: 
			InitBoard_E(Board);break;
		case _TYPEF :
			InitBoard_F(Board);break;
		case _TYPEG :
			InitBoard_G(Board);break;
		case _TYPEA: 
		default: return _INTERNAL_INCONSISTENCY;
		}
		if (Board->Offset > _MAX_OFFSET) 
			goto OnError;
	}

	if (pvmm->CalibrationIndex < _TABMEASCAL.NumberMeasure)
		return _RETURN_OK; // this was NOT the last 
	
	// CALIBRATION TERMINATED CALIBRATION TERMINATED 
	pvmm->CalibrationCpt = -1; // for next calibration

	// to set the calibration coeff and offset from the measures
	switch (Board->parameter.TypeofBoard)
	{
	case _TYPEA: 
		InitBoard_A(Board);break;
	case _TYPED: 
		InitBoard_D(Board);break;
	case _TYPEE: 
		InitBoard_E(Board);break;
	case _TYPEF:
		InitBoard_F(Board);break;
	case _TYPEG:
		InitBoard_G(Board);break;
	default: return _INTERNAL_INCONSISTENCY;
	}
	// test if the values are reasonnable
	// test offset
	ok = 1;
	if (Board->Offset > _MAX_OFFSET) 
		ok = 0;
	// test range V
	for (i=0;i<_NB_RANGEV;i++)
	{
		if(fabs(pvmm->CorrectionV[i]-1)>_ACCEPT_CALIBRATION) 
			ok=0;
	}
	// test range I
	for (i=0;i<_NB_RANGEI;i++)
	{
		if(fabs(pvmm->CorrectionI[i]-1)>_ACCEPT_CALIBRATION) 
			ok=0;
	}
			
	if (ok)		// Calibration accepted : use new values 
		Board->parameter.CalibrationStatus = _NORMAL_MODE;
	else 
	{				// Calibration rejected : use previous values
OnError:
	pvmm->CalibrationCpt = -1; // for next calibration
	for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++)
			Board->parameter.CalibrationTable[i] = Board->CalibrationTableSave[i];
		Board->parameter.CalibrationStatus = _CALIBRATION_FAILED;
		// initborad to translate measuredvalue/expectedvalue -> corection coeff.
		switch (Board->parameter.TypeofBoard)
		{
		case _TYPEA: InitBoard_A(Board);break;
		case _TYPED: InitBoard_D(Board);break;
		case _TYPEE: InitBoard_E(Board);break;
		case _TYPEF: InitBoard_F(Board);break;
		case _TYPEG: InitBoard_G(Board);break;
		default: return _INTERNAL_INCONSISTENCY;
		}
	}

	return _RETURN_OK; // return and calibtation completed
}//				FIN Calibrate_ADEF()
// ************************************************************************

long int CheckChannelBoard_ADEF(ACHANNEL *Channel,VAR_ADEF *var_adef)
{
	int	k,somethingchanged = 0;

	//				Check sub_address
	if (Channel->parameter.SubAddress >= _NB_SUBADD)
		return _INVALID_SUBADDRESS;

	// When only one channel for the board : mode SHOULD bee normal
	if (var_adef->NB_SUBADD==1)
	{
		if (Channel->parameter.PriorityFlag!=_NO_PRIORITY)
		return _INVALID_PRIORITY;
	}

	//				Check mode
	// all modes in case are ok, default is not;
	switch (Channel->parameter.Mode)
	{
	case _INIT_MODE:
		Channel->NumRangeI = var_adef->TabRangeI.DefaultRange;
		Channel->NumRangeV = var_adef->TabRangeV.DefaultRange;
		Channel->parameter.Mode = _INIT_BOARD_MODE;
		Channel->parameter.PreAveraging = _INIT_PREAVERAGING;
		Channel->parameter.ValueRangeI = 
			var_adef->TabRangeI.rg[var_adef->TabRangeI.DefaultRange].Value;
		Channel->parameter.ValueRangeV = 
			var_adef->TabRangeV.rg[var_adef->TabRangeV.DefaultRange].Value;
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

	k = SetRangeAndData(&(var_adef->TabRangeV),_RANGEBYVALUE,
		&Channel->NumRangeV,&Channel->parameter.ValueRangeV,0);
	if (k<0) 
		return _RANGE_CHANGE_NOT_POSSIBLE;
	if (k>0)
		somethingchanged++;

	k = SetRangeAndData(&(var_adef->TabRangeI),_RANGEBYVALUE,
		&Channel->NumRangeI,&Channel->parameter.ValueRangeI,0);
	if (k<0) 
		return _RANGE_CHANGE_NOT_POSSIBLE;
	if (k>0)
		somethingchanged++;

	if (somethingchanged)
		return _CHANNEL_HAS_BEEN_MODIFIED;
	else
		return _RETURN_OK;
}				// FIN CheckChannelBoard_ADEF()
// ****************************************************************

long int InitBoard_ADEF(ABOARD *Board,VAR_ADEF *var_adef)
// Two purposes:
// 1/	Initialize the Board the first time with default values.
// 2/	Derive the correction coefficients from the calibration measures.
{
	_PRIVATEMEMORY *pt = 0;
	int i,j;
	double x;

	if (Board->PrivateMemory==0)		// First call = default values
	{
		// This a REAL initialization and NOT to translate the calibration
		// measures into correction coefficient

		// Allocate private memory
		pt = (_PRIVATEMEMORY *)malloc(sizeof(_PRIVATEMEMORY));
		if (pt==0)
			return _CANNOT_ALLOCATE_MEM;
		Board->PrivateMemory = pt;

		// set the private memory parameters
		pt->NumberOverRange=0;	// number of consecutive overrange
		pt->NumberUnderRange=0;  // number of consecutive underrange
		pt->LastIncrV=_THEEPOCH; // last call where an increase was done
		pt->LastDecrI=_THEEPOCH;	// last call where a decrease was done
		pt->CalibrationCpt=-1;	// Counter for calibratiom
		pt->Accumulator = 0;	// Accumulator for calibration
		pt->MeasDone = 0;	// to be equal to MeasToDo
		pt->MeasToDo = 0;
		pt->Hit = 0;

		// set number of calibration (2 for offset + 9 for calibration)
		Board->parameter.NumberofCalibrationMeasure = 
			_NB_MEASURE_CALIB +_NB_MEASURE_OFFSET;
		// the 9 first are for calibration and the two last for offset

		// set fullscale
		Board->FullScale = _FULLSCALE;
		Board->NumberofChannels = var_adef->NB_SUBADD; // Number of channel for this Board

		// Give the calibration measure nominal values
		for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++) 
			Board->parameter.CalibrationTable[i] = -2.0;
		for (i=0;i<Board->parameter.NumberofCalibrationMeasure;i++) 
			Board->parameter.CalibrationTable[i] = 1.0;
		// For board E and G the measure for offset is special
		if (Board->parameter.TypeofBoard == _TYPEE)
			Board->parameter.CalibrationTable[0] = 0.0;
		if (Board->parameter.TypeofBoard == _TYPEG)
			Board->parameter.CalibrationTable[0] = 0.0;

		// Set the channel treated to zero since ChannelAlways present
		Board->ChannelTreated = 0;

		Board->Data = 0;
	}
	pt = Board->PrivateMemory;

	// In CALIBRATION mode, all Channels should be used
	if (Board->parameter.CalibrationStatus == _START_CALIBRATION_MODE)
	{
		for (i=0;i<_NB_SUBADD;i++)
			if (Board->Channels[i]->parameter.Mode == _NOT_USED_MODE)
				return _NOT_USED_CHANNEL_AND_CALIBRATION_INCOMPATIBLE;
	}
	// ***************************************************************
	// SET THE TABLES OF THE V & I RANGES                            *
	// ***************************************************************
	Board->parameter.NumberofIRanges = var_adef->NB_RANGEI;
	for(i=0;i<Board->parameter.NumberofIRanges;i++)
		Board->parameter.IRangesTable[i] = var_adef->TabRangeI.rg[i].Value;
	Board->parameter.NumberofVRanges = var_adef->NB_RANGEV;
	for(i=0;i<Board->parameter.NumberofVRanges;i++)
		Board->parameter.VRangesTable[i] = var_adef->TabRangeV.rg[i].Value;

	// ***************************************************************
	// INITIALIZE CORRECTION COEFFICIENTS FROM CALIBRATION MEASURES  *
	// ***************************************************************
	// first initialize the offset:
	// The  last measure x in CalibrationTable is for offset
	 /*
	Pour la mesure de l'offset a faire AVANT le calcul de coeff.
	Il y a 1 mesures  faire:
	1/ RangeV = 8 RangeI = 0 le resultat ==> X1 entier signe entre -65536 et +65536  
	l'offset est DIRECTEMENT la valeur entiere
	*/

	switch (Board->parameter.TypeofBoard)
	{
	case _TYPEA:
		break;
	case _TYPED:
		OffsetCalulation_D(Board,var_adef,_NB_MEASURE_CALIB+_NB_MEASURE_OFFSET);
		break;
	case _TYPEE:
		OffsetCalulation_E(Board,var_adef,_NB_MEASURE_CALIB+_NB_MEASURE_OFFSET);
		break;
	case _TYPEF:
		OffsetCalulation_F(Board,var_adef,_NB_MEASURE_CALIB+_NB_MEASURE_OFFSET);
		break;
	case _TYPEG:
		OffsetCalulation_G(Board,var_adef,_NB_MEASURE_CALIB+_NB_MEASURE_OFFSET);
		break;
	default: 
		return _INTERNAL_INCONSISTENCY;
	}

	// second initialize the voltage coefficient except for type A
	if (Board->parameter.TypeofBoard != _TYPEA) for (i=0;i<_NB_RANGEV;i++) 
	{
		double y;
		int k;

		for (x=1,j=0;j<_NB_MEASURE_CALIB;j++)
		{
			y = Board->parameter.CalibrationTable[j+_NB_MEASURE_OFFSET];
			k = var_adef->CalibMatrixV[i*var_adef->NB_MEASURE_CALIB+j];
		
			x *= raisepower(y,k);
		}
		pt->CorrectionV[i] = 1/x;
	}
	else // a board of type A has no calibration on V:
		pt->CorrectionV[0] = 1;

	// third initialize the current coefficient
	for (i=0;i<_NB_RANGEI;i++)
	{
		for (x=1,j=0;j<_NB_MEASURE_CALIB;j++)
		{
			x *= raisepower(Board->parameter.CalibrationTable[j+_NB_MEASURE_OFFSET],
				var_adef->CalibMatrixI[i*var_adef->NB_MEASURE_CALIB+j]);
		}
		pt->CorrectionI[i] = 1/x;
	}

	// For board E and F an extra-coefficient gives the quality of the calibration
	// this coeff is returned in the last element of the Calibration Table
	if ((Board->parameter.TypeofBoard == 'E')||(Board->parameter.TypeofBoard == 'F'))
	{
		i = var_adef->NB_RANGEI;
		for (x=1,j=0;j<var_adef->NB_MEASURE_CALIB;j++)
		{
			x *= raisepower(Board->parameter.CalibrationTable[j+var_adef->NB_MEASURE_OFFSET],
				var_adef->CalibMatrixI[i*var_adef->NB_MEASURE_CALIB+j]);
		}
		Board->parameter.CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE-1] =  1/x;
	}

	return _RETURN_OK;
}				// FIN InitBoard_ADEF()
// ****************************************************************

long int CalcBoard_ADEF(ABOARD *Board,VAR_ADEF *var_adef)
{
	short int oldodd,oldeven,flag;
	double x;
	ACHANNEL *Channel;
	int flagchangerange;
	int i;

	_PRIVATEMEMORY *pvmm = Board->PrivateMemory; 

	if (Board->parameter.CalibrationStatus == _CALIBRATION_MODE)
	{
		Calibrate_ADEF(Board,var_adef);
		return _RETURN_OK;
	}

	Channel = Board->Channels[Board->ChannelTreatedOld];

	// the two first call need a special treatement
	if (var_adef->VERYFIRST)
	{
		flag = 0;
		var_adef->VERYFIRST--;
		goto next;
	}
	//*************************************************************
	//				FIRST CALCULATE THE CORRECTED DATA
	//*************************************************************
	// compute the value taking ranges and corrections into account
	x = MeasuredValue(Board);  // |r| cannot exceed 1 by a large amount
	x /= var_adef->TabRangeI.rg[Board->NumRangeIOld].Value;
	x *= var_adef->TabRangeV.rg[Board->NumRangeVOld].Value;
	// Correct the value 
	x *= pvmm->CorrectionI[Board->NumRangeIOld];
	x /= pvmm->CorrectionV[Board->NumRangeVOld];

	//*************************************************************
	//						SECOND FILL THE FIFO
	//*************************************************************
	// To fill the fifo, prepare a local channel 
	// The channel number _MAXCHANNELPERBOARD is reseverd for calibration
	Channel->NumRangeIOld = Channel->NumRangeI;
	Channel->NumRangeVOld = Channel->NumRangeV;
	Channel->OldMeasureTime = Channel->mes.Time;
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
	if ((var_adef->BOARDTYPE == 'E')||(var_adef->BOARDTYPE == 'F')||(var_adef->BOARDTYPE == 'G'))
		Board->Over = Board->Over ^ _BITOVERLOAD;
	// FIN SINCE BOARD E&F&G IS NOT CORRECTLY PROGRAMMED SATOUT IS INVERSED, TO CORRECT
	if ((Board->Over) & (_BITOVERLOAD))
		Board->Status |= _BIT_STATUS_OVERLOAD;

	Channel->mes.Status = Board->Over | Board->Status;
	// since only the first bit is significant for Board
	if (Board->parameter.CalibrationStatus == _START_CALIBRATION_MODE)
	{
		flag = _FORCE_STOP_AVERAGING;
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
	Channel = Board->Channels[Board->ChannelTreated];
	if (flag)
	{
		// an averaging has been completed
		pvmm->MeasDone ++;
		if (Board->ChannelAlways >=0)
		{
			// There is a channel to measure ChannelAlways
			if ((Board->Channels[Board->ChannelAlways]->parameter.Mode == _NOT_USED_MODE) ||
						(Board->Channels[Board->ChannelAlways]->parameter.ScrutationTime==0))
				Board->ChannelAlways = -1; // a not used channel cannot be ChannelAlways
			else
			{
				pvmm->MeasDone = 0;
				// Channel ChannelAlways has been selected
				Board->ChannelTreated = Board->ChannelAlways;
				Channel = Board->Channels[Board->ChannelTreated];
			}
		}
		else
		{
			// There is no ChannelAlways channel
			if (pvmm->MeasDone >= Channel->parameter.ScrutationTime)
			{
				// reset the counter
				pvmm->MeasDone = 0;

				// is there a prority channel ?
				if (Board->ChannelPriority >=0)
				{
					if (Board->ChannelTreated != Board->ChannelPriority)
						Board->ChannelLast = Board->ChannelTreated;
					// There is a channel to measure in ChannelPriority
					if (Board->Channels[Board->ChannelPriority]->parameter.Mode == _NOT_USED_MODE)	
						Board->ChannelPriority = -1; // a not used channel cannot be ChannelPriority
					else
					{
						// Channnel.ChannelPriority is ChannelPriority channel
						if ((Board->ChannelTreated == Board->ChannelPriority) ||
							(Board->Channels[Board->ChannelPriority]->parameter.ScrutationTime==0))
						{
							// is Channel ChannelPriority has been selected
							int i,j,nb = var_adef->NB_SUBADD; // number of subchannel
							ACHANNEL *ChannelTest;

							Board->ChannelLast = (Board->ChannelLast+nb)%nb;
							for (j=0,i=(Board->ChannelLast+1)%nb;j<nb;i=(i+1)%nb,j++)
							{
								if (i==Board->ChannelPriority)
									continue;
								ChannelTest = Board->Channels[i];
								if (ChannelTest->parameter.Mode == _NOT_USED_MODE)
									continue;
								if (ChannelTest->parameter.ScrutationTime == 0)
									continue;
								Board->ChannelTreated = i;
								break;
							}
						} // end if (Board->ChannelTreated == Board->ChannelPriority)
						else
						{
							Board->ChannelTreated = Board->ChannelPriority;
						} // end else if (Board->ChannelTreated == Board->ChannelPriority)
					} // end if (Board->ChannelPriority !=-1)
				}
				else 
				{
					// HERE NO PRIRITY CHANNEL
							// is Channel ChannelPriority has been selected
					int i,nb = var_adef->NB_SUBADD; // number of subchannel
					ACHANNEL *ChannelTest;

					for (i=(Board->ChannelTreated+1)%nb;i!=Board->ChannelTreated;i=(i+1)%nb)
					{
						ChannelTest = Board->Channels[i];
						if (ChannelTest->parameter.Mode == _NOT_USED_MODE)
							continue;
						if (ChannelTest->parameter.ScrutationTime == 0)
							continue;
						Board->ChannelTreated = i;
						break;
					}
				} // end if (Board->ChannelTreated == Board->ChannelPriority)
				Channel = Board->Channels[Board->ChannelTreated];
			}
		}
	}
	Channel = Board->Channels[Board->ChannelTreated];

	//*************************************************************
	//				FOURTH DETERMINE NEW DATA
	//*************************************************************
	if (Board->Time%2)
		Board->Data = _ODDDATA;
	else
		Board->Data = _EVENDATA;

	// Include the address and subadress in the board
	Board->Data |= var_adef->MASK_SUBADD[Board->ChannelTreated];
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
	if ((_ABS(oldodd) < _FLOOR) || (_ABS(oldeven) < _FLOOR))
		flag = _TOOSMALL;
	if ((_ABS(oldodd)>_CEIL) || (_ABS(oldeven)>_CEIL))
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
		SetRangeAndData(&(var_adef->TabRangeI),_RANGEBYVALUE,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);

		SetRangeAndData(&(var_adef->TabRangeV),_RANGEBYVALUE,
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
		if (pvmm->NumberOverRange++ >= _MAX_NUMBER_OVERRANGE)
		{
			pvmm->NumberOverRange = 0;
			flagchangerange = _TOOBIG;
		}
		break;
	case _TOOSMALL:// Possibly _RANGEMINUS on I or _RANGEPLUS on V
		pvmm->NumberOverRange = 0;
		if (pvmm->NumberUnderRange++ >= _MAX_NUMBER_UNDERRANGE)
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
			SetRangeAndData(&(var_adef->TabRangeV),
					_RANGEPLUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&(var_adef->TabRangeI),
					_RANGEBYVALUE,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);			
			_COPY_DATA;
			return _RETURN_OK;

		case _FIX_VOLTAGE_MODE:		// and too big
			// RANGEMINUS on I
			pvmm->LastDecrI = Board->Time;
			SetRangeAndData(&(var_adef->TabRangeV),
					_RANGEBYVALUE,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&(var_adef->TabRangeI),
					_RANGEMINUS,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_CURRENT_MODE:		// and too big
			// RANGEMINUS on V
			if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING)
				break;
			i = SetRangeAndData(&(var_adef->TabRangeV),
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
			SetRangeAndData(&(var_adef->TabRangeI),
					flag,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data); 
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_VOLTAGE_MODE:		// and too big
			// RANGEMINUS on I
			pvmm->LastDecrI = Board->Time;
			i = SetRangeAndData(&(var_adef->TabRangeI),
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
			SetRangeAndData(&(var_adef->TabRangeV),
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
			if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING)
				break;
			SetRangeAndData(&(var_adef->TabRangeV),
					_RANGEMINUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&(var_adef->TabRangeI),
					_RANGEBYVALUE,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _FIX_VOLTAGE_MODE:		// and too small
			// RANGEPLUS on I 
			if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING)
				break;
			SetRangeAndData(&(var_adef->TabRangeV),
					_RANGEBYVALUE,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			SetRangeAndData(&(var_adef->TabRangeI),
					_RANGEPLUS,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_CURRENT_MODE:		// and too small
			// RANGEMINUS on V
			if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING)
				break;
			i = SetRangeAndData(&(var_adef->TabRangeV),
					_RANGEMINUS,	
					&Channel->NumRangeV,
					&Channel->parameter.ValueRangeV,
					&Board->Data);
			if (i==_RANGE_CHANGE_NOT_POSSIBLE)
			{
				// RANGEPLUS on I 
				if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING) 
					break;
				flag = _RANGEPLUS;
			}
			else
				flag = _RANGEBYVALUE;
			SetRangeAndData(&(var_adef->TabRangeI),
					flag,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			_COPY_DATA;
			return _RETURN_OK;

		case _PRIORITY_VOLTAGE_MODE:		// and too small
			// RANGEPLUS on I 
			if (Board->Time - pvmm->LastDecrI <_WAIT_INCREASING_I_AFTER_DECREASING) 
				break;
			i = SetRangeAndData(&(var_adef->TabRangeI),
					_RANGEPLUS,	
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					&Board->Data);
			if (i==_RANGE_CHANGE_NOT_POSSIBLE)
			{
				// RANGEMINUS on V
				if (Board->Time - pvmm->LastIncrV <_WAIT_DECREASING_V_AFTER_INCREASING) 
					break;
				flag = _RANGEMINUS;
			}
			else
				flag = _RANGEBYVALUE;
			SetRangeAndData(&(var_adef->TabRangeV),
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
	SetRangeAndData(&(var_adef->TabRangeI),flag, //flag depends on caibration or not
				&Channel->NumRangeI,
				&Channel->parameter.ValueRangeI,
				&Board->Data);

	SetRangeAndData(&(var_adef->TabRangeV),flag,//flag depends on caibration or not
				&Channel->NumRangeV,
				&Channel->parameter.ValueRangeV,
				&Board->Data);
	_COPY_DATA;
	return _RETURN_OK;

}				// FIN CalcBoard_ADEF()
// ****************************************************************

