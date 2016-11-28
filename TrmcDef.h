#define  _ABS(u)	((u)>=0?(u):-(u))

#define _CMD_DACA						0 // VRAI
#define _CMD_DACB						1  //VRAI
#define _CMD_REGUL						2 
#define _CMD_BOARD						3

#define _NBMAX_SUBADD					8

#define _BITOVERLOAD					1 
#define _BITCOMPLIANCE					2
#define _BITDACATMAX					4
  
#define _INVERSI0N_EVEN1				0 + 8 + 16+32+64+128 
#define _INVERSI0N_ODD1					1 + 8 + 16+32+64+128 
#define _INVERSI0N_EVEN2				4 + 8 + 16+32+64+128 
#define _INVERSI0N_ODD2					3 + 8 + 16+32+64+128 
 
#define _AUGEMNTE_NBRENVOI vartrmc->DelayCommTrmc+=1+vartrmc->DelayCommTrmc/10; 

#define _MAXCHANNELPERBOARD		8


#define _NOTUSED			-32767
#define	_DEFAUT_PREAVERAGING  	10	// 10 demi perionse  = 400 ms

#define	_NODATA					0
#define	_FIFOLENGTH		1000
//-------------------------------------------------------- 
//                   SOME DEFINES  
//-------------------------------------------------------- 
#define _ZERO 1e-30					// to test if a double is zero or not
#define _FORCE_STOP_AVERAGING 1

#define _KEY 0x68340000
#define _MASK 0x0000ffff

#define TARGET_RESOLUTION 1	// 1-millisecond target resolution 
#define _SYNCHRO 7		// How many 0 are sent at the end of a cmd-sent 
 
//#define _ANSWERALSOTO14(c) (((c)==_TYPEA) || ((c)==_TYPEC) || ((c)==_TYPEE)|| ((c)==_TYPEF)|| ((c)==_TYPEG)) 
#define _ANSWERALSOTO14(c) (((c)==_TYPEA) || ((c)==_TYPED) || ((c)==_TYPEC)|| ((c)==_TYPEE)|| ((c)==_TYPEF)|| ((c)==_TYPEG)) 
 
#define _NOT_BEAT	-1235438	// value unlikely to be set by chance

#define _MIN_COEFF	0.9		// smallest value for a coefficient
#define _MAX_COEFF	1.1		// largest value for a coefficient

// to initialize channels with default reasonnable values
//#define _DEFAULT_PREAVERAGING		25	// 40*25=1000ms=1second
//#define _DEFAULT_VALUERANGEI		1e-5
//#define _DEFAULT_VALUERANGEV		1e-5
#define _DEFAULT_SCRUTATION_TIME    10		// in preaveraging unit

#define _RECENT_CHANGE_TIME			5*25 // Time to forget a change in tic unit 25 means 1 second,

#define _THEEPOCH -10

#define _TOOSMALL -1
#define _TOOBIG 1
#define _NOCHANGE 0
#define _DECREASE -1
#define _INCREASE 1
#define _OK 0

#define _TWENTY	20

#define _NOBOOSTER_RETURN_TO_0	0
#define _NOBOOSTER_RETURN_TO_15	1
#define _BOOSTER_RETURN_TO_0	2
#define _BOOSTER_RETURN_TO_15	3
#define _DEFAULT_RESISTOR_REGUL 1000

//-------------------------------------------------------- 
//     			FIFO  STRUCTURE DEFINITIONS     
//-------------------------------------------------------- 

typedef struct 
{
	int	AverageCounter;
	int	Hit;
	int StartTime;  //bidon
	double MeasureRawAverage;
	double MeasureAverage;
	int Status;	
	int iRead; 
	int iWrite;  
	int Size; // size of the fifo
	short int  	TimerForgetChange;	
	AMEASURE  *data;
} FIFO; 
 

// ******************************************************* 
//				CHANNEL  STRUCTURE DEFINITIONS 
// ******************************************************* 
typedef struct achannel
{ 
	CHANNELPARAMETER parameter; // The parameters of the channel
	short int  	NumRangeI;		// intensity range 
	short int  	NumRangeV;		// voltage range 
	short int  	NumRangeIOld;	// intensity range 
	short int  	NumRangeVOld;	// voltage range 
	short int	ChangedRequired;
	short int	ConversionDone;	// if 0 : not used for regulation and no conversion is done
								// if bit_1=0 conversion performed for regul princ
								// if bit_2=1 conversion required for regul princ
								// if bit_3=0 conversion performed for regul aux
								// if bit_4=1 conversion required for regul aux
	int			OldMeasureTime;	// last measure time
	FIFO		*fifopt;		// Pointer to the fifo memory 
	AMEASURE	mes;			// Last leasurement after preaveraging
	struct aregul		*Regul;			// to which board it is associated
} ACHANNEL; 

// ******************************************************* 
//				REGULATION STRUCTURE DEFINITIONS 
// ******************************************************* 

