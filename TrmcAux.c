#ifdef powerc
#include "manip.h"
#else
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <malloc.h>
#endif

#include "Trmc.h"  
#include "TrmcPlatform.h" 
#include "TrmcRunLib.h" 
#include "TrmcDef.h"  
#include "TrmcProto.h" 


extern VARTRMC *vartrmc;
static int base1(short int,short int,short int *,short int *) ; 
static int base2(short int,short int,short int *,short int *) ; 

//************************************************************* 
//      THE VARTRMC STRUCTURE :  ALLOCATION 
//************************************************************* 
int TrmcAllocInit() 
// Allocate memory for the global variables
// this is not a lot, since it does NOT include the FIFOs 
// 
{ 
	int i,j;

	// THE allocation= allocation of variables used by TrmcRunLib
	vartrmc = (VARTRMC *)malloc(sizeof(VARTRMC)); 
	if(!vartrmc) 
		return _CANNOT_ALLOCATE_MEM; 
		 

	//  global inits
//	vartrmc->NbCall = 0; 
//	vartrmc->NbTics = 0; 
//  inutile car c'est fait au moment du demarrage du timer
	vartrmc->DelayCommTrmc = 0; 
	vartrmc->TimerRunning = 0;

	vartrmc->CycleInProgress = 0;
//  init boards
	vartrmc->NbofBoard = 0;
		
//  init channels	 
	vartrmc->NbofChannel = 0;
	vartrmc->Channels = (ACHANNEL *)0;

//	Set the address of the various subroutines
	vartrmc->InitBoard[_TYPEREGULMAIN] = InitBoard_Regul;
	vartrmc->CheckChannelofBoard[_TYPEREGULMAIN] = CheckChannel_Regul;
	vartrmc->CalcBoard[_TYPEREGULMAIN] = Calc_Regul;
		
	vartrmc->InitBoard[_TYPEREGULAUX] = InitBoard_Regul;
	vartrmc->CheckChannelofBoard[_TYPEREGULAUX] = CheckChannel_Regul;
	vartrmc->CalcBoard[_TYPEREGULAUX] = Calc_Regul;
		
	vartrmc->InitBoard[_TYPEA] = InitBoard_A;
	vartrmc->CheckChannelofBoard[_TYPEA] = CheckChannelBoard_A;
	vartrmc->CalcBoard[_TYPEA] = CalcBoard_A;

	vartrmc->InitBoard[_TYPEB] = InitBoard_B;
	vartrmc->CheckChannelofBoard[_TYPEB] = CheckChannelBoard_B;
	vartrmc->CalcBoard[_TYPEB] = CalcBoard_B;

	vartrmc->InitBoard[_TYPEC] = InitBoard_C;
	vartrmc->CheckChannelofBoard[_TYPEC] = CheckChannelBoard_C;
	vartrmc->CalcBoard[_TYPEC] = CalcBoard_C;

	vartrmc->InitBoard[_TYPED] = InitBoard_D;
	vartrmc->CheckChannelofBoard[_TYPED] = CheckChannelBoard_D;
	vartrmc->CalcBoard[_TYPED] = CalcBoard_D;

	vartrmc->InitBoard[_TYPEE] = InitBoard_E;
	vartrmc->CheckChannelofBoard[_TYPEE] = CheckChannelBoard_E;
	vartrmc->CalcBoard[_TYPEE] = CalcBoard_E;

	vartrmc->InitBoard[_TYPEF] = InitBoard_F;
	vartrmc->CheckChannelofBoard[_TYPEF] = CheckChannelBoard_F;
	vartrmc->CalcBoard[_TYPEF] = CalcBoard_F;

	vartrmc->InitBoard[_TYPEG] = InitBoard_G;
	vartrmc->CheckChannelofBoard[_TYPEG] = CheckChannelBoard_G;
	vartrmc->CalcBoard[_TYPEG] = CalcBoard_G;

	// Set the field Index of ALL the ABOARD structures and 
	// the calibration coefficients to -1
	for (j=0;j<_MAXBOARD;j++)
	{
		BOARDPARAMETER *bp;
		bp = &vartrmc->board[j].parameter;
		bp->AddressofBoard = 0;
		bp->TypeofBoard = 0;
		bp->CalibrationStatus = 0;
		bp->Index = j;
		bp->NumberofCalibrationMeasure = 0;
		for (i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++) // set calibration coeff
			bp->CalibrationTable[i] = -1.0;
	}

	return  _RETURN_OK; 
}			//  FIN  TrmcAllocInit() 
// **************************************************************************** 

