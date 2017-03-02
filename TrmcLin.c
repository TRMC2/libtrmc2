/* vim: set ts=4 sw=4 noet:
 *
 * TrmcLin.c: Platform dependent functions, Linux version.
 *
 * For testing purposes, this can be compiled with -DNORMAL_SCHEDULE.
 * Then the resulting program will be scheduled as a normal (not
 * real-time) process. Timings will not be very good but this will
 * prevent bugs from locking hard your system.
 */

/*
 * Assume the Raspberry Pi is the only ARM platform this will ever be
 * compiled for.
 */
#ifdef __arm__
#define RASPBERRY
#endif

#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/io.h>
#include <sys/mman.h>
#include "Trmc.h"			/* for error codes */
#include "TrmcDef.h"		/* definition of the VARTRMC type */
#include "TrmcRunLib.h"		/* for SynchroCall[12]() */
#include "TrmcPlatform.h"	/* functions exported by this file */
#ifdef RASPBERRY
#include <wiringPi.h>
#endif

/* I/O space of the serial ports. */
#ifndef RASPBERRY
#define BASE_COM1	0x3f8
#define BASE_COM2	0x2f8
#define COM_LENGTH	0x8
#define MCR_OFFSET	0x4
#define MSR_OFFSET	0x6
#else
#define PIN_CLOCK       0  /*  en wiringPI, physical: 11; BCM 17 */
#define PIN_READ        1  /*  en wiringPI, physical: 12; BCM 18 */
#define PIN_WRITTEN     2  /*  en wiringPI, physical: 13; BCM 27 */
#endif

/***********************************************************************
 * Communication through the serial port.
 */

/* Addresses of the MCR and MSR registers of the serial port. */
#ifndef RASPBERRY
static unsigned short mcr, msr;
#endif
/*
 * This function sends the bit d through the data_out line and sets the
 * clock line to 0 then 1. It gets TWO responses, r0 and r1, on the
 * data_in line: one for each state of the clock. r0 is the data. r1 is
 * for error control: it should be the converse of the previous data.
 * 
 * The I/O registers and relevant lines are:
 *		MCR: bit 0 = DTR = clock, bit 1 = RTS = data_out
 *		MSR: bit 4 = CTS = !data_in
 */
void SendBitPlatform(char d, short *r0, short *r1, short delay)
{
  d = (d != 0);		/* d should be 0 or 1 */
#ifndef RASPBERRY
  int i;
  for(i=0; i<delay; i++)
    outb(d<<1 | 0, mcr);
  *r0 = !(inb(msr) & 0x10);
  for(i=0; i<delay; i++)
    outb(d<<1 | 1, mcr);
  *r1 = !(inb(msr) & 0x10);
#else
  // on ecrit delay d fois dans MCR(1) = data_out et simultanement 0 dans MCR(0)= clock
  // on lit MSR(4)= datain ==> r0
  // on ecrit delay d fois dans MCR(1) = data_out et simultanement 1 dans MCR(0)= clock
  // on lit MSR(4)= datain ==> r1
  digitalWrite(PIN_WRITTEN,!d);  // pins use negative logic
  digitalWrite(PIN_CLOCK,!0);
  delayMicroseconds(delay);
  int msr = digitalRead(PIN_READ);
  *r0 = !!(msr);  // XXX: testing
  digitalWrite(PIN_CLOCK,!1);
  delayMicroseconds(delay);
  msr = digitalRead(PIN_READ);
  *r1 = !!(msr);
#endif
}     // FIN void SendBitPlatform(char d, short *r0, short *r1, short delay)
// *************************************************************************

/* Send final 0 and delay. */
void SendFinalPlatform(short delay)
{
#ifndef RASPBERRY
	while (delay--)
		outb(0, mcr);
#else
  digitalWrite(PIN_CLOCK,!0);
  digitalWrite(PIN_WRITTEN,!0);
  delayMicroseconds(delay);
#endif
}     // FIN void SendFinalPlatform(short delay)
// *************************************************************************

/***********************************************************************
 * Time management.
 */

static struct itimerval interval;
static sigset_t sigset_alrm;
static enum { WAIT_FIRST, WAIT_SECOND } timer_state = WAIT_FIRST;
static int timer_period;
static int timer_on;
static VARTRMC *trmc_globals;

/*
 * Returns the time elapsed in milliseconds since the first call.
 * Used only for differences.
 */
int ElapsedTimePlatform(void)
{
	static struct timeval first = {0, 0};	/* time of first call */
	struct timeval now;

	gettimeofday(&now, NULL);
	if (first.tv_sec == 0) first = now;		/* this is the first call */
	return (now.tv_sec	- first.tv_sec ) * 1000
		 + (now.tv_usec - first.tv_usec) / 1000;
}

