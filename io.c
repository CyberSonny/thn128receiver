#include "compiler.h"

/****************************************************************************
**
** Putchar function needed for ICC
**
** Parameters: send_char, character to send
**
** Returns: Sent character
**
****************************************************************************/

unsigned int putchar(unsigned char send_char)
{
while (!(UCSRA & 0x20))
; /* wait xmit ready */
UDR = (unsigned char) send_char;
return(send_char);
}

unsigned int _low_level_get(void)
{
/* Wait for data to be received */
while ( !(UCSRA & (1<<RXC)) )
;
/* Get and return received data from buffer */
return UDR;
}

#define LINE_LENGTH 80          /* Change if you need */

#define In_DELETE 0x7F          /* ASCII <DEL> */
#define In_EOL '\r'             /* ASCII <CR> */
#define In_SKIP '\3'            /* ASCII control-C */
#define In_EOF '\x1A'           /* ASCII control-Z */

#define Out_DELETE "\x8 \x8"    /* VT100 backspace and clear */
#define Out_SKIP "^C\n"         /* ^C and new line */
#define Out_EOF "^Z"            /* ^Z and return EOF */
static void put_message(char *s)
{
  while (*s)
    putchar(*s++);
}


/****************************************************************************
**
** Initializes UART0
**
** Parameters: None
**
** Returns: None
**
****************************************************************************/
void uart_init(void)
{
  UBRRH = (unsigned char)(0);
  UBRRL = (unsigned char)(16);// 57600bps @ 16MHz
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN);
  UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //8bit-NONE-1stop
}


