// version adaptee a sauvegarde cvs TrmcRunLib.h
//-------------------------------------------------------- 
//                   SOME DEFINES  
//-------------------------------------------------------- 
#define TARGET_RESOLUTION 1	// 1-millisecond target resolution 
#define _SYNCHRO 7		// How many 0 are sent at the end of a cmd-sent 
  
#define _PERIOD_IN_MS		40 		// Period for main timer
#define	_NBMAXCALL			(2*_MAXBOARD+5)						// max number of call in one period
#define _NBESSAI 			200		// How many sendings have to be succesfull to accept DelayCommTrmc 
#define _MAXDELAICOMMTRMC  		4000	// The largest accpeted value of NbRenvoi 
//#define _TEMPSMAX (2*_NBESSAI*_PERIOD_IN_MS/ (3* _NBMAXCALL) ) 	
				// max time allowed for _NBESSAI call 
				// 2/3 of theoritical max time allowed
#define _TEMPSMAX		(1*1000)
//-------------------------------------------------------- 
//                  PROTOTYPES OF NON DOCUMENTED FUNCTIONS 
//-------------------------------------------------------- 

// Fonctions resolues dans TrmcRunLib.c et appellees ds TrmcWin.c
int SynchroCall1(void *); 
void SynchroCall2(void *); 
int TrmcAllocInit(); 
int TrmcSetDelayComm(int *); 
int TrmcConfig(); 