int TrmcSetDelayComm(int *TimeToSend100Data) 
// Determine the best value of DelayCommTrmc, and the time 
// in ms to send 4 +2*8 commandes with this value of DelayCommTrmc 
// The value of DelayComm is kept in the struct vartrmc for use.
// The value of vartrmc is known through the FIRST field of varspec. 
// 
// Try to send a commande with DelayCommTrmc = 1 
// if _NBESSAI trial are successfull, the value is accepted 
// else DelayCommTrmc is incremented 
{ 
	int i,err=0,tbeg,dt;
	short int data = 0x7f32; 
	short int diag; 
	short int res; 

	vartrmc->DelayCommTrmc = 1; 
	for(;;)	// Will try to send slower and slower up to no error 
	{ 
		tbeg = ElapsedTimePlatform();  
		for(i=0;i<_NBESSAI;i++) 
		{ 
			err = base2((unsigned char) ((i+1)%4),data, 
				&diag,&res); 
			if (err)
                break;
		} 
 
		if (err == 0)		// this value is OK 
			break; 
		vartrmc->DelayCommTrmc+= 1+ (vartrmc->DelayCommTrmc)/20;
		if (vartrmc->DelayCommTrmc >= _MAXDELAICOMMTRMC) 
		{					// probably the trmc2 is off!! 
			break;
		} 
		if ((dt= ElapsedTimePlatform()-tbeg) > _TEMPSMAX)  
		{				// to slow : probably the trmc2 is off!! 
			break;
		} 
	} 

	if (err != 0)
	{
		free(vartrmc);
		vartrmc = 0; //to prevent subsequent call to the dll
		return _COMM_NOT_ESTABLISH;
	}

// Increases delay for safety 
	_AUGEMNTE_NBRENVOI; 
 
// Here DelayCommTrmc is set, now try to measure the time of communication for 1 sec acquisition 
	tbeg = ElapsedTimePlatform();  
	for(i=0;i<100;i++)  
		base2(0,data,&diag,&res); 
	*TimeToSend100Data = ElapsedTimePlatform() - tbeg; 
	return _RETURN_OK; 
}				// FIN SetDelayComm() 
// **************************************************************************** 
 
int  TrmcConfig()
// Find out what are the boards  
// set in NbBoard the number of boards 
// set the address in AddBoard and the types in TypeBoard 
// 
// leave the subroutine as soon as an error has been detected: 
// return  _RETURN_OK if no problem detected, 
// return _14_NOT_ANSWERWED if a board which SHOULD answer to 14 didn't 
// return _14_ANSWERWED if a board which SHOULD NOT answer to 14 did 
// the criterion for answering to 14 is in .h file 
// 
// for every possible code send every possible type 
// and test if get a correct answer 
{ 
	int a,c;
	short int diag; 
	short int res; 
	short int data,result[_MAXBOARD][15]; 
	ABOARD *localboard;
	long int retval;
	int AddresseVivante[] = _ADDRESSEVIVANTE ;

	// RECORDING THE ANSWERS 
	for(a=_FIRSTBOARD;a<_MAXBOARD;a++)	// for all possible address 
	{ 
		// FOR DEBUGGING PURPOSE
		for (c=0;c<15;c++) 
			result[a][c]=0;
		if (!AddresseVivante[a-1]) 
			continue;

		// END FOR DEBUGGING PURPOSE

		for(c=0;c<15;c++)				// for all possible codes 
		{ 
			data = ((c)<<4) + 8 + (a-_FIRSTBOARD); 
//bit :	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
//data =0 	0	0	0	0	0	0	0	c	c	c	c	1	a	a	a			
			base2(_CMD_BOARD,data,&diag,&res); 
			base2(_CMD_BOARD,data,&diag,&res); 
			result[a][c] = (diag&1); 
		}								// end all possible code 
	}									// end all possible @ 

	//  ERRORS ? 
	// two card at the same adress ; not possible to detect

	// answering to code 14 
	for(a=_FIRSTBOARD;a<_MAXBOARD;a++)	// for all possible @ 
	{ 
		for(c=0;c<14;c++)				// for all possible codes 
		{ 
			if (!result[a][c]) 	
				continue; 
			if ((_ANSWERALSOTO14(c+_TYPEA)) && (!result[a][14]))
				return _14_NOT_ANSWERWED; 
			//if (!(_ANSWERALSOTO14(c+_TYPEA)) && (result[a][14]))
				//return _14_ANSWERWED; 
		}								// end all possibles codes 
 
	}									// end all possible @  

	// AT THIS POINT EVERYTHING IS OK 
	// set the global structure and the the parameters 

	//  make RegulAux board:
	localboard = &vartrmc->board[_REGULAUXBOARD];
	localboard->parameter.AddressofBoard = _REGULAUXADDRESS; 
	localboard->parameter.TypeofBoard = _TYPEREGULAUX; 
	localboard->parameter.Index = _REGULAUXBOARD; 
	localboard->parameter.CalibrationStatus = 0; //
	localboard->Data = 0;
	localboard->PrivateMemory = 0;			// to indicate the FIRST call
	localboard->Regulation = &(vartrmc->Regulation1);
	localboard->PeriodinMs = vartrmc->periodinms;
	localboard->ChannelLast = 0;
	localboard->ChannelTreated = 0;
	localboard->ChannelPriority = -1;
	localboard->ChannelAlways = -1;
	localboard->Offset = 0;
	for(a=0;a<_MAX_NUMBER_OF_CALIBRATION_MEASURE;a++)
		localboard->parameter.CalibrationTable[a] = -1;

	retval = (vartrmc->InitBoard[_TYPEREGULAUX])(localboard);
	if (retval)
		return retval;
	
	//  make RegulMain board:
	localboard = &vartrmc->board[_REGULMAINBOARD];
	localboard->parameter.AddressofBoard = _REGULMAINADDRESS; 
	localboard->parameter.TypeofBoard = _TYPEREGULMAIN; 
	localboard->parameter.Index = _REGULMAINBOARD; //ie 1
	localboard->parameter.CalibrationStatus = 0; //
	localboard->Data = 0;
	localboard->PrivateMemory = 0;			// to indicate the FIRST call
	localboard->Regulation = &(vartrmc->Regulation0);
	localboard->PeriodinMs = vartrmc->periodinms;
	localboard->ChannelLast = 0;
	localboard->ChannelTreated = 0;
	localboard->ChannelPriority = -1;
	localboard->ChannelAlways = -1;
	localboard->Offset = 0;
	for(a=0;a<_MAX_NUMBER_OF_CALIBRATION_MEASURE;a++)
		localboard->parameter.CalibrationTable[a] = -1;

	retval = (vartrmc->InitBoard[_TYPEREGULMAIN])(localboard);// call function init
	if (retval)
		return retval;

	vartrmc->NbofBoard = 2; // ie Regul and DacB

	//  make all measurement boards:	
	for(a=_FIRSTBOARD;a<_MAXBOARD;a++)	// for all possible @ 
	{ 
		for(c=_TYPEA;c<_MAXTYPE;c++)	// for all possible codes 
		{ 
			if (result[a][c-_TYPEA]) 
				{
				int i;
				// A board has been detected at this @=a with code=c 
				// modify the global structre  
				localboard = &vartrmc->board[vartrmc->NbofBoard];
				localboard->parameter.AddressofBoard = a-1; // ICI 
				// 9 - a so that the @ corresponds to the DIP
				localboard->parameter.TypeofBoard = c; 
				// call function init of this board:
				localboard->parameter.CalibrationStatus = 0; //
				localboard->Data = 0;
				localboard->PrivateMemory = 0;		// to indicate the FIRST call
				localboard->Regulation = 0;
				localboard->PeriodinMs = vartrmc->periodinms;
				// set the Index
				localboard->parameter.Index = vartrmc->NbofBoard; 
				localboard->ChannelLast = 0;
				localboard->ChannelTreated = 0;
				localboard->ChannelPriority = -1;
				localboard->ChannelAlways = -1;
				localboard->Offset = 0;
				for(i=0;i<_MAX_NUMBER_OF_CALIBRATION_MEASURE;i++)
					localboard->parameter.CalibrationTable[i] = -1;
				retval = (vartrmc->InitBoard[c])(localboard);

				if (retval)
					return retval;
				vartrmc->NbofBoard++; 
				// init the board (not the channels)
				
				break;	// out of the loop on the codes c
				} 
		}					// end all possible codes 
	}						// end all possible @ 
		 

	return  _RETURN_OK; 
}          // FIN TrmcConfig() 
// ****************************************************************