typedef struct aregul
{ 
	short ForceAccTo0;
	short HeatingConfig;
	ACHANNEL *Channels[_NB_REGULATING_CHANNEL];
	ACHANNEL *ChannelOut;
	struct aboard  *Board;
	REGULPARAMETER parameter;
	double P;
	double I;
	double D;
	double alpha;
	double beta;
}	AREGUL;
 
//******************************************************** 
//     			BOARD  STRUCTURE DEFINITIONS     
//******************************************************** 
typedef struct aboard
{ 
	void      *PrivateMemory;	// Pointeur to a private memory
	long int  Time;				// Number of time has been called 
	BOARDPARAMETER parameter;	//  contains @ and sub@
	short int RangeFirst;		// if set send RegulDac before Range 
	int		  Data;				// complete set of data to send/board 
	int		  OddData;          // relevant to the inversion
	int		  EvenData;         // returnrelevant to the inversion
	unsigned short int FullScale;// Full scale masurement  
	short int Offset;			// Offset of masurement  
	short int Over;				// bit overload 
	short int Status;			// to keep track of the status during a cycle
	short int Measure;			// measure  
	short int OldOddMeasure;
	short int OldEvenMeasure;	// Old measurement calculation 
	short int PeriodinMs;		// Period in millisecondes
	short int ChannelTreated;	// -1 means no channel
	short int ChannelAlways;
	short int ChannelPriority;
	short int ChannelLast;
	short int NumRangeI;		// a help : the NumRange I value
	short int NumRangeV;		// a help : the NumRange V value
	short int ChannelTreatedOld;	// -1 means no channel
	short int NumRangeIOld;		// a help : the NumRange I value
	short int NumRangeVOld;		// a help : the NumRange V value
	short int NumberofChannels;
	ACHANNEL *Channels[_MAXCHANNELPERBOARD+1];
								// Array of the associated channels
								// including the calibration channel
	// for regul board this is the pointeur to the channels on which
	// regulate
	AREGUL *Regulation;		// Used only for regulation
	double	  CalibrationTableSave[_MAX_NUMBER_OF_CALIBRATION_MEASURE];
} ABOARD; 

//************************************************************* 
//      THE VARTRMC STRUCTURE : DEFINITION  
//************************************************************* 
typedef struct  
{ 
	short int com1;				// set if com1 used else com 2
	short int periodinms;		// period usually 40ms (value in .h)
	short int TimerRunning;		// set if the timer is running
	short int DelayCommTrmc;	// number of time a bit is re-sent(delay) 
	short int NbofBoard;		// number of boards detected 	5		 
	short int NbofChannel;		// number of channels initialized	
	short int Phase;			// phase ie in Syncrocall1 or in 2...
	short int CycleInProgress;  // set if in a cycle

	long int comm_error_in_callback;// number of communication error occurs since 
								 // last GetSynchroneousErrorTRMC
	long int calc_error_in_callback;// a calculation error occurs during 
								//the last GetSynchroneousErrorTRMC call 
	long int NbCall;			// Number of time Syncrocall1 has been called 
	long int NbTics;			// Number of tics
	
	long int (*InitBoard[_MAXTYPE])(ABOARD *board);
								// pointeur to functions to initi.
								// each type of board
	long int (*CheckChannelofBoard[_MAXTYPE])
		(ACHANNEL *channel);
								// pointeur to functions to initi.
								// a channel for baoard of each type
	long int (*CalcBoard[_MAXTYPE])(ABOARD *p1);
								// pointeur to functions to calcul.
								// each type of board

	ACHANNEL *Channels;			// Channel parameters table 
	ABOARD board[_MAXBOARD];			// Board parameters table 
	AREGUL Regulation0;	// 
	AREGUL Regulation1;	// 

} VARTRMC ; 


//************************************************************* 
//      THE TABRANGE STRUCTURE : DEFINITION  
//************************************************************* 


#define _MAXRANGE	32			//  Maximum number of Range for all boards

#define _RANGENOTDECIDED	0
#define _RANGEPLUS			-1
#define _RANGEMINUS			-2
#define _RANGEBYVALUE		-3
#define _RANGEBYNUMBER		-4


typedef struct 
{ 
	double	Value;		// Range Value (must be always >0 !!!) 	
	int Data;			// Data to be written to the board with Mask	
			// When TWO short int are to be sent the 2 MSB are the secund short to send
} ARANGE ; 


typedef struct 
{ 
	short int NbAutoRange;		// 	
	short int NbRange;			// 
	short int DefaultRange;		// 
	int Mask;// When TWO shorts int are to be sent the 2 MSB are the secund short to send
	ARANGE	rg[_MAXRANGE];
} TABRANGE ; 


typedef struct 
{ 
	short int NumRangeV;	// 			 
	short int NumRangeI;	// 	
	short int Channel;	// 	
	short int Time;		// 	
	double	Value;		//	Theoretical Value	
} AMEASURECAL ;


typedef struct 
{ 
	short int NumberMeasure;	// 
	AMEASURECAL	rg[_MAXRANGE];
} TABMEASURECAL;
