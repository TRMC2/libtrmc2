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
#include "TrmcRegul.h"

#define Intervalle(a,b,x) (	(a)<(b) ? ((x)<(a)? (a):((x)<(b)?(x):(b))) : ((x)<(b)? (b):((x)<(a)?(x):(a))) )
 
static TABRANGE TabRangeI = _TABRANGEI_REGUL;

long int InitBoard_Regul(ABOARD *Board)
{
	PRIVATEMEMORY_REGUL *pt;
	int i;

	if (Board->PrivateMemory==0)		// First call = default values
	{
		// This a REAL initialization and NOT to translate the calibration
		// measures into correction coefficient

		// Allocate private memory
		pt = (PRIVATEMEMORY_REGUL *)malloc(sizeof(PRIVATEMEMORY_REGUL));
		if (pt==0)
			return _CANNOT_ALLOCATE_MEM;
		Board->PrivateMemory = pt;

		for(i=0;i<_NB_REGULATING_CHANNEL;i++)
			Board->parameter.CalibrationTable[i] = -1;

		// set the private memory parameters
		pt->Accumulator = 0;	
		pt->LastPdx = 0;
		pt->alpha = 0.1;
		pt->beta = 0.1;
		pt->Filter1Dx = 0;
		pt->Filter2Dx = 0;
		Board->parameter.NumberofCalibrationMeasure = _NB_MEASURE_CALIB_REGUL;
	}
	pt = Board->PrivateMemory;

	if (Board->parameter.TypeofBoard == _TYPEREGULMAIN)
	{
		// main regulation : the values of the parameters are defined in .h
		Board->parameter.NumberofIRanges = TabRangeI.NbRange;
		for(i=0;i<TabRangeI.NbRange;i++)
			Board->parameter.IRangesTable[i] = TabRangeI.rg[i].Value;

		Board->parameter.NumberofVRanges = 0;
	}
	else
	{
		// auxiliary regulation : ALL the parameters are in the source
		Board->parameter.NumberofIRanges = 0;
		Board->parameter.NumberofVRanges = 2;
#define _VRANGE_REGULAUX 10
		Board->parameter.VRangesTable[0] = _VRANGE_REGULAUX;
		Board->parameter.VRangesTable[1] = 0;
	}
	return _RETURN_OK ;
}				// FIN InitBoard_Regul()
// ****************************************************************


static double Watt2Ampere(double x,AREGUL *Regul)
{	
	ACHANNEL *Channel=Regul->ChannelOut;

	if (Channel->parameter.Etalon)
	{
		if ((Channel->parameter.Etalon)(&x))
		{
			Channel->mes.Status |= _BIT_CONVERSION_ERROR;
			return 0.0; // Conversion not possible : force 0.0
		}
		else
			return x;
	}
	else
	{
		x = x<0 ? 0 : x;
		return sqrt(x/Regul->parameter.HeatingResistor); // SetRegul has already verified that Resitor!=0
	}
}				// FIN Watt2Ampere()
// ****************************************************************

static double Watt2Volt(double x,AREGUL *Regul)
{	
	ACHANNEL *Channel=Regul->ChannelOut;

	if (Channel->parameter.Etalon)
	{
		if ((Channel->parameter.Etalon)(&x))
		{
			Channel->mes.Status |= _BIT_CONVERSION_ERROR;
			return 0.0; // Conversion not possible : force 0.0
		}
		else
			return x;
	}
	else
	{
		x = x<0 ? 0 : x;
		return sqrt(x*Regul->parameter.HeatingResistor)*(1+1000/Regul->parameter.HeatingResistor); // SetRegul has already verified that Resitor!=0
	}
}				// FIN Watt2Volt()
// ****************************************************************