void MakeReguls(void)
{
	AREGUL *Regul;
	int i,j;
	// Make the two rwgulations regulations

	for (j=0;j<2;j++)
	{
		Regul = (j== _MAIN_REGULATION) ? &vartrmc->Regulation0 : &vartrmc->Regulation1;

		sprintf(Regul->parameter.name,"Regulation %d",j);
		Regul->parameter.Index = j;
		Regul->parameter.IndexofChannel[0] = -1;
		Regul->parameter.IndexofChannel[1] = -1;
		Regul->parameter.IndexofChannel[2] = -1;
		Regul->parameter.IndexofChannel[3] = -1;
		Regul->parameter.WeightofChannel[0] = 1;
		Regul->parameter.WeightofChannel[1] = 1;
		Regul->parameter.WeightofChannel[2] = 1;
		Regul->parameter.WeightofChannel[3] = 1;
		Regul->parameter.SetPoint = 0;
		Regul->parameter.P = 0;
		Regul->parameter.I = 0;
		Regul->parameter.D = 0;
		Regul->parameter.HeatingMax = 0;	
		Regul->parameter.HeatingResistor = _DEFAULT_RESISTOR_REGUL;
		Regul->parameter.ThereIsABooster = _NO;
		Regul->parameter.ReturnTo0 = _NO;

		Regul->HeatingConfig = _NOBOOSTER_RETURN_TO_15;
		Regul->ForceAccTo0 = 1;

		// Fixed value
		for (i=0;i<_NB_REGULATING_CHANNEL;i++)
			Regul->Channels[i] = (ACHANNEL *)0;
		// The two parameters ChannelOut and Board will be set elsewhere and later
	}
}          // FIN MakeReguls() 
// ****************************************************************


