/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009-2014

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  1.0  6  Jan 2010 - initial release
  1.1  12 Feb 2010 - fixed leap year calculation error
  1.2  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  1.3  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                     status, updated examples for Arduino 1.0, fixed ARM
                     compatibility issues, added TimeArduinoDue and TimeTeensy3
                     examples, add error checking and messages to RTC examples,
                     add examples to DS1307RTC library.
  1.4  5  Sep 2014 - compatibility with Arduino 1.5.7
*/

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


#include "TimeLib.h"


volatile unsigned int timemillisoverflows = 0;

static uint32_t sysTime = 0;
static uint32_t prevMillis = 0;
static uint32_t nextSyncTime = 0;
double millifracs = 0;
unsigned long precF_CPU=F_CPU;

uint8_t ICPBypass = 0;
int ocrtmp = 40000;
int Timervar = 0;
int Sourcevar = 0;
int intPinvar=0;
int intPin = 0;
int ovffac=0;
int initTimer(int Timer, int Source, int intPin) {
Timervar=Timer;
Sourcevar=Source;
intPinvar=intPin;

  if (intPin == 2 || intPin == 3)
  
  attachInterrupt(digitalPinToInterrupt(intPin), resetTimer, RISING);
  else if (intPin == 8 && Timer != 1)
  {
    cli();
    TCCR1B |= (1 << ICNC1);
    TIMSK1 |= (1 << ICIE1);
    sei();

  }
  else if (intPin == 8 && Timer == 1)
    ICPBypass = 0;
  else return 0;
  //source=0 = Mainclock;
  if (Timer == 0) {
    if (Source == 0)
    { cli();
      TCCR0A = 0;
      TIMSK0 = 0;
      TCCR0B |= (0 << CS00);    // clock
      TCCR0B |= (0 << CS01);
      TCCR0B |= (1 << CS02);
      TIMSK0 |= (1 << TOIE0);
      millifracs=1000.0f/(double)(precF_CPU/256);
      ovffac=256;
	  sei();
    } else if (Source == 1)
    {
      TCCR0B |= (0 << CS00);    // clock
      TCCR0B |= (1 << CS01);
      TCCR0B |= (1 << CS02);
      TIMSK0 |= (1 << TOIE0);
	  ovffac=256;
      millifracs = 1000.0f/32768;
    }
  } else if (Timer == 1) {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
 if (Source == 0)
    { 
    TCCR1B |= (0 << CS10);    // clock
    TCCR1B |= (0 << CS11);
    TCCR1B |= (1 << CS12);

    TIMSK1 |= (1 << OCIE1A);
	 OCR1A = 30000-1; //50 Overflows=1Sekunde
	 millifracs= 1000.0f/(double)(F_CPU/256);
      ovffac=30000;
	  if (intPin == 8){
      TCCR1B |= (1 << ICNC1);
    TIMSK1 |= (1 << ICIE1);
    }
} else  if (Source == 1)
    { 
    OCR1A = ocrtmp;            // compare match register 16MHz/256/2Hz
    //  TCCR1A |= (1 << WGM12);
    // icp4
    TCCR1B |= (1 << CS10);    // clock
    TCCR1B |= (1 << CS11);
    TCCR1B |= (1 << CS12);
    // TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
    
	
	if (intPin == 8){
      TCCR1B |= (1 << ICNC1);
    TIMSK1 |= (1 << ICIE1);
    }
	
	}
	sei();
  } else if (Timer == 2) {
 if (Source == 0)
    { cli();
      TCCR2A = 0;
	  TCCR2B = 0;
      TIMSK2 = 0;
      TCCR2B |= (1 << CS00);    // clock
      TCCR2B |= (1 << CS01);
      TCCR2B |= (0 << CS02);
      TIMSK2 |= (1 << TOIE0);
      millifracs=1000.0f/(precF_CPU/64L);
      ovffac=256;
	  sei();
    } else if (Source == 1)
    {
      TCCR0B |= (0 << CS20);    // clock
      TCCR0B |= (1 << CS21);
      TCCR0B |= (1 << CS22);
      TIMSK0 |= (1 << TOIE0);
	  ovffac=256;
      millifracs = 1000.0f/32768;
    }

  } else return 0;

}