static int Check_Regul_Interne(AREGUL *Regul)
{
	double Im,Vm;
	int BestRange,i;
	ABOARD *Board ;
	PRIVATEMEMORY_REGUL *pvmm;
	ACHANNEL *Channel = Regul->ChannelOut;

	Board = (ABOARD *)Regul->Board;
	pvmm=Board->PrivateMemory;
	// test that for the maximum admissible value HeatingMax, the current
	// is smaller that maximum whih depends on the Range

	Im = Watt2Ampere(Regul->parameter.HeatingMax,Regul);
	Vm = Regul->parameter.HeatingResistor * Im;	// Vm is in Volts
	
	// Determine the maximum value for each range
	for (i=0;i<TabRangeI.NbRange;i++)
	{
		switch (Regul->HeatingConfig)
		{
		case  _NOBOOSTER_RETURN_TO_0:
		case _BOOSTER_RETURN_TO_0:
			pvmm->TabVMax[i] = _VMAX_RETURN0_WITHOUT_BOOSTER;
			break;
		case _NOBOOSTER_RETURN_TO_15:
		case  _BOOSTER_RETURN_TO_15: 
			pvmm->TabVMax[i] = _VMAX_RETURN15_WITHOUT_BOOSTER;
			break;
		default:return _INTERNAL_INCONSISTENCY;
		}
	}
	// Special case of the Booster
	switch (Regul->HeatingConfig)
	{
	case  _NOBOOSTER_RETURN_TO_0:
	case _NOBOOSTER_RETURN_TO_15:
		pvmm->TabVMax[_BOOSTER_RANGE] = 0;
		break;
	case _BOOSTER_RETURN_TO_0:
	case  _BOOSTER_RETURN_TO_15: 
		pvmm->TabVMax[_BOOSTER_RANGE] = _VMAX_WITH_BOOSTER;
		break;
	default:return _INTERNAL_INCONSISTENCY;
	}


	for (i=0;i<TabRangeI.NbAutoRange;i++)
	{
		if ((pvmm->TabVMax[i]>Vm) && (TabRangeI.rg[i].Value>Im))
			break;
	}

	if (i >= TabRangeI.NbAutoRange)
		return _HEATINGMAX_TOO_LARGE;

	BestRange = i;

	if (Channel->parameter.Mode == _FIX_RANGE_MODE)
	{
		if (Channel->NumRangeI < BestRange)
			return _HEATINGMAX_TOO_LARGE;
	}

	if ((Channel->parameter.Mode == _SPECIAL_MODE)
		&&(Channel->parameter.BoardType == _REGULMAINBOARD))
	{
		Channel->NumRangeI = BestRange;
		SetRangeAndData(&TabRangeI,_RANGEBYNUMBER,
					&Channel->NumRangeI,
					&Channel->parameter.ValueRangeI,
					(int *)0);
	}

	return _RETURN_OK ;

}				// FIN Check_Regul_Interne()
// ****************************************************************

int Check_Regul(AREGUL *Regul)
{
	int i,st;
	double R,Pmax;
	double TicLength = 45e-3;
	// 45e-3 is a common approximated value of the tic for both frequency 50Hz and 60 Hz

	// translate the parmeters given to paremeters used

	Regul->I = Regul->parameter.I * TicLength;
	Regul->P = Regul->parameter.P;
	Regul->D = Regul->parameter.D / TicLength;

	// Build the parameters alpha and beta from P I and D
	if (fabs(Regul->D) < _ZERO)
	{
		// No filtering
		Regul->alpha =  0.0;
		Regul->beta =  0.0;
	}
	else
	{
		// evaluate alpha and beta
		Regul->alpha =  1.0 - (_DERIVATIVE_INTENSITY ) / Regul->D;
		Regul->beta =  1.0 - (_DERIVATIVE_INTENSITY * _DERIVATIVE_INTENSITY ) / Regul->D ;
	}
	Regul->alpha = Intervalle(0,_ALPHA_MAX_REGUL,Regul->alpha);
	Regul->beta = Intervalle(0,_ALPHA_MAX_REGUL,Regul->beta);

	if (Regul->parameter.Index == _MAIN_REGULATION)
	{
		// Main Regulation
		for (i=0;i<1000;i++)
		{
			st = Check_Regul_Interne(Regul);
			if (st != _HEATINGMAX_TOO_LARGE)
				return st;
			Regul->parameter.HeatingMax *= 0.99;
		}
		return st;
	}

	// auxiliary regulation
	R = Regul->parameter.HeatingResistor;
	if ((R+1000)*_MAX_CURRENT_DACB > _MAX_VOLTAGE_DACB) // because there is a 1000 ohms serial resistor
		Pmax = _MAX_VOLTAGE_DACB*_MAX_VOLTAGE_DACB*R/((R+1000)*(R+1000));
	else
		Pmax = _MAX_CURRENT_DACB*_MAX_CURRENT_DACB*R;

	if (Regul->parameter.HeatingMax > Pmax)
		Regul->parameter.HeatingMax = Pmax;

	return _RETURN_OK ;

}				// FIN Check_Regul()
// ********************************************************************