int MakeAllChannels()
// Create ALL the channels
{
	int i,j,k,l;
	ACHANNEL channel,*cpt;
	ABOARD *board;
	FIFO *fifopt;

	// First count the channel to be created (to avoid realloc)
	vartrmc->NbofChannel = 0;
	for (i=0;i<vartrmc->NbofBoard;i++)
	{
		board = &(vartrmc->board[i]);
		// Try to create channels
		for(j=0;j<_MAXCHANNELPERBOARD;j++)
		{
			// create the channel
			channel.parameter.BoardAddress = board->parameter.AddressofBoard;
			channel.parameter.SubAddress = j;
			channel.parameter.BoardType = board->parameter.TypeofBoard;
			channel.parameter.Mode = _INIT_MODE;
			channel.parameter.Etalon = 0;//int		(*Etalon)(double*);

			k = (vartrmc->CheckChannelofBoard[board->parameter.TypeofBoard])(&channel);
			if (k == _INVALID_SUBADDRESS)
				break;		// all the channels for this board have been set
			// here the channel is OK
			vartrmc->NbofChannel ++;
		}
	}

	// now the number of channels is set:one can allocate
	// but one needs one extra channel for calibrating per board
	// plus one extra extra channel for looping over the channels without pb
	i = vartrmc->NbofChannel + 1;
	vartrmc->Channels = (ACHANNEL *)malloc(i*sizeof(ACHANNEL));

	if (vartrmc->Channels == 0)
		return _CANNOT_ALLOCATE_MEM;

	// and perform the actual initialization
	for (l=i=0;i<vartrmc->NbofBoard;i++)
	{
		board = &(vartrmc->board[i]);
		// Try to create channels
		for(j=0;j<_MAXCHANNELPERBOARD;j++)
		{
			board->Channels[j] = 0; // not pointing to a channel yet 
			// create the channel
			cpt = &vartrmc->Channels[l];
			if (i == _REGULMAINBOARD)
				cpt->Regul = board->Regulation;
			else
				cpt->Regul = (AREGUL *)0;
			cpt->NumRangeI = -1;
			cpt->NumRangeV = -1;
			cpt->ChangedRequired = 0;
			cpt->OldMeasureTime = 0;
			cpt->ConversionDone = 0;
		
			cpt->mes.MeasureRaw = 0;
			cpt->mes.Measure = 0;
			cpt->mes.Time = 0;
			cpt->mes.Status = 0;
			cpt->mes.Number = 0;
			cpt->mes.ValueRangeI = 0.0;	
			cpt->mes.ValueRangeV = 0;	
		
			sprintf(cpt->parameter.name,"index=%d @=%d sub@=%d type=%d",
				l,board->parameter.AddressofBoard,j,board->parameter.TypeofBoard);
			cpt->parameter.BoardAddress = board->parameter.AddressofBoard;
			cpt->parameter.SubAddress = j;
			cpt->parameter.Index = l;
			cpt->parameter.BoardType = board->parameter.TypeofBoard;
			cpt->parameter.Mode = _INIT_MODE;
			cpt->parameter.FifoSize = _FIFOLENGTH;
			cpt->parameter.Etalon = 0;//int		(*Etalon)(double*);
			cpt->parameter.ScrutationTime = _DEFAULT_SCRUTATION_TIME;
			cpt->parameter.PriorityFlag = _NO_PRIORITY;

			k = (vartrmc->CheckChannelofBoard[board->parameter.TypeofBoard])(cpt);
			if (k == _INVALID_SUBADDRESS)
			{
				break;
				j = _MAXCHANNELPERBOARD;  // the extra channel for calibration
			}
			else
				l++; // real channel
			// here the channel is OK
			board->Channels[j] = cpt; 
			board->NumRangeI = cpt->NumRangeI;
			board->NumRangeV = cpt->NumRangeV;
			board->ChannelTreated = 0;
			board->NumRangeIOld = cpt->NumRangeI;
			board->NumRangeVOld = cpt->NumRangeV;
			board->ChannelTreatedOld = 0;

			// create the fifo
			fifopt = (FIFO *)malloc(sizeof(FIFO));
			if (!fifopt)
				return _CANNOT_ALLOCATE_MEM;
			cpt->fifopt = fifopt; 
			fifopt->iRead = 0;
			fifopt->iWrite = 0;
			fifopt->AverageCounter = 0;
			fifopt->Size = cpt->parameter.FifoSize + 1; // allocated = useful size + 1
			fifopt->data = (AMEASURE *)malloc(sizeof(AMEASURE)*fifopt->Size);
			if (fifopt->data == 0)
				return _CANNOT_ALLOCATE_MEM;
		}
	}
	return _RETURN_OK;
}          // FIN MakeAllChannels() 
// ****************************************************************

void SynchroRegulPointeur(void)
{
	// set the channelout for the regul
	AREGUL *Regul;

	// Make the regulations
	// only one yet
	Regul = &(vartrmc->Regulation0);
	Regul->ChannelOut = vartrmc->board[_REGULMAINBOARD].Channels[0];
	Regul->Board = &vartrmc->board[_REGULMAINBOARD];

	Regul = &(vartrmc->Regulation1);
	Regul->ChannelOut = vartrmc->board[_REGULAUXBOARD].Channels[0];
	Regul->Board = &vartrmc->board[_REGULAUXBOARD];

}          // FIN SynchroRegulPointeur() 
// ****************************************************************

static int base1(short int code, 
		 short int data, 
		 short int *diag, 
		 short int *res) 
