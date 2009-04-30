/********************************************************************************************
 Oregon Scientific v 1.0 decoder routine for THN128 sensors and RLP434A 433.92MHz receiver
 (c) by Alexander Yerezeyev 2007-2009  
  URL: http://alyer.frihost.net
  email: wapbox@bk.ru
  ICQ:  305-206-239
********************************************************************************************/

#include "compiler.h"
#include "maintime.h"
#include "osv1_dec.h"
u16 Counter10ms = 0x00;    // Timer that is counting 10ms ticks

//***************************************
//  OC1 OVF Timer routine
//***************************************
#pragma vector=TIMER1_COMPA_vect
__interrupt void INT_TIMER_DISPATCH(void) // interruption @ every 16us
{  
  static int counter, counter1, counter2;
    //Обработка таймеров
  if (++ counter1 == 625) {Counter10ms++; counter1=0;} // Increment counter each 10ms;
  if (++ counter != 0x30D4) return;  
  // this part is implemented each 200ms
  if (++ counter2 == 5) {CheckSensorAlive(); counter2=0;} // Increment counter each 1s;  
  counter=0;  
}