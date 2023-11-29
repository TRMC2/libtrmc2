// SPDX-License-Identifier: LGPL-3.0-or-later
/* vim: set ts=4 sw=4 noet:
 *
 * TrmcLin.c: Platform dependent functions, Linux version.
 *
 * If compiled with -DRASPBERRY_PI, the GPIO connector of the Raspberry
 * Pi will be used for communicating with the TRMC2:
 *   - clock:    GPIO17 (pin 11 of the GPIO connector)
 *   - data_in:  GPIO18 (pin 12)
 *   - data_out: GPIO27 (pin 13)
 * The library then has to be linked against libgpiod (compiler option
 * -lgpiod).
 *
 * If the option -DRASPBERRY_PI is _not_ provided, we assume this is a
 * PC with standard serial ports, and we use the control lines of either
 * of the first two ports (COM1/ttyS0, COM2/ttyS1) as follows:
 *   - clock:    DTR (pin 4 of the DE-9 connector)
 *   - data_in:  CTS (pin 8)
 *   - data_out: RTS (pin 7)
 *
 * For testing purposes, this can be compiled with -DNORMAL_SCHEDULE.
 * Then the resulting program will be scheduled as a normal (not
 * real-time) process. Timings will not be very good but this will
 * prevent bugs from locking hard your system.
 */

#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <sys/time.h>
#ifndef RASPBERRY_PI
# include <sys/io.h>
#endif
#include <sys/mman.h>
#include "Trmc.h"			/* for error codes */
#include "TrmcDef.h"		/* definition of the VARTRMC type */
#include "TrmcRunLib.h"		/* for SynchroCall[12]() */
#include "TrmcPlatform.h"	/* functions exported by this file */
#ifdef RASPBERRY_PI
# include <time.h>
# include <gpiod.h>
#endif

#ifdef RASPBERRY_PI
# define GPIO_CHIP_NAME	"gpiochip0"
# define CONSUMER		"libtrmc2"
# define PIN_CLOCK		17  /* GPIO17, pin 11 */
# define PIN_DATA_IN	18  /* GPIO18, pin 12 */
# define PIN_DATA_OUT	27  /* GPIO27, pin 13 */
#else
/* I/O space of the serial ports. */
# define BASE_COM1	0x3f8
# define BASE_COM2	0x2f8
# define COM_LENGTH	0x8
# define MCR_OFFSET	0x4
# define MSR_OFFSET	0x6
#endif

/***********************************************************************
 * Communication through the serial port.
 */

#ifdef RASPBERRY_PI
static struct gpiod_chip *gpio_chip;
static struct gpiod_line *line_clock, *line_data_in, *line_data_out;
#else
/* Addresses of the MCR and MSR registers of the serial port. */
static unsigned short mcr, msr;
#endif

#ifdef RASPBERRY_PI
/*
 * Delay for the requested number of microseconds.
 *
 * The standard functions usleep(), nanosleep() and clock_nanosleep()
 * suspend the calling process and thus end up sleeping way longer than
 * requested. This, instead, delays by busy-waiting.
 */
static void delay_us(short delay)
{
	if (delay < 1) return;
	struct timespec end, now;
	clock_gettime(CLOCK_MONOTONIC, &end);
	end.tv_nsec += delay * 1000L;
	end.tv_sec  += end.tv_nsec / 1000000000;
	end.tv_nsec %= 1000000000;
	do
		clock_gettime(CLOCK_MONOTONIC, &now);
	while (now.tv_sec < end.tv_sec
			|| (now.tv_sec == end.tv_sec
				&& now.tv_nsec < end.tv_nsec));
}
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
#ifdef RASPBERRY_PI
	gpiod_line_set_value(line_data_out, d);
	gpiod_line_set_value(line_clock, 0);
	delay_us(delay - 1);
	*r0 = !!gpiod_line_get_value(line_data_in);
	gpiod_line_set_value(line_clock, 1);
	delay_us(delay - 1);
	*r1 = !!gpiod_line_get_value(line_data_in);
#else
	int i;
	for(i=0; i<delay; i++)
		outb(d<<1 | 0, mcr);     /* data_out = d; clock = 0; */
	*r0 = !(inb(msr) & 0x10);  /* *r0 = data_in;           */
	for(i=0; i<delay; i++)
		outb(d<<1 | 1, mcr);     /* data_out = d; clock = 1; */
	*r1 = !(inb(msr) & 0x10);  /* *r1 = data_in;           */
#endif
}     // FIN void SendBitPlatform(char d, short *r0, short *r1, short delay)
// *************************************************************************

/* Send final 0 and delay. */
void SendFinalPlatform(short delay)
{
#ifdef RASPBERRY_PI
	gpiod_line_set_value(line_clock, 0);
	gpiod_line_set_value(line_data_out, 0);
	delay_us(delay - 1);
#else
	while (delay--)
		outb(0, mcr);
#endif
}     // FIN void SendFinalPlatform(short delay)
// *************************************************************************

