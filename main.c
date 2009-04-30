/********************************************************************************************
 Oregon Scientific v 1.0 decoder routine for THN128 sensors and RLP434A 433.92MHz receiver
 (c) by Alexander Yerezeyev 2007-2009  
  URL: http://alyer.frihost.net
  email: wapbox@bk.ru
  ICQ:  305-206-239

  MCU: ATMEL AVR ATMEGA16 @ 16MHz
********************************************************************************************/

#include "main.h"
#include "io.h"

#define MainTimerStart()\
  TCCR1B=(1<<WGM12)|(1<<CS00);\
  OCR1A=0x00FF;\
  sbi(TIMSK,OCIE1A);

char const __flash _Hello_string[]=\
"\r\n***********************************\r\n\
*** Oregon Scientific THN128 receiver\r\n\
*** v1.00 ATmega16\r\n\
*** (c)2009 by Alexander Yerezeyev\r\n\
*** e-mail: wapbox@bk.ru\r\n\
*** http://alyer.frihost.net\r\n\
*** ICQ: 305206239 \r\n\
************************************ \r\n";

int main( void )
{
  sbr (DDRD, PD5); // LED pin as output
  _delay_ms(200);
  cbr (PORTD, PD5); // LED OFF    
  #ifdef TRACEMODE
    uart_init();  
  #endif  
  cli();
  MainTimerStart();
  sbr (TIMSK, OCIE0); //enable OC T0 interrupts  
  sbr (MCUCR, ISC11); // Falling edge on INT1 generate an interrupt  
  cbr (MCUCR, ISC10);
  sbr(GICR, INT1);    // Enable INT1 processing   
  GIFR=(1<<INTF1);  // Clear INT1 IRQ  
  sei();
  #ifdef TRACEMODE
    printf_P(_Hello_string);
  #endif  
}
