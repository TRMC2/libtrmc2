// SPDX-License-Identifier: LGPL-3.0-or-later
/*
                       TrmcBoard.c

		Specific subroutines for each board type(A,B,C,D,E)
     Dacs and Regulation are considered and treated as boards
*/
/*
For each board 3 subroutines must be provided:
1/  long int CheckChannelBoardX(ACHANNEL *Channel)
2/  long int InitBoardX(ABOARD *board)
3/  long int CalcBoardA(ABOARD *boardpt)
where X=REGULMAIN,REGULAYX,A,B,C,D,E

1/ CheckChannelBoardX(ACHANNEL *Channel)
verify the validity of the parameters: if there is a small illegality
modify them to make them legal, if there is a big illegality do nothing
and return an error code. Small illegality is a value range not exactly
in the table: take the closest. Big illegality is an invalid mode.
Note that the mode _INIT_MODE has to be handle here.

2/ InitBoardX(ABOARD *board)
do various initialisations : private memory, initialize counters

3/ CalcBoardX(ABOARD *boardpt)
For regular board take the data from boardpt and put it in channeltable 
after suitable normalization and conversion. The value will go in the FIFO 
after averaging (SynchroCall2). 
For the DACs take the value from channeltable and put it in boardpt.
Prepare also the data0 and data1 to be sent by SynchroCall2
*/

//*****************************************************************************
#ifdef powerc
#include "manip.h"
#define  abs(u)	((u)>=0?(u):-(u))
#else
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <malloc.h>
#include <math.h> 
#include <float.h> 
#endif

#include "Trmc.h" 
#include "TrmcRunLib.h"

#include "TrmcDef.h"
#include "TrmcProto.h"
#include "TrmcBoard.h"

#define	_RANGE(n)	{\
	*NumRange = n;\
	*ValueRange = Range->rg[n].Value;\
	if (DataRange) *DataRange = \
			(*DataRange & (Range->Mask^0xffffffff)) | Range->rg[n].Data;}

int SetRangeAndData(TABRANGE *Range, 
					short int Mode,
					short int* NumRange,
					double *ValueRange,
					int* DataRange)
/*    SET NumRange, ValueRange and DataRange WITH CORRECT VALUES

Range  is the table of possible range	= INPUT
Mode is how to decide the new value		= INPUT
*NumRange =position in the array		= INPUT if MODE=_RANGEBYNUMBER else OUTPUT
*ValueRange = the value					= OUTPUT if MODE=_RANGEBYNUMBER else INPUT
*DataRange =	binary code to send		= OUTPUT (except if null)
if Mode = _RANGEBYNUMER
		then *NumRange is tested to be possible 
if Mode == _RANGEPLUS or _RANGEMINUS  
        then try to change NumRange (+ or -) INSIDE the
		allowed values for autorange (0 to NbAutoRange -1)
if Mode == _RANGEBYVALUE	
		then *ValueRange is tested to be close enough to the closest possible value 
		
ALLWAYS *NumRange, *ValueRange and *DataRange are set accordingly

Return	0 if everything was OK, 
		a positive error code if it has NOT been possible
		a negative error code if a parameter has been changed but done
*/
{
	int n=*NumRange;

	if(Mode==_RANGEBYNUMBER) // The INDEX is given
	{
		if((n<0) || (n>=Range->NbRange) )
		{
			_RANGE(Range->DefaultRange);
			return _WRONG_RANGEINDEX;
		}
		_RANGE(n);
		return	_RETURN_OK;
	}

	if (Mode==_RANGEBYVALUE)  // The value is given
	{
		double x,x2min = DBL_MAX;
		int kmin = -1;

		for(n=0;n<Range->NbRange;n++)
		{
			x = (*ValueRange - Range->rg[n].Value);
			if (fabs(x)<1e-30)
			{
				// We have found the correct value 
				x2min = 0;
				kmin = n;
				break;
			}
			x = x/Range->rg[n].Value;
			if (n)
			{
				if(x*x<x2min) 
				{
					x2min=x*x;
					kmin=n;
				}
			}
			else 
			{
				x2min=x*x;kmin=n;
			}
		}
		// kmin points to the closest value, and RELATIVE square distance is x2min
		if (x2min>10*10)
		{
			_RANGE(Range->DefaultRange);
			return _RANGE_CHANGE_NOT_POSSIBLE;
		}

		_RANGE(kmin);
		return  _RETURN_OK;
	}
	
	if (Mode == _RANGEPLUS) // The INDEX is given to be incremented
	{
		n++;
		if(n>=Range->NbAutoRange)
		{
			_RANGE(Range->NbAutoRange-1);
			return _RANGE_CHANGE_NOT_POSSIBLE;
		}
		_RANGE(n);
		return  _RETURN_OK; 
	}

	if (Mode == _RANGEMINUS) // The INDEX is given to be decremented
	{
		n--;
		if(n<0)
		{
			_RANGE(0);
			return _RANGE_CHANGE_NOT_POSSIBLE;
		}
		_RANGE(n);
		return  _RETURN_OK; 
	}

	return  _WRONG_MODE_IN_RANGE; 
}           // FIN SetRangeAndData()  
// ************************************************************* 

double raisepower(double x,int k) 
// compute x**k with 0**k =0 EVEN IF K<0 !!
{
	int i;
	double y;
	if (fabs(x)<1e-10)  // 0**k = 0 far ALL k 
		return 0.0;
	if (k==0)			// x**0 = 1
		return 1.0;
	for(y=1,i=0;i<abs(k);i++)
		y *= x;
	if (k>0) 
		return y;
	else
		return 1/y;		// can't be zero since x has been tested.
}				// FIN raisepower()
// *****************************************************************
