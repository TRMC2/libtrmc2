/*
                          TrmcRunLib.c

  Everything NOT specific neither to a board type nor to a platform
*/
#ifdef powerc
#include "manip.h"
#else
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#ifndef __linux__
#include <malloc.h>
#include <windows.h>
#include <winsvc.h>
#define _MULTI_SYSTEM  // alllow NT family
#endif
#endif


#define _DLL

#include "Trmc.h"  
#include "TrmcRunLib.h" 
#include "TrmcDef.h"  
#include "TrmcProto.h" 
#include "TrmcPlatform.h" 
#include "TrmcRegul.h" 


// ****************************************************************
//                THE ONLY GLOBAL VARIABLE
// ****************************************************************
VARTRMC *vartrmc;

// ****************************************************************
//				DEFINITION OF EXPORTED FUNCTIONS
// ****************************************************************

int _DLLSTATUS StartTRMC(INITSTRUCTURE *init)
// VERY FIRST CALL:
// 1) allocate global memory(excluding the FIFOs)
// 2) find the parameters for transmission
// 3) find the boards, their types and address
// 4) proceed to various initializations
// 5) run the timer (unless *TimeToSend20DataInMs=_NOT_BEAT)
// SUBSEQUENT CALLS:
// if the timer is ruuning  return a positive (light) error and do nothing
// else                     run the timer (unless *TimeToSend20DataInMs=_NOT_BEAT) 
 
// return an error code
{
	int i,n100;

	if (vartrmc == 0) // ie very first call
	{
		// -------- MEMORY ALLOCATION + INITIALIZATION ------------
		// allocate the global memory, memory for variables
		// dedicated to a platform EXCLUDED
		i = TrmcAllocInit();
		if (i)
			return i; // could not allocate memory


		//  Verification of the validity of the parameter
		if (init->Com == _COM1)
		{
			vartrmc->com1 = _COM1;
		}
		else if (init->Com == _COM2)
		{
			vartrmc->com1 = _COM2;
		}
		else 
			return _INVALID_COM;

		if (init->Frequency == _50HZ)
			vartrmc -> periodinms = 40; // 40 milisecondes
		else if (init->Frequency == _60HZ)
			vartrmc -> periodinms = 50;// 50 milisecondes
		else if (init->Frequency == _NOTBEATING)
			vartrmc -> periodinms = 0;// 0 milisecondes
		else if ((init->Frequency & ~_MASK) == _KEY )// is it  a special period?
			vartrmc->periodinms = init->Frequency & _MASK;
		else 
			return _INVALID_FREQUENCY;

		// Platform initialisations
		i = InitPlatform(vartrmc);
		if (i)
			return i;

		// ----------- SET THE COMMUNICATION PARAMETERS -----------
		i = TrmcSetDelayComm(&n100); // n is the time to send 100 data
 		if (i)
			return i; // could not establish communication

	// ---------------- CONFIGURE -----------------------------
		i = TrmcConfig(); // Will detect AND initialize the boards
		if (i)
			return i;

	// -------- MAKE THE REGULATIONS ---------------------------
		MakeReguls();

	// -------- CLEAR THE FLAG TO DETECT IF THE TRMC HAS BEEN STOPPED
		vartrmc->comm_error_in_callback = 0; 
		vartrmc->calc_error_in_callback = 0;

	// --------- SET ALL THE POSSIBLE CHANNELS ---------------
		i = MakeAllChannels();
		if (i)
			return i;

	// -------- SYNCHRONIZE: COPY @ of Channnels and Board and Regul in one another
		SynchroRegulPointeur();

	// ---------------------- SET THE COMMUNICATION TIME
	// 4 calls to base in SynchroCall1 + 1+2*(board-2) in SynchroCall2
		init->CommunicationTime = (int)((2.0*vartrmc->NbofBoard+1.0)*n100/100.0);

	}  // END very first call

// --------------------- RUN THE TIMER -------------------- 
	if (!vartrmc->TimerRunning)
	{
		if (vartrmc->periodinms)
		{	
			vartrmc->NbCall = 0; 
			vartrmc->NbTics = 0; 
			i = BeatPlatform();	
			vartrmc->TimerRunning = 1;
		}
		else
			i = _RETURN_OK;

		if (i)
			return i;

		return  _RETURN_OK;
	}
	else
		return _TIMER_ALREADY_RUNNING;

}					// FIN StartTRMC()
// ***************************************************************

