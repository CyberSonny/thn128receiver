#include <iom16.h>
#include <intrinsics.h>
#include <pgmspace.h>
#include <stdbool.h>
#define outp(val, reg)  (reg = val)
#define inp(reg)        (reg)

#define cli()           __disable_interrupt();
#define sei()           __enable_interrupt();

#define sbi(reg,bit)  (reg |= (1<<bit))   //<! set bit in port
#define cbi(reg,bit)  (reg &= ~(1<<bit))  //<! clear bit in port
#define sbr(reg,bit)  (reg |= (1<<bit))   //<! set bit in port
#define cbr(reg,bit)  (reg &= ~(1<<bit))  //<! clear bit in port
#define _BV(x)	   (1<<x)

// CPU frequency is 16 MHZ!!!!!!
#define XTALL			16.0

#define nop() __no_operation();
#define	_delay_us(us)	__delay_cycles (XTALL * us);
#define _delay_ms(ms)	_delay_us (1000 * ms) 


typedef unsigned char u8;
typedef unsigned short u16;