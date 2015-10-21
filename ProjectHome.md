This project includes receiver and decoder parts for Oregon Scientific wireless sensor THN128.
The transmitted from THN128 signal is receipted with RLP434A 433.92MHz ASK receiver that is conected to INT1 pin of ATmega16 MCU. Then signal is decoded using Manchester data coding scheme and printed via UART (RS-232 if MAX232 IC is connected). So it is simple 433.92Mhz to RS-232 converter for Oregon Scientific THN128 sensor. You can modify this software for another sensors that uses Oregon Scientific version 1 protocol.
It is also possible to receive ver 2.1 protocol, but I dont share this software fo free... I hope that you inderstand me.
The presented software is written with plain C language in IAR5.11b compiler. But I think it is easy to modify sources for GCC or other compilers.

Best regards!