// send the code then the data 
// set the two bits of diagnostic (_BITOVERLOAD  and  _BITCOMPLIANCE)
// and res to the result 
// return  _RETURN_OK if OK, a non-zero code if an error occured 
// ie a bit was NOT returned complemented 
{ 
	//power of two: 
	int pot[16] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768}; 
	int cpt,j,jj;				// to send the 16 bits 
	short int r0,r1;		// to store the two responses 
	char s,sav;				// the current and previous code sent 
	char FlagErr;			// Flag for error 

	if (code>3) 
		return _WRONG_CODE_IN_BASE; // wrong code 

	for (j=0;j<_SYNCHRO;j++) 
		SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc); 
	SendBitPlatform(1,&r0,&r1,vartrmc->DelayCommTrmc);
 
	*diag = 0;
	SendBitPlatform((char)(code&2),&r0,&r1,vartrmc->DelayCommTrmc);			// Send the bit c0  
	SendBitPlatform((char)(code&1),&r0,&r1,vartrmc->DelayCommTrmc);			// Send the bit c1  
	*diag |= (_BITOVERLOAD*r0);			// Diagnostic teet to satout (overload or presenece)
	FlagErr = ((code&2) == r1);			// test if an error occurs 

	*res = 0;							// to build the result
	for (sav=code&1,cpt=0;cpt<16;cpt++) 
	{ 
		s = !!(data&pot[15-cpt]);		// data to send (zero or one) 
		if (FlagErr)  
				s = 0;					// if error one send 0 
		// Send the data (or 0 if error)  
		SendBitPlatform(s,&r0,&r1,vartrmc->DelayCommTrmc);	
	
		// Use r1 to detect communication problem
		FlagErr =  FlagErr || (r1==sav);// test errors r1==sav is INCORRECT 
	
		// use r0 to set one of the 2 variables diag or res
		if (cpt==0) // r0 is used to set diag
			*diag |= ((!r0) * (_BITCOMPLIANCE));// comp_regul set after sending 1st bit
		else		// r0 is used to set res
			*res |= r0*pot[15+1-cpt];		// one more bit of the result

		sav = s;						// remember old data sent 
	}
	// Send the first bit of validation,
	// and use r0 to set the variable res
	SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc);
	*res += r0*pot[0];		// last bit of the result
	FlagErr =  FlagErr || (r1==sav);// test errors r1==sav is INCORRECT 
	
	// CHANGER LE BIT DE PDS FORT POUR OBTENIR UN ENTIER SIGNE
	// CORRECT 
	// BRUT   *res ^= 0x8000;

	// send the validation 
	if (FlagErr) 
	{ 
		for(j=0;j<3;j++) 
			SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc); 
	} 
	else 
	{ 
		for(jj=0;jj<20;jj++)
			SendFinalPlatform(vartrmc->DelayCommTrmc); 
 		// Send the second bit of validation:
		SendBitPlatform(1,&r0,&r1,vartrmc->DelayCommTrmc); 
		// Send the third bit of validation: 
		SendBitPlatform(1,&r0,&r1,vartrmc->DelayCommTrmc);
		//  pour rajouter un delay avant la conversion
		for(jj=0;jj<10;jj++)
			SendFinalPlatform(vartrmc->DelayCommTrmc); 	
		//	Send the final 0: 
		SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc);
	} 
	 
	// final 0 + delay 
	SendFinalPlatform(vartrmc->DelayCommTrmc); 
 
	if (FlagErr) 
		return _WRONG_ANSWER_IN_BASE; 
	else 
		return  _RETURN_OK; 
}		//FIN base1() 
// ************************************************************* 
//  base2  identique a base1 mais sans les delais
static int base2(short int code, 
		 short int data, 
		 short int *diag, 
		 short int *res) 
// send the code then the data 
// set the two bits of diagnostic (_BITOVERLOAD  and  _BITCOMPLIANCE)
// and res to the result 
// return  _RETURN_OK if OK, a non-zero code if an error occured 
// ie a bit was NOT returned complemented 
{ 
	//power of two: 
	int pot[16] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768}; 
	int cpt,j;				// to send the 16 bits 
	short int r0,r1;		// to store the two responses 
	char s,sav;				// the current and previous code sent 
	char FlagErr;			// Flag for error 

	if (code>3) 
		return _WRONG_CODE_IN_BASE; // wrong code 

	for (j=0;j<_SYNCHRO;j++) 
		SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc); 
	SendBitPlatform(1,&r0,&r1,vartrmc->DelayCommTrmc); 
 
	*diag = 0;
	SendBitPlatform((char)(code&2),&r0,&r1,vartrmc->DelayCommTrmc);			// Send the bit c0  
	SendBitPlatform((char)(code&1),&r0,&r1,vartrmc->DelayCommTrmc);			// Send the bit c1  
	*diag |= (_BITOVERLOAD*r0);			// Diagnostic teet to satout (overload or presenece)
	FlagErr = ((code&2) == r1);			// test if an error occurs 

	*res = 0;							// to build the result
	for (sav=code&1,cpt=0;cpt<16;cpt++) 
	{ 
		s = !!(data&pot[15-cpt]);		// data to send (zero or one) 
		if (FlagErr)  
				s = 0;					// if error one send 0 
		// Send the data (or 0 if error)  
		SendBitPlatform(s,&r0,&r1,vartrmc->DelayCommTrmc);	
	
		// Use r1 to detect communication problem
		FlagErr =  FlagErr || (r1==sav);// test errors r1==sav is INCORRECT 
	
		// use r0 to set one of the 2 variables diag or res
		if (cpt==0) // r0 is used to set diag
			*diag |= ((!r0) * (_BITCOMPLIANCE));// comp_regul set after sending 1st bit
		else		// r0 is used to set res
			*res |= r0*pot[15+1-cpt];		// one more bit of the result

		sav = s;						// remember old data sent 
	}
	// Send the first bit of validation,
	// and use r0 to set the variable res
	SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc);
	*res += r0*pot[0];		// last bit of the result
	FlagErr =  FlagErr || (r1==sav);// test errors r1==sav is INCORRECT 
	
	// CHANGER LE BIT DE PDS FORT POUR OBTENIR UN ENTIER SIGNE
	// CORRECT 
	// BRUT   *res ^= 0x8000;

	// send the validation 
	if (FlagErr) 
	{ 
		for(j=0;j<3;j++) 
			SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc); 
	} 
	else 
	{ 
//		for(jj=0;jj<20;jj++)
//			SendFinalPlatform(vartrmc->DelayCommTrmc); 
 		// Send the second bit of validation:
		SendBitPlatform(1,&r0,&r1,vartrmc->DelayCommTrmc); 
		// Send the third bit of validation: 
		SendBitPlatform(1,&r0,&r1,vartrmc->DelayCommTrmc);
		//  pour rajouter un delay avant la conversion
//		for(jj=0;jj<10;jj++)
//			SendFinalPlatform(vartrmc->DelayCommTrmc); 	
		//	Send the final 0: 
		SendBitPlatform(0,&r0,&r1,vartrmc->DelayCommTrmc);
	} 
	 
	// final 0 + delay 
	SendFinalPlatform(vartrmc->DelayCommTrmc); 
 
	if (FlagErr) 
		return _WRONG_ANSWER_IN_BASE; 
	else 
		return  _RETURN_OK; 
}		//FIN base2() 
// ************************************************************* 

