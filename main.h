/********************************************************************************************
 Oregon Scientific v 1.0 decoder routine for THN128 sensors and RLP434A 433.92MHz receiver
 (c) by Alexander Yerezeyev 2007-2009  
  URL: http://alyer.frihost.net
  email: wapbox@bk.ru
  ICQ:  305-206-239

  MCU: ATMEL AVR ATMEGA16 @ 16MHz
********************************************************************************************/
#include "compiler.h"
// if TRACEMODE is define then we print packet information via UART
#define TRACEMODE 