long int CheckChannel_Regul(ACHANNEL *Channel)
// allowed mode = _NOT_USED_MODE _FIX_RANGE_MODE _FIX_VOLTAGE_MODE
// the parameter Channel->ValueRangeV is fitted in [-10,10] this
// is the value in Volts to impose on the dacB
//
// reduit le HeatingMax pour le rendre compatible avec le channel out
// de la regulation et la configuration de la regulation 
{
	int k;

	if (Channel->parameter.SubAddress >= _NB_SUBADD_REGUL)
		return _INVALID_SUBADDRESS;

	if (Channel->parameter.BoardAddress == _REGULMAINADDRESS)
	{
		// This a the main regulation
		//				Check mode
		// all modes in case are ok, default is not;
		switch (Channel->parameter.Mode)
		{
		case _INIT_MODE:
			Channel->NumRangeI = 0;
			Channel->NumRangeV = 0;
			Channel->parameter.PreAveraging = _INIT_PREAVERAGING_REGUL;
			Channel->parameter.ValueRangeV = 0;
			Channel->parameter.ValueRangeI = _INIT_VALUERANGE_REGUL;
			Channel->parameter.FifoSize = _FIFOLENGTH; // unusefull for board B

			Channel->parameter.Mode = _INIT_MODE_REGUL;
		case _FIX_RANGE_MODE:
		case _FIX_VOLTAGE_MODE:
		case _SPECIAL_MODE:
		break;
		case _NOT_USED_MODE:
		default:
			return _INVALID_MODE; 
		}

		if (Channel->parameter.ValueRangeV != 0)
			return _INVALID_CHANNELPARAMETER;

		k = SetRangeAndData(&TabRangeI,_RANGEBYVALUE,
			&Channel->NumRangeI,&Channel->parameter.ValueRangeI,0);
		if (k<0) 
			return _RANGE_CHANGE_NOT_POSSIBLE;
		else if (k>0)
			return _CHANNEL_HAS_BEEN_MODIFIED;
		else
			return _RETURN_OK;
	}
	else
	{
		// This is the auxilliary regulation
		switch (Channel->parameter.Mode)
		{
		case _INIT_MODE:
			Channel->NumRangeI = 0;
			Channel->NumRangeV = 0;
			Channel->parameter.PreAveraging = _INIT_PREAVERAGING_REGUL;
			Channel->parameter.ValueRangeV = _MAX_VOLTAGE_DACB;
			Channel->parameter.ValueRangeI = 0;
			Channel->parameter.FifoSize = _FIFOLENGTH; 

			Channel->parameter.Mode = _INIT_MODE_REGUL;
		case _FIX_RANGE_MODE:
		case _SPECIAL_MODE:
			break;
		default:
			return _INVALID_MODE; 
		}

		if (Channel->parameter.ValueRangeI != 0)
			return _INVALID_CHANNELPARAMETER;

		return _RETURN_OK;
	} // FIN auxilliary regulation
}				// FIN CheckChannel_Regul()
// ****************************************************************

#define _BEGIN	0
#define _END	(_WAIT_BEFORE_CALIB_REGUL-1)
#define _KEEP_GOING	1

void Calibrate_Regul(ABOARD *Board)
{
	/* 
		mettre sur la gamme OPEN (ValueRangeI=0)
	    attendre un delai en dur
	    trouver la valeur a mettre dans le dac de la regul telle quz*
	   
	*/
	PRIVATEMEMORY_REGUL *pvmm = Board->PrivateMemory; 
	short s;
	double v;
	int index,state,n;

	index = pvmm->Cpt/_WAIT_BEFORE_CALIB_REGUL;
	state = (pvmm->Cpt++)%_WAIT_BEFORE_CALIB_REGUL;

	if (state == _BEGIN)
	{
		if (index == 0)			// Special case to test lowest value
		{
			pvmm->xi = _START_VALUE_CALIB_MIN;
			pvmm->x = pvmm->xi;
		}
		else if (index == 1)	// Special case to test largest value
		{
			pvmm->xa = _START_VALUE_CALIB_MAX;
			pvmm->x = pvmm->xa;
		}
		else					// Regular case
			pvmm->x = (pvmm->xi + pvmm->xa)/2;

		// set the data to send
		v = 0;
		n = 0;
		SetRangeAndData(&TabRangeI,_RANGEBYVALUE,&s,&v,&n);
		pvmm->DataToSend = (n & 0xffff0000) | (pvmm->x & 0x0000ffff);
	}
	else if (state == _END)
	{
		// Read the value of y for the current value of x
		pvmm->y = Board->Over;

		// find the new inteerval with both end different
		if (index == 0)			// Special case to test lowest value
		{
			pvmm->yi = pvmm->y;
		}
		else if (index == 1)	// Special case to test largest value
		{
			pvmm->ya = pvmm->y;
			if (pvmm->yi == pvmm->ya)
			{
				// CALIBRATION NOT POSSIBLE SINCE BOTH VALUE EQUAL
				// Leave the okd value of offset
				Board->parameter.CalibrationStatus = _CALIBRATION_FAILED;
			}
				
		}
		else					// Regular case  ie not i==0 or i==1
		{
			if (pvmm->y == pvmm->yi)
				pvmm->xi = pvmm->x;
			else if (pvmm->y == pvmm->ya)
				pvmm->xa = pvmm->x;
			else
				for(;;);

			if (pvmm->xa ==  pvmm->xi+1) 
			{
				// SUCCESS THE OFFSET HAS BEEN FOUND !!!!
				Board->Offset = pvmm->x;
				Board->parameter.CalibrationTable[0] = pvmm->x; // to facilitate the lecture
				Board->parameter.CalibrationStatus = _NORMAL_MODE;
			}

		}		//  end state == end and index >=2
	}			// end state == end
	
	Board->Data =  pvmm->DataToSend;

	return ;

}				// FIN Calibrate_Regul()
// ****************************************************************


