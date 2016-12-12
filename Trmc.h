//				Trmc.h
//  modif pour test cvs
#ifdef __linux__
#define _DLLSTATUS 
#define _EXTERN 
#else
#ifdef _DLL
#define _DLLSTATUS	__declspec( dllexport )
#else
#define _DLLSTATUS	__declspec( dllimport )
#endif 
#ifdef _CPP
#define _EXTERN extern "C"
#else
#define _EXTERN extern 
#endif
#endif
// **************************************************************** 
//						CONSTANTS FOR INITIALIZATION
// ****************************************************************
#define _NOTBEATING			0
#define _50HZ				1 
#define _60HZ				2


#define _COM1				1
#define _COM2				2

// ****************************************************************
//						CONSTANTS FOR ACCESSING CHANNELS
// **************************************************************** 
// different modes to address CHANNELS
#define  _BYINDEX			1
#define  _BYADDRESS			2

//             different mode for a CHANNEL
enum {	_INIT_MODE = -2,  		// Mode to be use at initialisation time.
		_NOT_USED_MODE=-1,		// Channel will NOT be addressed by synchroneous subroutines
		_FIX_RANGE_MODE,		// NO auto change at all
		_FIX_CURRENT_MODE,		// The VOLTAGE range is auto changed
		_FIX_VOLTAGE_MODE,		// The CURRENT range is auto changed
		_PRIORITY_CURRENT_MODE,	// The VOLTAGE Range is auto changed, at a mininal or maximal
								// range, the CURENT range is changed
		_PRIORITY_VOLTAGE_MODE,	// The CURRENT Range is auto changed, at a mininal or maximal
								// range, the VOLTAGE range is changed
		_SPECIAL_MODE			// used only for channel associated to a REGULATION:
								// choose the best ranges using parameters of the regulation
								// and keep it in _FIX_RANGE_MODE
	};

// ****************************************************************
//						CONSTANTS FOR ACCESSING BOARDS
// **************************************************************** 
// different mode for a BOARD
#define _CALIBRATION_FAILED		-1
#define _NORMAL_MODE			0
#define _START_CALIBRATION_MODE 2
#define _CALIBRATION_MODE		1

// list of the different types of boards:
enum {_TYPEREGULMAIN,_TYPEREGULAUX,
	_TYPEA,_TYPEB,_TYPEC,_TYPED,_TYPEE,_TYPEF,_TYPEG,_MAXTYPE};

// Values of PriorityFlag
enum {_NO_PRIORITY=0, _PRIORITY,_ALWAYS};		//	0, 1, 2

// indices:
#define	_REGULMAINBOARD			0		// index of main regulation
#define	_REGULAUXBOARD			1		// index of auxilliary regulation 
#define	_FIRSTBOARD				2		// board 0 and 1 are for regulation and Regul 
#define	_MAXBOARD (8+_FIRSTBOARD)	// 2 regulations + 8 measurement boards 

// addresses:
#define	_REGULMAINADDRESS		99		// Main regulation fake address
#define	_REGULAUXADDRESS		98		// Auxilliary reguluation fake address


#define _MAX_NUMBER_OF_CALIBRATION_MEASURE 16 
// maximum number of calibration coefficients for a board.

#define _MAX_NUMBER_OF_V_RANGES 32		// maximum number of voltage ranges
#define _MAX_NUMBER_OF_I_RANGES 32		// maximum number of intenity ranges

// ****************************************************************
//						CONSTANTS FOR ACCESSING REGULATION
// **************************************************************** 
// REGULATION PARAMETERS
#define _MAIN_REGULATION       0
#define _AUXILIARY_REGULATION  1
#define _NB_REGULATING_CHANNEL 4
#define _EMPTY_CHANNEL        -1
#define _NO				       0
#define _YES			       1
#define _AUTOMATIC		      -1

#define _LENGTHOFNAME		  64
// ******************************************************* 
//						STRUCTURES 
// ******************************************************* 
typedef struct 
{ 
	int Com;			// which serial port _COM1 or _COM2
	int Frequency;			// frequency _60HZ _50HZ _NOTBEATING or special debuging
	int CommunicationTime;	// time spent durin a period to communicate
	int futureuse;		// for futur use
} INITSTRUCTURE;

