/*
   TimeRTC.pde
   example code illustrating Time library with Real Time Clock.

*/
#include <TimeLib.h>



#include <Wire.h>
//#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

void setup()  {
  Serial.begin(115200);
//initTimer(int Timer, in Source, int InterruptPin);
//Timer= 0,1 (Timer0, Timer1)
//Source= 0 MainClock, 1 InputTrigger T0/T1
//InterruptPin 0(INT0),1(INT1),8 (ICP1),99 (own Triggerpin with resetTimer())
//resetTimer(); 
//Set CountVars to 0; 
initTimer(1,0,8);
//resetTimer();
   unsigned long tmpcheck=F_CPU/256;
   Serial.println(tmpcheck);
   double millifracs= (double)1000.0f/tmpcheck;
printDouble(millifracs,10000);

delay(2000);
}
int lastprecmillis=0;
int lastmillis=0;
int sync=0;
void loop()
{

  long mymillis=millis();
  long myprecmillis=precmillis();
  
 // Serial.println(now());
 //Serial.println(myprecmillis-lastprecmillis);
 //lastprecmillis=myprecmillis;
//Serial.println(mymillis-lastmillis);
  Serial.println( myprecmillis-mymillis-sync);
   Serial.println( precmillis());

  delay(1000);
if(sync==0)
sync= myprecmillis-mymillis;
}
void printDouble( double val, unsigned int precision) {
  // prints val with number of decimal places determine by precision
  // NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
  // example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)

  Serial.print (int(val));  //prints the int part
  Serial.print("."); // print the decimal point
  unsigned int frac;
  if (val >= 0)
    frac = (val - int(val)) * precision;
  else
    frac = (int(val) - val ) * precision;
  Serial.println(frac, DEC) ;
}