#define _DATA0(i) ((short)(vartrmc->board[(i)].Data &0xffff))
#define _DATA1(i) ((short)((vartrmc->board[(i)].Data>>16) &0xffff))


int SynchroCall1(void *AuxVoidPt)
// this procedure will be called every 40 ms 
{ 
	VARTRMC *vartrmc = AuxVoidPt;
	short int diag; 
	short int res,data,flip; 
	int localstatus;
 
//-------------------------------------------------------- 
//        IS THIS SUBROUTINE CALLED WITHIN A PREVIOUS CYCLE 
//-------------------------------------------------------- 
	vartrmc->NbTics++;
		
	if (vartrmc->CycleInProgress)
		return 1;
	vartrmc->CycleInProgress = 1; // We are now in a new cycle

//-------------------------------------------------------- 
//                   VARIOUS INITIALIZATIONS 
//-------------------------------------------------------- 
	// firstly increased the number of call and see it odd or even tick
	vartrmc->NbCall++; 
	flip = (vartrmc->NbCall)%2; 
	// clear the Regul.Over to accumulate compliance durin boards interrogation
	vartrmc->board[_REGULMAINBOARD].Over = 0;
	localstatus = 0;

//-------------------------------------------------------- 
//                   INVERSE 
//-------------------------------------------------------- 
	
	if (flip) 
		data = _INVERSI0N_EVEN1; 
	else 
		data = _INVERSI0N_ODD1; 

	localstatus += base2(_CMD_BOARD,data,&diag,&res); 
//-------------------------------------------------------- 
//                   AUXILLIARY REGULATION
//-------------------------------------------------------- 
	
	if (vartrmc->board[_REGULAUXBOARD].Channels[0]->parameter.Mode != _NOT_USED_MODE)
	{
		localstatus += base2(_CMD_DACB,_DATA0(_REGULAUXBOARD),&diag,&res);
		diag = vartrmc->board[_REGULAUXBOARD].Over;
	}
     
//-------------------------------------------------------- 
//                   MAIN REGULATION
//-------------------------------------------------------- 
	// Regulation commands are ALWAYS sent 
	if (vartrmc->board[_REGULMAINBOARD].RangeFirst) 
	{ 
		localstatus += base2(_CMD_REGUL,_DATA1(_REGULMAINBOARD),&diag,&res); 
		diag = (~diag);  /*ie 0=ok 1=over onee keeps only significant bits*/
		vartrmc->board[_REGULMAINBOARD].Over |= ((diag)&(_BITCOMPLIANCE));

		localstatus += base2(_CMD_DACA,_DATA0(_REGULMAINBOARD),&diag,&res); 
		diag = (~diag);  /*ie 0=ok 1=over onee keeps only significant bits*/
		vartrmc->board[_REGULMAINBOARD].Over |= ((diag)&(_BITCOMPLIANCE));
	} 
	else
	{ 
		//  send  data0 to the DACA and  data1 to the regul
		localstatus += base2(_CMD_DACA, _DATA0(_REGULMAINBOARD),&diag,&res); 
		diag = (~diag);  /*ie 0=ok 1=over one keeps only significant bits*/
		vartrmc->board[_REGULMAINBOARD].Over |= ((diag)&(_BITCOMPLIANCE));

		localstatus += base2(_CMD_REGUL,_DATA1(_REGULMAINBOARD),&diag,&res); 
		diag = (~diag);  /*ie 0=ok 1=over onee keeps only significant bits*/
		vartrmc->board[_REGULMAINBOARD].Over |= ((diag)&(_BITCOMPLIANCE));
	}

	if (localstatus)
		vartrmc->comm_error_in_callback++;

	return 0; // normal operation Master can call SynchroCall2

}           // FIN SynchroCall1() 
// ************************************************************* 
 