typedef struct 
{ 
	// 5 IDENTIFYING FIELDS:
	char name[_LENGTHOFNAME];
	double ValueRangeI;	     // Intensity range
	double ValueRangeV;	     // Voltage range 
	int	BoardAddress;		// Address of the board AS CHOOSEN ON THE BAORD WITH THE dip
	int	SubAddress;			// Address of the channel on the board (from 0)
	int	BoardType;			// Type (see the enum)
	int	Index;				// Index for identifying the channel
	// The remaining fields are parameters
	int	Mode;			     // See the enum
	int	PreAveraging;        // number of measures averaged in a result 
	int	ScrutationTime;		 // for mutichannel board only number of measures before going
							 // to next channel
	int	PriorityFlag;		 // for mutichannel board only NO_PRIORITY or _PRIORITY or _ALWAYS
							// Intensity range
							// Voltage range 
	int	FifoSize;			 // Size in measure of the fifo
	int	(*Etalon)(double *); // transform the argument in Ohm in Kelvinand return 0 if ok
} CHANNELPARAMETER; 


typedef struct 
{ 
	double CalibrationTable[_MAX_NUMBER_OF_CALIBRATION_MEASURE];
							// Calibration table ie ratio of measured/expected
							// The last coefficient is a quality of calibration for boards E & F
	double IRangesTable[_MAX_NUMBER_OF_V_RANGES]; // value of the ith I range
	double VRangesTable[_MAX_NUMBER_OF_I_RANGES]; // value of the ith V range
	int TypeofBoard;		// Type of the board 
	int AddressofBoard;		// Address of the board  
	int Index;				// Index to refer to the board
	int CalibrationStatus;	// input : _NORMAL_MODE _START_CALIBRATION_MODE
							// output :	 _NORMAL_MODE _CALIBRATION_FAILED _CALIBRATION_MODE	
	int NumberofCalibrationMeasure;	// Number of Calibration measures including offset
	int NumberofIRanges;    // Number of I ranges for this board
	int NumberofVRanges;    // Number of V ranges for this board
} BOARDPARAMETER;

typedef struct 
{ 
	// These 5 fields are used ONLY by Set end Get functions
	// not used in vartrmc->Regulation.
	char	name[_LENGTHOFNAME];
	double	SetPoint;					// Value on which regulate in Ohm or K (see below)
	double	P;							// Proportionnal term, if P=0 power=setpoint, if P<0 power hold to the last integral power
	double	I;							// Integral term
	double	D;							// Differential term
	double	HeatingMax;					// Maximal value in W for regulating
	double	HeatingResistor;			// Value in Ohm of the heater resistor 
	double	WeightofChannel[_NB_REGULATING_CHANNEL]; // Weight of the input
	int		IndexofChannel[_NB_REGULATING_CHANNEL]; // Up to 4 channels = input of regulation
										// _EMPTY_CHANNEL = no channel 
	int		Index;						// Index  = _REGULMAINBOARD OR _REGULAUXBOARD
	int		ThereIsABooster;			// A booster is connected (_YES or_NO)
	int		ReturnTo0;					// _YES _NO or _AUTOMATIC (if you don't know)
	
	
} REGULPARAMETER; 
 

typedef struct 
{ 
	double MeasureRaw;		// measure in Ohm
	double Measure;		// measure after conversion
	double ValueRangeI;			// used value range for I 
	double ValueRangeV;			// used value range for V 
	int	Time;					// time of the BEGINNING of the measure in ticks
	int	Status;					// see below
	int	Number;					// number of averaged points
	int Nothing;				// just to have a even number of bytes

} AMEASURE; 

typedef struct 
{ 
	int CommError;		// Number of communication error
	int CalcError;		// Number of calculation error
	int TimerError;		// Number of lost tics 
	int Date;			// Data at which errors have been set
} ERRORS; 
 
// ************************************************************
//        PROTOTYPES OF THE 11 DOCUMENTED FUNCTIONS
// ************************************************************

// ************************************************************
//				 3+1 GENERAL FUNCTIONS
// ************************************************************

