
#ifdef powerc
#include "manip.h"
#else
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <malloc.h>
#include <math.h> 
#endif

#define _DLL

#include "Trmc.h" 
#include "TrmcRunLib.h"

#include "TrmcDef.h"
#include "TrmcProto.h"
#include "TrmcBoard.h"
#include "TrmcDac.h"
 

long int InitBoard_DacB(ABOARD *board)
{
	board->Data = 0;
	return _RETURN_OK ;
}				// FIN InitBoard_DacB()
// ****************************************************************

long int CheckChannel_DacB(ACHANNEL *Channel)
// allowed mode = _NOT_USED_MODE _FIXE_RANGE_MODE _FIXE_VOLTAGE_MODE
// the parameter Channel->ValueRangeV is fitted in [-10,10] this
// is the value in Volts to impose on the dacB
{
	if (Channel->parameter.SubAddress >= _NB_SUBADD_DACB)
		return _INVALID_SUBADDRESS;

	//				Check mode
	// all modes in case are ok, default is not;
	switch (Channel->parameter.Mode)
	{
	case _INIT_MODE:
		Channel->NumRangeI = 0;
		Channel->NumRangeV = 0;
		Channel->parameter.PreAveraging = _INIT_PREAVERAGING_DACB;
		Channel->parameter.ValueRangeV = 0;
		Channel->parameter.ValueRangeI = _INIT_VALUERANGE_DACB;
		Channel->parameter.FifoSize = _FIFOLENGTH; // unusefull for board B

		Channel->parameter.Mode = _INIT_MODE_DACB;
	case _FIX_RANGE_MODE:
	case _FIX_VOLTAGE_MODE:
	case _NOT_USED_MODE:
		break;
	default:
		return _INVALID_MODE; 
	}
	
	Channel->parameter.ValueRangeV = 0;

	if (Channel->parameter.ValueRangeI > 10)
	{
		Channel->parameter.ValueRangeI = 10;
		return _CHANNEL_HAS_BEEN_MODIFIED;
	}
	if (Channel->parameter.ValueRangeI < -10)
	{
		Channel->parameter.ValueRangeI = -10;
		return _CHANNEL_HAS_BEEN_MODIFIED;
	}
	return _RETURN_OK ;
}				// FIN CheckChannel_DacB()
// ****************************************************************

long int Calc_DacB(ABOARD *boardpt)
{
	ACHANNEL *Channel;

	Channel=boardpt->Channels[0];	// DACB has only one channel

	if (Channel->parameter.Mode == _NOT_USED_MODE)
	{
		boardpt->ChannelTreated = -1; // Ie no chanel treated
		return _RETURN_OK;
	}
	boardpt->ChannelTreated = 0;

	boardpt->Data=(int) (32767.*(Channel->parameter.ValueRangeI/10.));

	Channel->mes.Time = boardpt->Time;
	Channel->mes.MeasureRaw = Channel->parameter.ValueRangeI;
	Channel->mes.Status = 1;

	return _RETURN_OK ;
}				// FIN Calc_DacB()
// ****************************************************************