long int Calc_Regul(ABOARD *Board)
// Board->Channels[0] is the channels used to store the value used for
// heatuing. and Board->Channels[i] for i=1,2,3,4 point to the channel
// to use to regulate

// consigne en kelvin HeatingMax en Watt
// P = Watt/kelvin
// I = 1/seconde
// D = seconde
// Gain (fourni par le channel regul) par def y=sqrt(x)/R convertit W en A
//
// controle 1 : verifier que pour la puissance maximum (HeatingMax) 
//				le courant est inferieur au maxi qui depend de la gamme
//				et la tension = R * Courant cree par puiss max pour la gamme
//				consideree si mode fixe, la plis grande si mode variable
//				Ce test est fait par SetRegul.
// controle 2 : 
{
	int i,n;
	short flag;
	double heating,dx,Pdxmin,Pdxmax,w,wi,derive,Pdx;
    double OutCurrent = 0,OutVoltage = 0;  /* avoid warning */
	ACHANNEL *Channel;
	AREGUL *Regul = Board->Regulation;
	ACHANNEL *ChannelOut = Regul->ChannelOut;
	REGULPARAMETER *Regulationp;
	PRIVATEMEMORY_REGUL *pvmm = Board->PrivateMemory; 
	int RegulPrinc;
	double P,I,D;
	
    ChannelOut->mes.Status = 0;

	Regulationp = &Regul->parameter;

	RegulPrinc = (Regul->parameter.Index == _MAIN_REGULATION) ? 1 : 0 ;

	if (RegulPrinc)
	{
		if (Board->parameter.CalibrationStatus == _START_CALIBRATION_MODE)
		{
			Board->parameter.CalibrationStatus = _CALIBRATION_MODE;
			pvmm->Cpt = 0;
		}
		if (Board->parameter.CalibrationStatus == _CALIBRATION_MODE) 
		{
			Calibrate_Regul(Board);
			return _RETURN_OK;
		}
	} // FIN RegulPrinc
	else
	{
		// Ici on traite la regulution auxilliaire:cas particulier 
		ACHANNEL *ChannelOut = Board->Channels[Board->ChannelTreated];
		// channel de sortie de la regu
		
		if ((ChannelOut->parameter.Mode == _SPECIAL_MODE)
			&&(ChannelOut->parameter.BoardType == _REGULAUXBOARD))
			 // le 2eme test est inutile mais pour lisibilite on le laisse
		{
			// Before jumping :
			// set i)OutVoltage, ii) heating iii)Board->Status and
			// ChannelOut->OldMeasureTime  ChannelOut->NumRangeIOld ChannelOut->NumRangeVOld
			OutVoltage = Intervalle(-ChannelOut->parameter.ValueRangeV,
									 0.9999*ChannelOut->parameter.ValueRangeV,
									 Regulationp->SetPoint);
			
			//heating = OutVoltage*OutVoltage/Regul->parameter.HeatingResistor; 
			heating = OutVoltage;
			Board->Over = 0;
			Board->Status = 0;
			ChannelOut->OldMeasureTime = ChannelOut->mes.Time;
			ChannelOut->NumRangeIOld = ChannelOut->NumRangeI;
			ChannelOut->NumRangeVOld = ChannelOut->NumRangeV;
			goto RegulAuxUtiliseeEnOutput; // one skips auxilliary calculation !!
		}
	}

	// the WriteFifo will accumulate both raw and converted values:
	ChannelOut->ConversionDone = 2; 

	// Check if the accumulator has to be reset
	if (Regul->ForceAccTo0)
	{
		pvmm->Accumulator = 0;
		pvmm->Filter1Dx = 0;
		pvmm->Filter2Dx = 0;

		Regul->ForceAccTo0 = 0;
	}

	P = Regul->P;
	I = Regul->I;
	D = Regul->D;
	if (P <= 0) 
    {
		// FIXED VALUE ie no regulating  
		if (P == 0)
			pvmm->Accumulator = Regulationp->SetPoint;// Manual power
		P = 0 ;
		pvmm->WatchDog = _WATCHDOG_REGUL_VALUE;
		pvmm->Filter1Dx = 0;
		pvmm->Filter2Dx = 0;
		dx = 0;
		D = 0;
	}
	else
	{
		//REGULATION MODE with up to 4 channels
		dx = 0;
		w = 0;
		for(i=0;i<_NB_REGULATING_CHANNEL;i++)
		{
			int notok;
			Channel = Regul->Channels[i];

			if ( !Channel )
				continue;		// no channel

			wi = Regulationp->WeightofChannel[i];

			if (wi==0)
				continue;

			if (Channel->mes.Status & (_BIT_RECENT_CHANGE))
				continue; // the value is not correct 

			// here a valid channel in used is found
			// Get the value and correct it Translate resistor in K
			if (RegulPrinc)
			{
				// bit 2 for regul princ
				notok = Channel->ConversionDone & 2;
				Channel->ConversionDone |=  2;// set bit 2 for regul princ
			}
			else
			{
				// bit 4 for regul aux
				notok = Channel->ConversionDone & 8;
				Channel->ConversionDone |=  8;// set bit 4 for regul aux
			}
			// here notok is either 2 or 8
			if (notok)
				continue;
			// if not zero calcboard convert data and decrement 

			// Channel->mes.Measure is valid in K
			dx += (Regulationp->SetPoint - Channel->mes.Measure)*wi;
			w += wi;
		} // end loop aover regualting channel
	
		// Filtering the dx using Derivative coefficient (D=0==> No filtering)
		pvmm->Filter1Dx  = Regul->alpha * pvmm->Filter1Dx
								+ (1-Regul->alpha)*dx;
		pvmm->Filter2Dx  = Regul->beta * pvmm->Filter2Dx
								+ (1-Regul->beta)*pvmm->Filter1Dx;
		dx = pvmm->Filter2Dx;

		if (w == 0)
		{	// One cannot regulate since no correct measures
			if (pvmm->WatchDog)
				pvmm->WatchDog--;
		}
		else
			pvmm->WatchDog = _WATCHDOG_REGUL_VALUE;
	}	// else mode fixe

	// convert dx in Watt:
	Pdx = P*dx;

	if (pvmm->WatchDog)
	{
		// Accumulator contains avraeged heating in Watt
		// dx contains if the spacing (error) in Watt

		// force Accumulator to be in [0,HeatingMax]:
		pvmm->Accumulator = 
			Intervalle(0,Regulationp->HeatingMax,pvmm->Accumulator);

		// heating = (acc + I * Pdx) + Pdx + D *(Pdx-LastPdx)
		// dxmin = dx such that heating = 0 
		// Pdxmin = -(pvmm->Accumulator)/(Regulationp->I + 1);
		Pdxmin = (D*pvmm->LastPdx - pvmm->Accumulator)/(I + 1 + D);

		// dxmax = dx such that heating = max value
		// Pdxmax = (Regulationp->HeatingMax-pvmm->Accumulator)/(Regulationp->I+1);
		Pdxmax = (Regulationp->HeatingMax  + D*pvmm->LastPdx- pvmm->Accumulator)
				/(I + 1 + D);

		// force dx in the interval
		Pdx = Intervalle(Pdxmin,Pdxmax,Pdx);

		// the derivative has been possibli modified by the last line, recompute it.
		derive = Pdx - pvmm->LastPdx; // derive in tics unit 

		pvmm->Accumulator += I * Pdx;
		// is in the interval by construction

		heating = pvmm->Accumulator + Pdx + D * derive;
			// is in the interval by construction
	}
	else
	{	
	// here a too big heating is asked for a long time : something wrong turn regul off.
	heating = 0;
	pvmm->Accumulator = 0;
	ChannelOut->mes.Status = _BIT_REGUL_CLEARED;
	} // if (pvmm->WatchDog)	

	pvmm->LastPdx = Pdx;	// store the value for the next time

	
	// At this point heating is the value for heating 
	// This is DIFERENT from standard board since the value is accumulated
	// the status and range are set NOW and not after reading at the next tick
	// convert the heating (Watt) in current (A)

	if (heating < 1e-15)
		// if the heating too small heating = 0 to avoid no significativ value. 
		heating = 0;

	if (RegulPrinc)
		OutCurrent = Watt2Ampere(heating,Regul);
	else
		OutVoltage = Watt2Volt(heating,Regul);

	// **************************************************************
	//		        PREPARE THE DATA TO SEND ie FIND THE NEW RANGES
	// **************************************************************
	Board->Status = 0; // bit corresponding to diff. possible pbs will be set
	// value to send = heating 

	ChannelOut->OldMeasureTime = ChannelOut->mes.Time;
	ChannelOut->NumRangeIOld = ChannelOut->NumRangeI;
	ChannelOut->NumRangeVOld = ChannelOut->NumRangeV;

	Board->Data = 0;
	if (RegulPrinc)
	{
		n = (int)((-32768.0*OutCurrent)/ChannelOut->parameter.ValueRangeI);
		flag = _RANGEBYVALUE;

		if (ChannelOut->parameter.Mode == _FIX_VOLTAGE_MODE)
		{
			if (_ABS(n) < _FLOOR_REGUL) 
			{
				Board->RangeFirst = 1;
				flag = _RANGEMINUS;
			}
			else if (_ABS(n) > _CEIL_REGUL)
			{
				Board->RangeFirst = 0;
				flag = _RANGEPLUS;
			}
		}

		SetRangeAndData(&TabRangeI,flag,
						&ChannelOut->NumRangeI,
						&ChannelOut->parameter.ValueRangeI,
						&Board->Data);
		// correct if there is NO booster
		if ((ChannelOut->NumRangeI == _BOOSTER_RANGE) && (Regulationp->ThereIsABooster==0))
		{
			ChannelOut->NumRangeI--;
			SetRangeAndData(&TabRangeI,_RANGEBYNUMBER,
						&ChannelOut->NumRangeI,
						&ChannelOut->parameter.ValueRangeI,
						&Board->Data);
		}

		Board->NumRangeI = ChannelOut->NumRangeI;
		Board->NumRangeV = ChannelOut->NumRangeV;
		// 65536 -> parameter.ValueRangeI
		// data  -> heating
		n = (int)((-32768.0*OutCurrent)/ChannelOut->parameter.ValueRangeI);
		n += Board->Offset;		// To send 0 A, put Board->Offset in the DAC
		n = Intervalle(-32768,0,n);
		n &= 0x0000ffff;
		Board->Data &= 0xffff0000;
		Board->Data |= n;
	}
	else
	{
		// auxilliary regulation : 2 ranges 0 or _MAX_VOLTAGE_DACB
RegulAuxUtiliseeEnOutput:
		if (ChannelOut->parameter.ValueRangeV)
			n = (int)((32768.0*OutVoltage)/ChannelOut->parameter.ValueRangeV);
		else	
			n = 0;

		Board->Data = ((n<<16)&0xffff0000) + (n&0x0000ffff);
	}

	// **************************************************************
	//		        SEND THE VALUE IN THE FIFO
	// **************************************************************

	// status doit contenir boost mis dans overrange
	// le bit regulcleared
	// le bit no channel trouve
	// un channel au moins a ete saute a cause de soçn status
	// les bits habituels de changement de gamme icluding TOOSMALL AND TOOBIG
	if (Board->Over) 
		Board->Status |=_BIT_STATUS_OVERLOAD;
	ChannelOut->mes.Time = Board->Time;
	ChannelOut->mes.Status |=  Board->Status;
	if (RegulPrinc)
		ChannelOut->mes.MeasureRaw = OutCurrent;
	else
		ChannelOut->mes.MeasureRaw = OutVoltage;
	ChannelOut->mes.Measure = heating;
	// write to the fifo Without forcing stop accu
	WriteFifoMeasure(ChannelOut,0); 

	
	return _RETURN_OK ;
}				// FIN Calc_Regul()
// ****************************************************************