_EXTERN int _DLLSTATUS StartTRMC(INITSTRUCTURE *init);
// 1) allocate global memory(excluding the FIFOs)
// 2) find the parameters for transmission
// 3) find the boards, their types and address
// 4) proceed to various initializations
// 5) run the timer (unless *TimeToSend20DataInMs=_NOT_BEAT)
// 6) initialize all boards, channels and regulations

_EXTERN int _DLLSTATUS StopTRMC(void);
// Kill the periodic timer. 

_EXTERN void _DLLSTATUS *SecretTRMC(void);
// An help to debug, not to be used by non expert.

_EXTERN int _DLLSTATUS GetSynchroneousErrorTRMC(ERRORS *error);
// Return a negative code if the function GetSynchroneousErrorTRMC 
// has not been executed, and a zero or positive number if an error 
// has been reflected in the 2 first fields of the ERROR structure.
// The Commerror and CalcError fields contain the number of errors 
// SINCE the last call. whereas the TimerError is the number of lost 
// tics since the epoch.
//
// Return 0 if everything clear.

// ************************************************************
//				MANIPULATING CHANNELS (4 FUNCTIONS)
// ************************************************************
_EXTERN int _DLLSTATUS GetNumberOfChannelTRMC(int *);
// Get the number of channel in n and return an error code
// if an errot occurs *n is not determined.

_EXTERN int _DLLSTATUS SetChannelTRMC(CHANNELPARAMETER *channel);
// To change one or several parmeters of a channel according to the channel *channel.
// The fields Address, SubAddress, BoardType and Index HAVE to match the
// corresponding fields in the table otherwise nothing is done and an eror code is returned.

_EXTERN int _DLLSTATUS GetChannelTRMC(int bywhat,CHANNELPARAMETER *);
// To get the parameters of a channel from the channel table.
// if bywhat = _BYADDRESS the channel with this address is
//     searched in the channel table and copied into the structure pointed to by channel.
// if bywhat = _BYINDEX the channel with index channel->Index is
//     searched in the channel table and copied into the structure pointed to by channel.
// An error code is returned if no corresponding channel is found in the table.

// ************************************************************
//				MANIPULATING REGULATION (2 FUNCTIONS)
// ************************************************************
//				ONLY ONE REGULATION CHANNEL IS ALLOWED
_EXTERN int _DLLSTATUS SetRegulationTRMC(REGULPARAMETER *r);
// To set the parameters used by a regulation.
// The regulation is either the main ot the auxilliary regul depending
// on r->Index. It can be _MAIN_REGULATION or _AUXILIARY_REGULATION


_EXTERN int _DLLSTATUS GetRegulationTRMC(REGULPARAMETER *);
// To get the parameters used by a regulation.
// The regulation is either the main ot the auxilliary regul depending
// on r->Index. It can be _MAIN_REGULATION or _AUXILIARY_REGULATION

// ************************************************************
//				MANIPULATING BOARDS (3 FUNCTIONS)
// ************************************************************
_EXTERN int _DLLSTATUS GetNumberOfBoardTRMC(int *n);
// Get the number of board in n and return an error code
// if an errot occurs *n is not determined.

_EXTERN int _DLLSTATUS GetBoardTRMC(int bywhat,BOARDPARAMETER *board);
// To get the parameter associated to a board.
// if bywhat = _BYADDRESS the board with this address is
//	 searched in the board table and copied into the  structure pointed to by board.
// if bywhat = _BYINDEX the board with index board->Index is 
//   searched in the board table and copied into the structure pointed to by board.
// An error code is returned if no corresponding board is fount in the table.

_EXTERN int _DLLSTATUS SetBoardTRMC(BOARDPARAMETER *);
// To set the parameter associated to a board.

// return an error code.
// ******************************************************* 
//					MEASURING (why not?) (1 FUNCTION)
// ******************************************************* 
_EXTERN int _DLLSTATUS ReadValueTRMC(int index, AMEASURE *measure);
// Fill the structure pointed to by measure by the
// measured values for channel indexed by index in the table.