#if  TIMERVAR==1
ISR(TIMER1_CAPT_vect)          // timer compare interrupt service routine
{
resetTimer();
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{
  sei();
  if (Sourcevar == 1)
  TCNT1 = 0;
else 
{
	timemillisoverflows++;
TCNT1=0;
	} 
 cli();
}
#elif TIMERVAR==0
ISR(TIMER0_OVF_vect)
{
//  if (timemillisoverflows < 250)
    timemillisoverflows++;
 // else
   // timemillisoverflows = 0;
  //TCNT0=0;
}
#else
ISR(TIMER2_OVF_vect)
{
//  if (timemillisoverflows < 250)
    timemillisoverflows++;
 // else
   // timemillisoverflows = 0;
  //TCNT0=0;
}
#endif
//double millifracs=0.030517578125;
unsigned long  precmillis()
{
	  unsigned int clocks =0;
  cli();
if(Timervar==0)
  clocks = TCNT0;
else
clocks=TCNT1;	

  uint32_t ofv = timemillisoverflows;
  sei();
  //unsigned int x=((uint16_t)ofv*256)+clocks;

  long x = (((ofv * ovffac) + clocks) * millifracs) + .5;
  x+=sysTime*1000L;
  return x;
}
void resetTimer() {
  cli();
  sysTime++;
  if(Timervar==0)
	   TCNT0 = 0;
   else   if(Timervar==1)
	   TCNT1 = 0;
  timemillisoverflows = 0;
  sei();
}
static tmElements_t tm;          // a cache of time elements
static time_t cacheTime;   // the time the cache was updated
static uint32_t syncInterval = 300;  // time sync will be attempted after this many seconds

void refreshCache(time_t t) {
  if (t != cacheTime) {
    breakTime(t, tm);
    cacheTime = t;
  }
}

int hour() { // the hour now
  return hour(now());
}

int hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;
}

int hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now());
}

int hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if ( tm.Hour == 0 )
    return 12; // 12 midnight
  else if ( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t isAM() { // returns true if time now is AM
  return !isPM(now());
}

uint8_t isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);
}

uint8_t isPM() { // returns true if PM
  return isPM(now());
}

uint8_t isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12);
}

int minute() {
  return minute(now());
}

int minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;
}

int second() {
  return second(now());
}

int second(time_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int day() {
  return (day(now()));
}

int day(time_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int weekday() {   // Sunday is day 1
  return  weekday(now());
}

int weekday(time_t t) {
  refreshCache(t);
  return tm.Wday;
}

int month() {
  return month(now());
}

int month(time_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int year() {  // as in Processing, the full four digit year: (2009, 2010 etc)
  return year(now());
}

int year(time_t t) { // the year for the given time
  refreshCache(t);
  return tmYearToCalendar(tm.Year);
}

/*============================================================================*/
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

static  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // API starts months from 1, this array starts from 0

void breakTime(time_t timeInput, tmElements_t &tm) {
  // break the given time_t into time components
  // this is a more compact version of the C library localtime function
  // note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1

  year = 0;
  days = 0;
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days = 0;
  month = 0;
  monthLength = 0;
  for (month = 0; month < 12; month++) {
    if (month == 1) { // february
      if (LEAP_YEAR(year)) {
        monthLength = 29;
      } else {
        monthLength = 28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
      break;
    }
  }
  tm.Month = month + 1;  // jan is month 1
  tm.Day = time + 1;     // day of month
}

time_t makeTime(tmElements_t &tm) {
  // assemble time elements into time_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds = tm.Year * (SECS_PER_DAY * 365);
  for (i = 0; i < tm.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm.Year)) {
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i - 1]; //monthDay array starts from 0
    }
  }
  seconds += (tm.Day - 1) * SECS_PER_DAY;
  seconds += tm.Hour * SECS_PER_HOUR;
  seconds += tm.Minute * SECS_PER_MIN;
  seconds += tm.Second;
  return (time_t)seconds;
}
/*=====================================================*/
/* Low level system time functions  */

static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

#ifdef TIME_DRIFT_INFO   // define this to get drift data
time_t sysUnsyncedTime = 0; // the time sysTime unadjusted by sync
#endif

int setupTimer(int Mode, int Pin, int Timer)
//Mode 0->INT0 // 1->INT1// 2->PCINT //3->TimerOwn
//Timer0->T0//Timer1->ICP1//
{
  if (Mode == 0) {


  } else if (Mode == 1) {


  } else if (Mode == 2) {


  } else return 0;



}

time_t now() {
  // calculate number of seconds passed since last call to now()
  cli();
time_t tmptime=	sysTime++;
	sei();
  return (time_t)tmptime;
}

void setTime(time_t t) {
#ifdef TIME_DRIFT_INFO
  if (sysUnsyncedTime == 0)
    sysUnsyncedTime = t;   // store the time of the first call to set a valid Time
#endif

  sysTime = (uint32_t)t;
  nextSyncTime = (uint32_t)t + syncInterval;
  Status = timeSet;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
}

void setTime(int hr, int min, int sec, int dy, int mnth, int yr) {
  // year can be given as full four digit year or two digts (2010 or 10 for 2010);
  //it is converted to years since 1970
  if ( yr > 99)
    yr = yr - 1970;
  else
    yr += 30;
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  setTime(makeTime(tm));
}

void adjustTime(long adjustment) {
  sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
timeStatus_t timeStatus() {
  now(); // required to actually update the status
  return Status;
}

void setSyncProvider( getExternalTime getTimeFunction) {
  getTimePtr = getTimeFunction;
  nextSyncTime = sysTime;
  now(); // this will sync the clock
}

void setSyncInterval(time_t interval) { // set the number of seconds between re-sync
  syncInterval = (uint32_t)interval;
  nextSyncTime = sysTime + syncInterval;
}