int _DLLSTATUS StopTRMC()
// Kill the periodic timer. 
// Its Id is found in the structure varspec
{
	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	// First stop the timer
	if (vartrmc->TimerRunning)
	{
		StopTimerPlatform();
		vartrmc->TimerRunning = 0;
	}
	else
		return _TIMER_NOT_RUNNING;

	return _RETURN_OK ;
}           // FIN StopTRMC()
 // ************************************************************

int _DLLSTATUS GetSynchroneousErrorTRMC(ERRORS *error)
// Return a negative code if the function GetSynchroneousErrorTRMC 
// has not been executed, and a zero or positive number if an error 
// has been reflected in the 2 first fields of the ERROR structure.
// The Commerror and CalcError fields contain the number of errors 
// SINCE the last call. whereas the TimerError is the number of lost 
// tics since the epoch.
//
// Return 0 if everything clear.
{
	//int st; mood du 21/10/2002

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	if (vartrmc->TimerRunning == 0 )
		 return _TIMER_NOT_RUNNING;

	// compute the return code
	//st = vartrmc->comm_error_in_callback+vartrmc->comm_error_in_callback;mod du 21/10/2002
	 
	// set the fields 
	error->CommError = vartrmc->comm_error_in_callback;
	error->CalcError = vartrmc->calc_error_in_callback;
	error->TimerError = (vartrmc->NbTics - vartrmc->NbCall);
	error->Date = vartrmc->NbTics;

	// clear the 2 first fields
	vartrmc->comm_error_in_callback = _RETURN_OK;
	vartrmc->calc_error_in_callback = _RETURN_OK;

	return _RETURN_OK;
}			// FIN GetSynchroneousErrorTRMC()
//**************************************************************

void _DLLSTATUS *SecretTRMC()
{
	return vartrmc;
}			// FIN Secret()
//**************************************************************


// ************************************************************
//				MANIPULATING CHANNELS
// ************************************************************
int _DLLSTATUS GetNumberOfChannelTRMC(int *n)
// Get the number of channel in n and return an error code
// if an errot occurs *n = -1;
{
	*n =-1;
	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	*n = vartrmc -> NbofChannel;

	return _RETURN_OK;
}           // FIN GetNumberOfChannelTRMC()
 // ************************************************************

