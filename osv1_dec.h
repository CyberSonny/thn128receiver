/********************************************************************************************
 Oregon Scientific v 1.0 decoder routine for THN128 sensors and RLP434A 433.92MHz receiver
 (c) by Alexander Yerezeyev 2007-2009  
  URL: http://alyer.frihost.net
  email: wapbox@bk.ru
  ICQ:  305-206-239

  MCU: ATMEL AVR ATMEGA16 @ 16MHz
********************************************************************************************/

/*******************************************************************
  Declaring structure of Oregon Scientific THN128 data array
  see http://alyer.frihost.net for details
*******************************************************************/
typedef struct
{
  union
  {
    unsigned char Byte0;
    struct 
    {
      unsigned char Dumy0:6,
                    Channel:2; // 7-6 bits

    };
  };
  union
  {
    unsigned char Byte1;
    struct 
    {
      unsigned char Th3:4, 
                    Th2:4;
    };
  };
  union
  {
    unsigned char Byte2;
    struct 
    {
      unsigned char Th1:4,
                    Dummy1:1,
                    Sign:1,
                    HH:1,
                    LowBat:1; // 7bit                                                                            
    };
  };
  union
  {
    unsigned char Byte3;
    struct 
    {
      unsigned char CRC; // 7-6 bits
    };
  };
}OS1Data;

/*******************************************************************
  Declaring structure of THN128 incoming data storage
*******************************************************************/
typedef struct
{
  signed int Temperature10x;
  signed int Temperature10xMin;
  signed int Temperature10xMax;
  unsigned char Humidity;     // it is for my own sensor only
  unsigned char HumidityMin;  // it is for my own sensor only
  unsigned char HumidityMax;  // it is for my own sensor only
  bool LowBat;  
  unsigned char Status;
  unsigned int TimeStamp;
  unsigned int UpdatePeriod;
} THN128storage;

// Global structures with the received data from 3 channels
extern THN128storage Sensor1, Sensor2, Sensor3;
// New packet received flag
extern volatile bool RFData_received;
// Check old signals routine
extern void CheckSensorAlive();