void SynchroCall2(void *AuxVoidPt) 
// this procedure will be called every 40 ms 
{ 
	VARTRMC *vartrmc = AuxVoidPt;
	short int diag; 
	short int res,data;
	short int flip;
	int i,j;
	short int unsish;
	ACHANNEL *Channel;
	int localstatus;

	localstatus = 0;
	flip = (vartrmc->NbCall)%2; 
 
//-------------------------------------------------------- 
//              SEND THE VARTRMC CLOSE MEASURE
//-------------------------------------------------------- 
	if (flip) 
		data = _INVERSI0N_EVEN2; 
	else 
		data = _INVERSI0N_ODD2; 
	localstatus += base2(_CMD_BOARD,data,&diag,&res); 
//-------------------------------------------------------- 
//                  NOW SEND  
//-------------------------------------------------------- 
	for (i=_FIRSTBOARD;i<vartrmc->NbofBoard;i++)
	{	
		ABOARD *boardpt = &vartrmc->board[i];

		localstatus += base1(_CMD_BOARD,_DATA0(i),&diag,&res); 
 
		// ***************************************************************
		// THIS CALL TO base() WILL EFFECTIVELY GET THE VALUE 
		// ***************************************************************
		localstatus += base2(_CMD_BOARD,_DATA1(i),&diag,&unsish);

		// unsish is a unsigned short ie 0<= unsish <65536
		// this raw value will be treated by MeasuredValue()
		diag = (~diag);  /*ie 0=ok 1=over onee keeps only significant bits*/

		boardpt->Measure = unsish; 
		boardpt->Over = (diag)&_BITOVERLOAD; // Follow Overange only discard Compliance
		vartrmc->board[_REGULMAINBOARD].Over |= ((diag)&(_BITCOMPLIANCE));
	} 
	if (localstatus)
		vartrmc->comm_error_in_callback++;
 
//-------------------------------------------------------- 
//                  CALCULATION AND WRITE IN FIFOs
//--------------------------------------------------------  
	for (i=0;i<vartrmc->NbofBoard;i++) // looping over boards
	{
		ABOARD *boardpt = boardpt = &(vartrmc->board[i]); 

		boardpt->Time = vartrmc->NbCall;//  time for measure

		//	first set the bits to inverse the reading of the measure
		// it has to be done even if not actual reading is performd
		// for electrical reasons. If the channel is treated this will
		// be overwritten in calcboard, but if since a continue
		// calcboard is not called, the inversion will be done anyway	
		// the problem is that one does not touch the other bits
#define _EVENDATA (boardpt->EvenData)
#define _ODDDATA (boardpt->OddData)

		if (boardpt->Time%2)
			boardpt->Data = (boardpt->Data & (~(_EVENDATA | _ODDDATA))) | _ODDDATA;  
		else
			boardpt->Data = (boardpt->Data & (~(_EVENDATA | _ODDDATA))) | _EVENDATA;  

		if (boardpt->ChannelTreated == -1)
			continue;	// no Channel has been treated by the call to CalcBoardX
		
		Channel = boardpt->Channels[boardpt->ChannelTreated];		

		// For multi-channel board the scan of channel is performed in CalcBoard
		if (Channel->parameter.Mode == _NOT_USED_MODE)
		{
			boardpt->ChannelTreated = 
				(boardpt->ChannelTreated+1)%boardpt->NumberofChannels;
			continue;
		}

		// CALCULATION FOR THIS BOARD + FILL THE FIFOS
		j = (vartrmc->CalcBoard[boardpt->parameter.TypeofBoard])(boardpt);
		if (j)
			vartrmc->calc_error_in_callback = 0;


	}  // end loop board

	vartrmc->CycleInProgress = 0; // Current cycle completed
}           // FIN SynchroCall2() 
// ************************************************************* 