/*
 * This function is called as a handler of SIGALRM when a timer expires.
 * It calls alternatively SynchroCall1() and SynchroCall2() and restarts
 * the timer in such a way that calls to SynchroCall1() are periodic
 * with period timer_period and SynchroCall2() is called 4 ms after
 * SynchroCall1().
 *
 * Note: The delay between SynchroCall1() and SynchroCall2() is actually
 * 10 ms, i.e. the scheduler resolution on Linux/i386.
 */
static void timer_callback(int signum)
{
	static struct timeval last = {0, 0};	/* call to SynchroCall1() */
	struct timeval now;
	int delay;

	(void) signum;	/* unused */

	if (timer_state == WAIT_FIRST) {
		if (!timer_on) return;
		gettimeofday(&last, NULL);
		if(SynchroCall1(trmc_globals) == 0) {

			/* Schedule call to SynchroCall2(). */
			interval.it_value.tv_usec = 4000;
			setitimer(ITIMER_REAL, &interval, NULL);
			timer_state = WAIT_SECOND;
			return;

		}
	} else {
		SynchroCall2(trmc_globals);
	}

	/* Schedule call to SynchroCall1(). */
	if (!timer_on) return;
	gettimeofday(&now, NULL);
	delay = timer_period
		- 1000000 * (now.tv_sec - last.tv_sec)
		- (now.tv_usec - last.tv_usec);
	if (delay < 1000) delay = 1000;
	interval.it_value.tv_usec = delay;
	setitimer(ITIMER_REAL, &interval, NULL);
	timer_state = WAIT_FIRST;
}

/*
 * Start periodic timer.
 * First call to SynchroCall1() scheduled in 1 ms.
 */
int BeatPlatform(void)
{
	interval.it_value.tv_usec = 1000;
	timer_on = 1;
	timer_state = WAIT_FIRST;
	setitimer(ITIMER_REAL, &interval, NULL);
	return	_RETURN_OK;
}

/* Stop timer after the next call to SynchroCall2(). */
void StopTimerPlatform(void)
{
	timer_on = 0;
	interval.it_value.tv_usec = 0;

	/* Block SIGALRM to avoid race condition. */
	sigprocmask(SIG_BLOCK, &sigset_alrm, NULL);
	if (timer_state == WAIT_FIRST)
		setitimer(ITIMER_REAL, &interval, NULL);
	sigprocmask(SIG_UNBLOCK, &sigset_alrm, NULL);
}


/***********************************************************************
 * Initialization.
 */

/* Called via atexit() upon termination. */
void terminate(void)
{
	StopTRMC();
}

/* Handler for deadly signals. */
void quit_callback(int signum)
{
	(void) signum;
	exit(EXIT_FAILURE);  /* exit() calls terminate() */
}

/* Platform specific initialization. */
int InitPlatform(void *ptr)
{
	VARTRMC *vartrmc = ptr;
#ifndef NORMAL_SCHEDULE
	struct sched_param priority;
#endif
	struct sigaction action;

	/* Get write permission on the I/O space of the serial port. */
#ifndef RASPBERRY
	unsigned long base;
	switch (vartrmc->com1) {
		case _COM1: base = BASE_COM1; break;
		case _COM2: base = BASE_COM2; break;
		default: return _INVALID_COM;
	}
	if (ioperm(base, COM_LENGTH, 1) == -1)
		return _COM_NOT_AVAILABLE;
	mcr = base + MCR_OFFSET;
	msr = base + MSR_OFFSET;
#else
	if (wiringPiSetup() == -1)
	  return _CANT_SETUP_WIRINGPI;
	pinMode(PIN_CLOCK,OUTPUT);
	pinMode(PIN_READ,INPUT);
	pinMode(PIN_WRITTEN,OUTPUT);
#endif
	/* Initialize globals. */
	trmc_globals = vartrmc;
	timer_period = 1000 * vartrmc->periodinms;
	sigemptyset(&sigset_alrm);
	sigaddset(&sigset_alrm, SIGALRM);

#ifndef NORMAL_SCHEDULE
	/* Lock memory in RAM in order to avoid page faults. */
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
		return _TIMER_NOT_CAPABLE;

	/* Become a real-time process. */
	priority.sched_priority = 10;
	if (sched_setscheduler(0, SCHED_FIFO, &priority) == -1)
		return _TIMER_NOT_CAPABLE;
#endif

	/* Catch SIGALRM. */
	action.sa_handler = timer_callback;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	if (sigaction(SIGALRM, &action, NULL) == -1)
		return _TIMER_NOT_CAPABLE;

	/* Catch some deadly signals. */
	action.sa_handler = quit_callback;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGHUP, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);

	/* Stop the TRMC2 on termination. */
	atexit(terminate);

	return _RETURN_OK;
}