int _DLLSTATUS SetChannelTRMC(CHANNELPARAMETER *channel)
//		To create or change one or several parmeters of a channel.
//		according to the channel *channel.
//      The fields Address, SubAddress, BoardType and Index HAVE to match the
//	    corresponding fields in the table
{
	ACHANNEL localchannel;
	int st,i,ChangeInFifoSize;

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	if ((channel->Index<0)||(channel->Index>=vartrmc->NbofChannel))
			return _NO_SUCH_CHANNEL;
	if (vartrmc->Channels[channel->Index].parameter.BoardAddress != channel->BoardAddress)
			return _NO_SUCH_CHANNEL;
	if (vartrmc->Channels[channel->Index].parameter.SubAddress  != channel->SubAddress)
			return _NO_SUCH_CHANNEL;
	if (vartrmc->Channels[channel->Index].parameter.BoardType != channel->BoardType)
			return _NO_SUCH_CHANNEL;

	// prepare a channel to test if CheckChannelBoard is ok before copying
	localchannel = vartrmc->Channels[channel->Index];

	// is the associated board is in calibration mode
	for (i=0;i<vartrmc->NbofBoard;i++)
		if (vartrmc->board[i].parameter.AddressofBoard == channel->BoardAddress)
			break;
	if (i==vartrmc->NbofBoard)
		return _INTERNAL_INCONSISTENCY;

	if (vartrmc->board[i].parameter.CalibrationStatus == _CALIBRATION_MODE)
		return _BOARD_IN_CALIBRATION;

	ChangeInFifoSize = (localchannel.parameter.FifoSize != channel->FifoSize);
	
	localchannel.parameter = *channel;
	localchannel.ChangedRequired = 1;

	st = (vartrmc->CheckChannelofBoard[channel->BoardType])(&localchannel);
	// st is the status : flag if something has been modified
	if (st<0)       //  A too bad error occurs : NO change done
		return st;

	// now check the fifo
//	if (localchannel.parameter.FifoSize != channel->FifoSize)
	if (ChangeInFifoSize)
	{
		AMEASURE *save = vartrmc->Channels[channel->Index].fifopt->data,*pt;

		// prevent to write in the fifo:
		vartrmc->Channels[channel->Index].fifopt->data = 0;

		// try to re-allocate
		int allocatedSize = channel->FifoSize + 1;
		pt = save;
#ifdef powerc
		free(pt);
		pt = (AMEASURE *)malloc(allocatedSize*sizeof(AMEASURE));
#else
		pt = (AMEASURE *)realloc(pt,allocatedSize*sizeof(AMEASURE));
#endif
		if (pt==0)
		{
			// unsuccessfull : restore old value
			vartrmc->Channels[channel->Index].fifopt->data = save;
			return _CANNOT_ALLOCATE_MEM;
		}
		// succesfull keep it
		vartrmc->Channels[channel->Index].fifopt->Size = allocatedSize;
		vartrmc->Channels[channel->Index].fifopt->data = pt;
		vartrmc->Channels[channel->Index].fifopt->iRead = 0;
		vartrmc->Channels[channel->Index].fifopt->iWrite = 0;
		vartrmc->Channels[channel->Index].fifopt->AverageCounter = 0;
		vartrmc->Channels[channel->Index].fifopt->Hit = 0;
		localchannel.parameter.FifoSize = channel->FifoSize;	
	}

	// ScrutatioTime==0 NEEDS a piorityflag=_NO_PRIORITY
	if ((localchannel.parameter.ScrutationTime==0) && 
		(localchannel.parameter.PriorityFlag!=_NO_PRIORITY))
		return _NO_PRIORITY_WITH_ZERO_SCRUTATION;


	// copy th channel localchannel in the table since it is OK
	vartrmc->Channels[channel->Index] = localchannel;

	// here the set has been accepted, set PRIORITY and ALWAYS of the corresponding board
	{
		// new method >  26 fev 2002
		ABOARD *Board;
		int i,j;

		for (i=_FIRSTBOARD;vartrmc->NbofBoard;i++)
		{
			Board = &vartrmc->board[i];
			for (j=0;j<Board->NumberofChannels;j++)
			{
				if (Board->Channels[j] == &vartrmc->Channels[channel->Index])
					break;
			}

			if (j != Board->NumberofChannels)
			{
				int k;
				// The board corresponding to this channel has been found
				// it will be set anayway 
				if (Board->ChannelPriority == j)
					Board->ChannelPriority =-1;
				if (Board->ChannelAlways == j)
					Board->ChannelAlways =-1;

				switch (localchannel.parameter.PriorityFlag)
				{
				case _PRIORITY:
					// if a channel was already in PRIORITY mode : turn it to normal
					for (k=0;k<Board->NumberofChannels;k++)
					{
						if (Board->Channels[k]->parameter.PriorityFlag == _PRIORITY)
						{
							Board->Channels[k]->parameter.PriorityFlag = _NO_PRIORITY;
							Board->ChannelPriority = -1; // any negative number means no channel	
						}
					}
					Board->ChannelPriority = j;
					vartrmc->Channels[channel->Index].parameter.PriorityFlag = _PRIORITY;
					break;
				case _ALWAYS:
					// if a channel was already in ALWAYS mode : turn it to normal
					for (k=0;k<Board->NumberofChannels;k++)
					{
						if (Board->Channels[k]->parameter.PriorityFlag == _ALWAYS)
						{
							Board->Channels[k]->parameter.PriorityFlag = _NO_PRIORITY;
							Board->ChannelAlways = -1; // any negative number means no channel
						}
					}
					Board->ChannelAlways = j;
					vartrmc->Channels[channel->Index].parameter.PriorityFlag = _ALWAYS;
					break;
				default:
					if (Board->ChannelPriority == j)
						Board->ChannelPriority = -1; // any negative number means no channel
					if (Board->ChannelAlways == j)
						Board->ChannelAlways = -1; // any negative number means no channel
					break;
				}
				break;
			}
		}
	}  // bloc servant uniquement a declarer une variable locale
	// copy in channel the EXACT channel parameter of the table
	*channel = vartrmc->Channels[channel->Index].parameter;

	// if the channel is a output channel of a regul call check regul
	if (channel->BoardType == _TYPEREGULMAIN)
	{
		REGULPARAMETER Regul;
		Regul.Index = 0;
		GetRegulationTRMC(&Regul);
		if ((st = SetRegulationTRMC(&Regul))<0)
			return st;

	}
	return st;
}				// FIN SetChannelTRMC()
//************************************************************* 

