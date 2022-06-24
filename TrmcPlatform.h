// SPDX-License-Identifier: LGPL-3.0-or-later
// Prototypes of funcions shared by TrmcRunLib and
// TrmcWin or TrmcMac or TrmcLinux

int  BeatPlatform(void);

int InitPlatform(void *vartrmc);

void StopTimerPlatform(void);

void SendBitPlatform(char d,short int *r0,short int *r1,
					 short int DelayCommTrmc);

void SendFinalPlatform(short int DelayCommTrmc);

int ElapsedTimePlatform(void); 
//  return the elapsed time in ms from an arbitrary but fixed origin
