// version adaptee a sauvegarde cvs TrmcProto.h

 
//************************************************************* 
//                  LOCAL FUNCTION PROTOTYPES  
//************************************************************* 
int	WriteFifoMeasure(ACHANNEL *,int );
int	ReadFifoMeasure(ACHANNEL *,AMEASURE *);
int  createachannel(CHANNELPARAMETER *);
int locatechannel(int ,CHANNELPARAMETER *);
void newname(char name[_LENGTHOFNAME]);
int MakeAllChannels();

//-------------------------------------------------------- 
//   prototypes  for all  boards   
//-------------------------------------------------------- 
long int InitBoard_Regul(ABOARD *board);
long int InitBoard_A(ABOARD *board);
long int InitBoard_B(ABOARD *board);
long int InitBoard_C(ABOARD *board);
long int InitBoard_D(ABOARD *board);
long int InitBoard_E(ABOARD *board);
long int InitBoard_F(ABOARD *board);
long int InitBoard_G(ABOARD *board);

long int CheckChannel_Regul(ACHANNEL *channel);
long int CheckChannelBoard_A(ACHANNEL *channel);
long int CheckChannelBoard_B(ACHANNEL *channel);
long int CheckChannelBoard_C(ACHANNEL *channel);
long int CheckChannelBoard_D(ACHANNEL *channel);
long int CheckChannelBoard_E(ACHANNEL *channel);
long int CheckChannelBoard_F(ACHANNEL *channel);
long int CheckChannelBoard_G(ACHANNEL *channel);

long int Calc_Regul(ABOARD *boardpt);
long int CalcBoard_A(ABOARD *boardpt);
long int CalcBoard_B(ABOARD *boardpt);
long int CalcBoard_C(ABOARD *boardpt);
long int CalcBoard_D(ABOARD *boardpt);
long int CalcBoard_E(ABOARD *boardpt);
long int CalcBoard_F(ABOARD *boardpt);
long int CalcBoard_G(ABOARD *boardpt);

//-------------------------------------------------------- 
//    macro and  functions   defined for all  boards   
//-------------------------------------------------------- 


double	MeasuredValue(ABOARD *boardpt);

void BigPb(int in,int from);