int _DLLSTATUS GetChannelTRMC(int bywhat,CHANNELPARAMETER *channel)
// To get the parameters of a channel from the channel table.

// if bywhat = _BYADDRESS the channel with this address is
//			searched in the channel table and copied into the 
//			structure pointed to by channel.

// if bywhat = _BYINDEX the channel with index channel->Index is
//			searched in the channel table and copied into the 
//			structure pointed to by channel.

// An error code is returned if no corresponding channel is 
// found in the table.
{
	int i;
	ACHANNEL *pt;

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	switch (bywhat)
	{
	case  _BYADDRESS:
		channel->Index = -1;	// To generate an error if @ not found
		for (i=0;i<vartrmc->NbofChannel;i++)
		{
			pt = &(vartrmc->Channels[i]);
			if (pt->parameter.BoardAddress != channel->BoardAddress)
				continue; // not the searched address
			if (pt->parameter.SubAddress != channel->SubAddress)
				continue; // not the searched sub-address
			channel->Index = i;
			break;
		}
	case _BYINDEX:
		if ((channel->Index<0)|| (channel->Index>=vartrmc->NbofChannel))
			return _NO_SUCH_CHANNEL;
		break;
	default: 
		return _INVALID_BYWHAT;
	}

	*channel = vartrmc->Channels[channel->Index].parameter ;

	return _RETURN_OK;
}				// FIN GetChannelTRMC()
//************************************************************* 



// ************************************************************
//				MANIPULATING REGULATION (2 FUNCTIONS)
// ************************************************************
//				ONLY ONE REGULATION CHANNEL IS ALLOWED
int _DLLSTATUS SetRegulationTRMC(REGULPARAMETER *RegulGiven)
// To set up the regulation parameters AND the associated channel.

// If no regulation channel exists then one is created with
//		the parameter from regulparameter, MaxHeating is ValueRangeV
// if ONE regulation channel exists  this channel is MODIFIED
//		according to the parameter.
// if TWO OR MORE regulation channel exist an error is returned
//      and nothing is done. 
// The Board->Channel[0] is the actual channel
// Board->Channels[i], i=1..4 are the four regulating channel except
// if Board->Channels[1] == -1 which means constant value

// controle 1 : verifier que pour la puissance maximum (maxheating) 
//				le courant est inferieur au maxi qui depend de la gamme
//				et la tension = R * Courant cree par puiss max pour la gamme
//				consideree si mode fixe, la plis grande si mode variable