// return the number of values in the fifo BEFORE the read (Le 7-1-2004) 
// ******************************************************* 
//					ERROR CODES 
// ******************************************************* 
#define _TIMER_NOT_RUNNING					4
#define _TIMER_ALREADY_RUNNING				3
#define _WRONG_RANGEINDEX	  				2
#define _CHANNEL_HAS_BEEN_MODIFIED			1
#define _RETURN_OK							0
#define _TRMC_NOT_INITIALIZED				-25
#define _NO_SUCH_BOARD						-43
#define _NO_BOARD_AT_THIS_ADDRESS			-16
#define _NO_BOARD_WITH_THIS_INDEX			-27

#define _NO_SUCH_CHANNEL					-19
#define _INVALID_SUBADDRESS					-18
#define	_INVALID_MODE						-20
#define _INVALID_PRIORITY					-21
#define _INVALID_BYWHAT						-26
#define _INVALID_ADDRESS					-28

#define _RANGE_CHANGE_NOT_POSSIBLE  		-12
#define	_WRONG_MODE_IN_RANGE				-15
#define _BOARD_IN_CALIBRATION				-42

#define _INVALID_CALIBRATION_PARAMETER      -47
#define _INVALID_CALIBRATION_STATUS			-46
#define _CALIBRATION_FAILED					-1
#define _NOT_USED_CHANNEL_AND_CALIBRATION_INCOMPATIBLE -5

#define _NO_PRIORITY_WITH_ZERO_SCRUTATION	-49
#define _NO_SUCH_REGULATION					-50
#define _INVALID_REGULPARAMETER				-51
#define _INVALID_CHANNELPARAMETER			-52
#define _HEATINGMAX_TOO_LARGE				-53
#define _CHANNEL_NOT_IN_USE					-38

#define _INVALID_COM						-45
#define _INVALID_FREQUENCY					-48
#define	_CANNOT_ALLOCATE_MEM				-6 
#define _COMM_NOT_ESTABLISH					-36
#define _COM_NOT_AVAILABLE                  -35

#define _14_NOT_ANSWERWED					-2 
#define _14_ANSWERWED						-3 
#define _WRONG_CODE_IN_BASE					-4 
#define _WRONG_ANSWER_IN_BASE				-37 
#define _TIMER_NOT_CAPABLE					-29
#define _INTERNAL_INCONSISTENCY				-44
#define _CANT_SETUP_WIRINGPI                            -60

#define _BIT_RECENT_CHANGE					0x80000001          
#define _BIT_STATUS_OVERLOAD				0x2  
#define _BIT_STATUS_AUTOCHANGE_I			0x4		
#define _BIT_STATUS_AUTOCHANGE_V			0x8
#define _BIT_STATUS_COMM_ERROR_IN_CALLBACK  0x10
#define _BIT_STATUS_CALC_ERROR_IN_CALLBACK  0x20
#define _BIT_TOOBIG							0x40
#define _BIT_TOOSMALL						0x80
#define _BIT_CHANNEL_CHANGED				0x100
#define _BIT_CHANNEL_CHANGE_REQUIRED		0x200
#define _BIT_CONVERSION_ERROR				0x400
#define _BIT_REGUL_CLEARED					0x800
#define _BIT_DAC_AT_MAX						0x1000

#define _MASK_STATUS_ERROR  (_BIT_STATUS_OVERLOAD	+ _BIT_STATUS_AUTOCHANGE_I +\
		_BIT_STATUS_AUTOCHANGE_V + _BIT_STATUS_COMM_ERROR_IN_CALLBACK +\
		_BIT_STATUS_CALC_ERROR_IN_CALLBACK  + _BIT_CHANNEL_CHANGED +\
		_BIT_CHANNEL_CHANGE_REQUIRED	+ _BIT_CONVERSION_ERROR	+ _BIT_DAC_AT_MAX )


// FOR DEBUGGING PURPOSE
/* rajouter __stdcall devant les proto et les decl=mode d'appel + general*/
#if 0	//						0	1	2	3	4	5	6	7	8
#define	_ADDRESSEVIVANTE	{	0,	1,	0,	0,	0,	0,	0,	0,	1}
#else	//						0	1	2	3	4	5	6	7	8
#define	_ADDRESSEVIVANTE	{	1,	1,	1,	1,	1,	1,	1,	1,	1}
#endif

// END FOR DEBUGGING PURPOSE