int WriteFifoMeasure(ACHANNEL* Channel,int ForceStopAveraging) 
// SYNCHRONEOUS call
// return 1 if an avearge has been compelted 0 if not
{
	FIFO *fifopt=Channel->fifopt;

	if (fifopt->data == 0)
		return 0;

	if (Channel->parameter.PreAveraging==0)
		return 0;

	//  Starting to average PreAveraging values
	if(!fifopt->AverageCounter)
	{
		fifopt->Status = 0;						// 0 means OK
		fifopt->MeasureAverage = 0;				// Value cleared before accumulating
		fifopt->MeasureRawAverage = 0;				// Value cleared before accumulating
		fifopt->Hit = 0;					// Value cleared before accumulating
//		fifopt->TimerForgetChange = 0;
	}

	// accumulate the read value 
	fifopt->MeasureAverage += Channel->mes.Measure;
	fifopt->MeasureRawAverage += Channel->mes.MeasureRaw;
	fifopt->AverageCounter++;
	fifopt->Hit++;

	// accumulate the status
	// result from CalBoard
	// if a change of channel has been done
	if (Channel->mes.Time-Channel->OldMeasureTime > 1)
		Channel->mes.Status |= _BIT_CHANNEL_CHANGED;
	//if a changed of range has been required
	if (Channel->ChangedRequired)
	{
		Channel->ChangedRequired = 0; // now it is reflected in the status
		Channel->mes.Status |= _BIT_CHANNEL_CHANGE_REQUIRED;
	}

	// if an AUTO-change in the range occured
	if (Channel->NumRangeI != Channel->NumRangeIOld)
		Channel->mes.Status |= _BIT_STATUS_AUTOCHANGE_I;
	if (Channel->NumRangeV != Channel->NumRangeVOld)
		Channel->mes.Status |= _BIT_STATUS_AUTOCHANGE_V;

	// Take care of the bit indicating a recent change (for regul mainly)
	if (Channel->mes.Status & (_MASK_STATUS_ERROR))
		fifopt->TimerForgetChange = _RECENT_CHANGE_TIME;
	if (fifopt->TimerForgetChange)
	{
		Channel->mes.Status  |= _BIT_RECENT_CHANGE;
		fifopt->TimerForgetChange--;
	}

	// The status is ready to be accumulated:
	fifopt->Status |= Channel->mes.Status; // set a one in any postion of mes.Status

	// *****************************************************************************
	// if the number of values accumulated is >= PreAveraging then proceed
	// *****************************************************************************
	if ((fifopt->AverageCounter >= Channel->parameter.PreAveraging)
		|| (ForceStopAveraging))
	{
		// If the fifo is full, drop the oldest item
		if ((fifopt->iWrite+1)%fifopt->Size == fifopt->iRead)
			fifopt->iRead = (fifopt->iRead+1)%fifopt->Size;

		double x;
		// Value:		
		x =	fifopt->MeasureRawAverage/fifopt->Hit;
		fifopt->data[fifopt->iWrite].MeasureRaw = x;

		if ((Channel->parameter.BoardType == _MAIN_REGULATION) ||
			(Channel->parameter.BoardType == _AUXILIARY_REGULATION) )
		{
			// Traitement d'un channel associe a une "fausse board" : regulation
			// on retourne la moyenne des convertis
			x = fifopt->MeasureAverage/fifopt->Hit;
		}
		else 
		{
			if (Channel->parameter.Etalon)
			{
				// Traitement d'un channel associe a une "vraie board"
				// on retourne le converti de la moyenne (et non la moyenne des convertis)
				if ((Channel->parameter.Etalon)(&x))
					fifopt->Status |= _BIT_CONVERSION_ERROR;
			}
		}

		fifopt->data[fifopt->iWrite].Measure = x;
		
		// Time:
		fifopt->data[fifopt->iWrite].Time = vartrmc->NbTics;

		// Numbr of accumulated measures:
		fifopt->data[fifopt->iWrite].Number = fifopt->Hit;

		if 	(vartrmc->comm_error_in_callback) // set the 5th if comm error 
			fifopt->Status |= _BIT_STATUS_COMM_ERROR_IN_CALLBACK;
		if 	(vartrmc->calc_error_in_callback) // set the 6th if a calc. error
			fifopt->Status |= _BIT_STATUS_CALC_ERROR_IN_CALLBACK;

		fifopt->data[fifopt->iWrite].Status = fifopt->Status;
	
		// Ranges:
		fifopt->data[fifopt->iWrite].ValueRangeI = Channel->parameter.ValueRangeI;
		fifopt->data[fifopt->iWrite].ValueRangeV = Channel->parameter.ValueRangeV;

		// Clear for next averaging
		fifopt->AverageCounter = 0;
		fifopt->iWrite=(fifopt->iWrite+1)%fifopt->Size;

		return 1;
	}

	return 0;
} 
//			FIN WriteFifoMeasure()
// **************************************************************

int	ReadFifoMeasure(ACHANNEL *Channel,AMEASURE *measure)
{
	// ASYNCHRONEOUS call return the number of data to be read BEFORE the call
	FIFO * fifopt=Channel->fifopt;
	int NumberofData;

	NumberofData = (fifopt->iWrite + fifopt->Size - fifopt->iRead)%fifopt->Size;
	if	(NumberofData)
	{	
		*measure = fifopt->data[fifopt->iRead];
		fifopt->iRead = (fifopt->iRead+1)%fifopt->Size;
	}
	return NumberofData;
}				// FIN ReadFifoMeasure()
// ************************************************************* 

void FlushFifo(ACHANNEL *Channel)
// ASYNCHRONEOUS call. Empty the channel's FIFO, discarding its contents.
{
	FIFO * fifopt = Channel->fifopt;
	fifopt->iRead = fifopt->iWrite;
}

 double	MeasuredValue(ABOARD *Board)
/*
Return a double r whose absolute value acnnot exceed 1 by a large amount:
	if 0V at DAC input then r around 0 (offset/fullscale)
	if -10V at DAC input then r around -1
	if +10V at DAC input then r around 1

  Compute z = x0 - 2*x1 + x2 where xi are consecutive raw measures
shift the values (2->1, 1->0) taking into account the tick parity
scale properly the z value and 
*/
{
	int a;
 	double x;

	if (Board->Time%2==0)			//  Even measurement
 	{	
 		a = -(int) Board->OldEvenMeasure 
			- (int) Board->Measure 
			+ (int) 2*Board->OldOddMeasure;
 		Board->OldEvenMeasure = Board->Measure;
 	}
	else							//  Odd measurement
 	{		
 		a = -(int) 2*Board->OldEvenMeasure 
			+ (int) Board->OldOddMeasure 
			+ (int) Board->Measure;
 		Board->OldOddMeasure = Board->Measure;
 	}
	// here   -(4*32768-2) <= a < 4*32767-2
	a = a/2;
	// here  -(2*32768-1) <= a < 2*32767-1
	a = a - Board->Offset;
	// here  -(2*32768-1)-Offset <= a < 2*32767-1 -Offset

	//  Board->FullScale is around 65000
	x = ((double)a ) / ((double)Board->FullScale);
	// (-(2*32768-1)-Offset)/Fullscale <= x < (2*32767-1 -Offset)/Fullscale
	

	return(x);
 }				// FIN MeasuredValue()
 // *************************************************************
 