{
	// il faut que RegulOutput(x) fonction croissante de x
	// donne les pointeurs sur ces canaux dans le CHANNEL de la RegulGiven
	AREGUL *RegulInMem;
	AREGUL Regul;
	int i,warning,k,notfake;

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	// This is where the regualtion is choosen
	switch (RegulGiven->Index) 
	{
	case  _MAIN_REGULATION :	
		RegulInMem = &vartrmc->Regulation0;break;
	case  _AUXILIARY_REGULATION :	
		RegulInMem = &vartrmc->Regulation1;
		if (RegulGiven->ThereIsABooster)
			return _INVALID_REGULPARAMETER;
		break;
	default:
		return _NO_SUCH_REGULATION;
	}

	// Test a positive Resistor
	if (RegulGiven->HeatingResistor<=0)
		return _INVALID_REGULPARAMETER;

	// Do a copy to test
	Regul = *RegulInMem;
	Regul.parameter = *RegulGiven;

	// Test that all channels are valid
	warning = _RETURN_OK;
	if (Regul.parameter.P != 0)
	{
		// Real regualtion  (not a fixed cconsigne)
		for (notfake=0;notfake<=1;notfake++)
		{
			// notfake = 0 for testing if will be accepted
			// fakr = 1 for actual changes
			if (notfake)
			{
				// Initialize all the Channels to 0, will be possibly chaged
				for(i=0;i<_NB_REGULATING_CHANNEL;i++)
					Regul.Channels[i] = (ACHANNEL *)0;
			}

			for(k=i=0;i<_NB_REGULATING_CHANNEL;i++)
			{
				// if channel valid put the address of channel
				// it is not possible to regulate on channel 0 or 1 since they are reguls
				if (Regul.parameter.IndexofChannel[i] == _EMPTY_CHANNEL) 
					continue;  // not used channel the value 0 is alerady set
				if ((Regul.parameter.IndexofChannel[i]>=2) && 
					(Regul.parameter.IndexofChannel[i]<vartrmc->NbofChannel))
				{
					k++;
					if (notfake)
					{
						Regul.Channels[i] =
							&vartrmc->Channels[Regul.parameter.IndexofChannel[i]];
					}
				}
				else
					return _INVALID_REGULPARAMETER;
					// The RegulGiven given in parameter is correct: do the setting
			} // end loop over regulating channels
			if(!k) // No regulating channel
				return _INVALID_REGULPARAMETER;
		}// END FAKE
	}
	// synchronise the HeatingConfig
	if (Regul.parameter.ThereIsABooster == _YES)
	{
		if (Regul.parameter.ReturnTo0 == _YES)
			Regul.HeatingConfig = _BOOSTER_RETURN_TO_0;
		else if (Regul.parameter.ReturnTo0 == _NO)
			Regul.HeatingConfig = _BOOSTER_RETURN_TO_15;
		else if (Regul.parameter.ReturnTo0 == _AUTOMATIC)
		{
			if (Regul.parameter.HeatingResistor <= 28)
				Regul.HeatingConfig = _BOOSTER_RETURN_TO_0;
			else
				Regul.HeatingConfig = _BOOSTER_RETURN_TO_15;
		}
		else
			return _INVALID_REGULPARAMETER;
	}
	else if (Regul.parameter.ThereIsABooster == _NO)
	{
		if (Regul.parameter.ReturnTo0 == _YES)
			Regul.HeatingConfig = _NOBOOSTER_RETURN_TO_0;
		else if (Regul.parameter.ReturnTo0 == _NO)
			Regul.HeatingConfig = _NOBOOSTER_RETURN_TO_15;
		else if (Regul.parameter.ReturnTo0 == _AUTOMATIC)
		{
			if (Regul.parameter.HeatingResistor <= 28)
				Regul.HeatingConfig = _NOBOOSTER_RETURN_TO_0;
			else
				Regul.HeatingConfig = _NOBOOSTER_RETURN_TO_15;
		}
		else
			return _INVALID_REGULPARAMETER;
	}
	else 
		return _INVALID_REGULPARAMETER;

	// verify that the channel and the regul together make sense
	if ( (i = Check_Regul(&Regul))!=0)
		return i;

	// Regul is OK
	// test if one goes from fixed target to regulate on channels
	if ((RegulInMem->parameter.P == 0) && (Regul.parameter.P > 0))
		// one was heating according to a FIXED value and one will regulate
		// P<0 means holding the power
		Regul.ForceAccTo0 = 1;
	else
		Regul.ForceAccTo0 = 0;

	// copy now the given regulparameter in the table
	*RegulInMem = Regul;
	// to synchronise the actual paremeter with the returned values
	*RegulGiven = Regul.parameter;
	return warning;

}			// FIN SetRegulationTRMC()
//************************************************************* 