/***********************************************************************
 * Time management.
 */

static int platform_initialized;
static struct itimerval interval;
static sigset_t sigset_alrm;
static enum {
	TIMER_START, WAIT_FIRST, WAIT_SECOND
} timer_state = TIMER_START;
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

/* Add some microseconds to a struct timeval. */
static void add_delay(struct timeval *ptv, int delay_us)
{
	ptv->tv_usec += delay_us;
	while (ptv->tv_usec >= 1000000) {
		ptv->tv_usec -= 1000000;
		ptv->tv_sec += 1;
	}
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
	static struct timeval scheduled_time = {
		0, 0
	};	/* time of scheduled call to SynchroCall1() */
	struct timeval now;
	int delay;

	(void) signum;	/* unused */

	switch (timer_state) {

		/* Very first call after BeatPlatform(). */
		case TIMER_START:
			/* Pretend we were scheduled to run now. */
			gettimeofday(&scheduled_time, NULL);
			timer_state = WAIT_FIRST;
			/* fallthrough */

		/* Time to call SynchroCall1(). */
		case WAIT_FIRST:
			if (!timer_on) return;
			if (SynchroCall1(trmc_globals) != 0) return;

			/* Schedule call to SynchroCall2(). */
			interval.it_value.tv_usec = 4000;
			setitimer(ITIMER_REAL, &interval, NULL);
			timer_state = WAIT_SECOND;
			break;

		/* Time to call SynchroCall2(). */
		case WAIT_SECOND:
			SynchroCall2(trmc_globals);
			if (!timer_on) return;

			/* How long before next call to SynchroCall1()? */
			add_delay(&scheduled_time, timer_period);
			gettimeofday(&now, NULL);
			delay = 1000000 * (scheduled_time.tv_sec - now.tv_sec)
					+ (scheduled_time.tv_usec - now.tv_usec);

			/*
			 * If we got out of sync (maybe the clock was stepped by the
			 * user), reset the scheduled time.
			 */
			if (delay < 1000 || delay >= timer_period) {
				delay = timer_period - 4000;
				scheduled_time = now;
				add_delay(&scheduled_time, delay);
			}

			/* Schedule call to SynchroCall1(). */
			interval.it_value.tv_usec = delay;
			setitimer(ITIMER_REAL, &interval, NULL);
			timer_state = WAIT_FIRST;
			break;
	}
}

/*
 * Start periodic timer.
 * First call to SynchroCall1() scheduled in 1 ms.
 */
int BeatPlatform(void)
{
	if (!platform_initialized)
		return _TIMER_NOT_CAPABLE;
	interval.it_value.tv_usec = 1000;
	timer_on = 1;
	timer_state = TIMER_START;
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
	if (timer_state == TIMER_START || timer_state == WAIT_FIRST)
		setitimer(ITIMER_REAL, &interval, NULL);
	sigprocmask(SIG_UNBLOCK, &sigset_alrm, NULL);
}


/***********************************************************************
 * Initialization.
 */

/* Called via atexit() upon termination. */
static void terminate(void)
{
	StopTRMC();
#ifdef RASPBERRY_PI
	gpiod_chip_close(gpio_chip);
#endif
}

/* Handler for deadly signals. */
static void quit_callback(int signum)
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

	/*
	 * Make this function idempotent: StartTRMC() may call us again if,
	 * the previous time it was called, it failed to establish
	 * communication with the TRMC2.
	 */
	if (platform_initialized)
		return _RETURN_OK;

#ifdef RASPBERRY_PI
	gpio_chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
	if (!gpio_chip)
		return _CANNOT_FIND_GPIO_CHIP;
	line_clock = gpiod_chip_get_line(gpio_chip, PIN_CLOCK);
	line_data_in  = gpiod_chip_get_line(gpio_chip, PIN_DATA_IN);
	line_data_out = gpiod_chip_get_line(gpio_chip, PIN_DATA_OUT);
	if (!(line_clock && line_data_in && line_data_out))
		return _CANNOT_FIND_GPIO_LINE;
	if (gpiod_line_request_output_flags(line_clock, CONSUMER,
			GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW, 0) == -1)
		return _CANNOT_RESERVE_GPIO_LINE;
	if (gpiod_line_request_input(line_data_in, CONSUMER) == -1)
		return _CANNOT_RESERVE_GPIO_LINE;
	if (gpiod_line_request_output_flags(line_data_out, CONSUMER,
			GPIOD_LINE_REQUEST_FLAG_ACTIVE_LOW, 0) == -1)
		return _CANNOT_RESERVE_GPIO_LINE;
#else
	/* Get write permission on the I/O space of the serial port. */
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

	platform_initialized = 1;
	return _RETURN_OK;
}