int _DLLSTATUS GetRegulationTRMC(REGULPARAMETER *Regul)
// To Get the regulation parameter.
// if ONE regulation channel exists, 
//     this channel is set in the parameter
// else an error is returned
{

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	// This is where the regualtion is choosen
	switch (Regul->Index) 
	{
	case  _MAIN_REGULATION :	
		*Regul = vartrmc->Regulation0.parameter;break;
	case  _AUXILIARY_REGULATION :	
		*Regul = vartrmc->Regulation1.parameter;break;
	default:
		return _NO_SUCH_REGULATION;
	}

	return _RETURN_OK;
}			// FIN GetRegulationTRMC()
//************************************************************* 
 
// ************************************************************
//				MANIPULATING BOARDS
// ************************************************************
int _DLLSTATUS GetNumberOfBoardTRMC(int *n)
// Get the number of board in n and return an error code
// if an errot occurs *n = -1;
{
	*n = -1;
	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	if (vartrmc->comm_error_in_callback)		// Probably turned off
	{
		vartrmc->comm_error_in_callback = 0; // Clear the flag
		return _COMM_NOT_ESTABLISH;		// Return erroe
	}

	*n = vartrmc -> NbofBoard;
	 return _RETURN_OK;
}           // FIN GetNumberOfBoardTRMC()
 // ************************************************************

int _DLLSTATUS GetBoardTRMC(int bywhat,BOARDPARAMETER *boardparam)
// To get the parameter associated to a board.

// boradparam points to a BOARDPARAMETER struct.
// bywhat is a flag = _BYINDEX or _BYADDRESS
// This function looks in the table if a board with
// the same address or index, depending on bywhat, exists.
// The fields WHICH ARE NOT THOSE corresponding to bywhat
// are copied from the table.
{
	int ad,ind,i;
	BOARDPARAMETER localbp;

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	if (boardparam == 0)
		return _INVALID_ADDRESS;
	
	ad = boardparam->AddressofBoard;
	ind = boardparam->Index;

	switch (bywhat)
	{
	case _BYADDRESS:
		for(ind=0;ind<vartrmc->NbofBoard;ind++)
			if (vartrmc->board[ind].parameter.AddressofBoard == ad)
				break;
		if (ind==vartrmc->NbofBoard)
			return _NO_BOARD_AT_THIS_ADDRESS;
		break;
	case _BYINDEX:
		if ((ind<0) || (ind >= vartrmc->NbofBoard))
			return _NO_BOARD_WITH_THIS_INDEX;
		break;
	default:
		return _INVALID_BYWHAT;
	}

	// The board has been found : copy it
	localbp = vartrmc->board[ind].parameter;

	boardparam->NumberofCalibrationMeasure = localbp.NumberofCalibrationMeasure;
	boardparam->AddressofBoard = localbp.AddressofBoard;
	boardparam->TypeofBoard = localbp.TypeofBoard;
	boardparam->Index = localbp.Index;
	boardparam->CalibrationStatus = localbp.CalibrationStatus;
	for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++)
		boardparam->CalibrationTable[i] = localbp.CalibrationTable[i];
	boardparam->NumberofIRanges = localbp.NumberofIRanges;
	for (i=0;i<_MAX_NUMBER_OF_I_RANGES;i++)
		boardparam->IRangesTable[i] = localbp.IRangesTable[i];
	boardparam->NumberofVRanges = localbp.NumberofVRanges;
	for (i=0;i<_MAX_NUMBER_OF_V_RANGES;i++)
		boardparam->VRangesTable[i] = localbp.VRangesTable[i];

	return _RETURN_OK;
}				// FIN GetBoardTRMC()
//************************************************************* 
 
int _DLLSTATUS SetBoardTRMC(BOARDPARAMETER *boardparam)
// To set the parameter associated to a board.

// boradparam points on a BOARDPARAMETER struct.
// bywhat is a flag = _BYINDEX or _BYADDRESS
// This function looks in the table if a board with
// the same address or index, according to bywhat, exists.
// The fields WHICH ARE NOT THOSE corresponding to bywhat
// are copied from the parameter.
{
	int i;
	ABOARD *local;

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	if (boardparam == 0)
		return _INVALID_ADDRESS;

	// test the index
	if ((boardparam->Index<0) || (boardparam->Index >= vartrmc->NbofBoard))
		return _NO_SUCH_BOARD;

	// The board has been found : 
	local = &(vartrmc->board[boardparam->Index]);

	// Test the address
	if (local->parameter.AddressofBoard != boardparam->AddressofBoard)
		return _NO_SUCH_BOARD;

	// Verify the type of the board
	if (local->parameter.TypeofBoard!= boardparam->TypeofBoard)
		return _NO_SUCH_BOARD;

	// Check not in calibration
	if (local->parameter.CalibrationStatus == _CALIBRATION_MODE)
		return _BOARD_IN_CALIBRATION;

	if (!((boardparam->CalibrationStatus==_NORMAL_MODE)
		||(boardparam->CalibrationStatus==_START_CALIBRATION_MODE)))
		return _INVALID_CALIBRATION_STATUS;

	// copy the calibration coefficients FROM the parameter 
	// first test if the number is ok
	if (local->parameter.NumberofCalibrationMeasure != 
		boardparam->NumberofCalibrationMeasure)
		return _INVALID_CALIBRATION_PARAMETER;

	for(i=0;i<local->parameter.NumberofCalibrationMeasure;i++)
		local->parameter.CalibrationTable[i] = boardparam->CalibrationTable[i];
	
	// and the other parameters

	// initialisation de l'offset en courant pour la regul principale
	if (!boardparam->Index)
		local->Offset = (int) boardparam->CalibrationTable[0];
	
	local->parameter.CalibrationStatus = boardparam->CalibrationStatus;

	// Now init the board
	i = (vartrmc->InitBoard[local->parameter.TypeofBoard])(local);

	return i;
}				// FIN SetBoardTRMC()
//************************************************************* 
 

// ************************************************************
//				MEASURING ...
// ************************************************************

int _DLLSTATUS ReadValueTRMC(int index,AMEASURE *measure)
// Fill the structure pointed to by measure by the
// measured values for channel indexed by index in the table.

// return the number of values in the fifo BEFORE the read (Le 7-1-2004)
{
	ACHANNEL channel;
	int st;

	if (vartrmc == 0)
		return _TRMC_NOT_INITIALIZED;

	if ((index<0)||(index>=vartrmc->NbofChannel))
		return _NO_SUCH_CHANNEL;

	channel = vartrmc->Channels[index];
	if (channel.parameter.Mode == _NOT_USED_MODE)
		return _CHANNEL_NOT_IN_USE;

	st = ReadFifoMeasure(&channel,measure);
	return st;
}           // FIN ReadValueTRMC()
// ************************************************************




