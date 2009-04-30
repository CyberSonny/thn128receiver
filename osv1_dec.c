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
#include "osv1_dec.h"
#include "maintime.h"

#define deltaT 16
#define deltaT2 8
#define InRange(TVAL, TSETUP, dT) ((TVAL<=(TSETUP+dT))&&(TVAL>=(TSETUP-dT)))

#ifdef TRACEMODE
char const __flash _DecNum[]="%02d";
char const __flash _DecNum1[]="%1d";
char const __flash _FloatNum[]="%5.4f";
char const __flash _HexWordNum[]="%04X";
char const __flash _HexByteNum[]="%02X";
char const __flash _HexByte1[]="%01X";
char const __flash _Channel[]="\r\n Channel(1..3)#: %d";
char const __flash _Temperature[]="\r\n Temperature (x10): %d";
char const __flash _Humidity[]="\r\n Humidity: %02d";
char const __flash _Battery[]="\r\n Battery Status: ";
char const __flash _OK[]="OK";
char const __flash _Bad[]="Bad!";
char const __flash _Razer[]="\r\n";
char const __flash _Celsium[]="C";
char const __flash _Percent[]="%%";
char const __flash _RFPacket[]="\r\n RAW RF packet: ";
char const __flash _RAWtimings[]="\r\n RAW tTimings: ";
char const __flash _Timestamp[]="\r\n Timestamp (x10ms): %d";
char const __flash _CRC[]="\r\n CRC check: ";
#endif


const u8 TminH=0x1B; // (1750us) Tmin High Level for v1 protocol
const u8 TminL=0x13; // (1190us) Tmin Low Level for v1 protocol
const u8 TmaxH=0x31; // (1500us) Tmax High Level for v1 protocol
const u8 TmaxL=0x2B; // (1500us) Tmax Low Level for v1 protocol
const u8 Tout=0x80; // T time out for v1 protocol

const u8 TstartBitH=0x59;       //High startbit for v1 protocol
const u8 TstartBitL=0x42;       //First Low Startbit for v1 protocol
const u8 TstartBitL2short=0x56; //Second Short Low Startbit for v1 protocol
const u8 TstartBitL2long=0x69;  //Second Long Low Startbit for v1 protocol
static u8  INT1_cnt; // INT1 counter  

static bool Synchronised=false;
volatile bool RFData_received;
// receipt buffer
u8 RXData[4]={0};

#ifdef TRACEMODE
u8 RAWData[92]={0};
#define DUMP()  {print_dump(RAWData, 92);}
#endif

// Arrays for incoming data storage (for 3 channels)
THN128storage Sensor1, Sensor2, Sensor3;

//1024 prescaler
#define StartT0() {\
  TCNT0=0;\
  OCR0=Tout;\
  TCCR0=(1<<WGM01)|(1<<CS02)|(1<<CS00);\
}

#define StopT0(void) {\
  TCCR0=0;\
}


#pragma inline = forced
void RXDataSet(unsigned char BitNum, unsigned char BitVal)
{
  unsigned char ByteNumber;
  ByteNumber=(BitNum>>3);
  switch (BitVal)
  {
  case 0:
    cbr ((RXData[ByteNumber]), (BitNum&0x07));
    break;
  case 1:
    sbr ((RXData[ByteNumber]), (BitNum&0x07));
    break;
  default:
    break;
  }  
}

#ifdef TRACEMODE
void print_dump(unsigned char* p, unsigned int len)
{
  unsigned char i=0;
  while (i<len)
  {
     printf_P(_HexByteNum, *p);
     i++;
     p++;
     putchar (' ');
  }  
  printf_P(_Razer);
}
#endif          

// ****************************************************
// Process received packet
// ****************************************************
void RXprocess()
{
  signed int Temperature;  
  unsigned char Humidity, Channel;  
  unsigned int CRC;
  unsigned int Timestamp;
  bool BatteryDead, CRCOK;
  OS1Data* pOS1Data;
  pOS1Data=(OS1Data*)&RXData[0];
  // calculate CRC for received data 
  CRC = RXData[0]+RXData[1]+RXData[2];   
  // compare CRCs
  CRCOK=false;
  if   ((pOS1Data->CRC== (unsigned char)((CRC&0x00FF)+(CRC>>8)))||(pOS1Data->CRC==(unsigned char)(CRC&0x00FF)))
  {    
    CRCOK=true;
    Channel = pOS1Data->Channel;    
    Temperature=100*(pOS1Data->Th1)+10*(pOS1Data->Th2)+(pOS1Data->Th3);  
    if (Channel==0) Humidity= Temperature;
    else 
    {
      if ((pOS1Data->Sign)!=0)  Temperature=-Temperature;
    }
     
    if ((pOS1Data->LowBat)!=0)
    {
      BatteryDead=true;
    }
    else BatteryDead=false; 
    Timestamp=Counter10ms;
      switch (Channel)
      {
        case 0:                                        
          Sensor1.Status= 0x01; // Signal OK
          Sensor1.TimeStamp=Timestamp; // New TimeStamp      
          Sensor1.Humidity=Humidity; // it is for my own sensor only
          if (Sensor1.Humidity > Sensor1.HumidityMax) Sensor1.HumidityMax = Sensor1.Humidity;// it is for my own sensor only
          if (Sensor1.Humidity < Sensor1.HumidityMin) Sensor1.HumidityMin = Sensor1.Humidity;// it is for my own sensor only
          Sensor1.LowBat=BatteryDead;
          Sensor1.Status=1;
        break;
        case 1:       
          Sensor2.Status= 0x01; // Signal OK
          Sensor2.TimeStamp=Timestamp; // New TimeStamp
          Sensor2.Temperature10x=Temperature;
          if (Sensor2.Temperature10x > Sensor2.Temperature10xMax) Sensor2.Temperature10xMax = Sensor2.Temperature10x;
          if (Sensor2.Temperature10x < Sensor2.Temperature10xMin) Sensor2.Temperature10xMin = Sensor2.Temperature10x;
          Sensor2.LowBat=BatteryDead;
          Sensor2.Status=1;          
        break;
        case 2:
          Sensor3.Status= 0x01; // Signal OK
          Sensor3.TimeStamp=Timestamp; // New TimeStamp       
          Sensor3.Temperature10x=Temperature;
          if (Sensor3.Temperature10x > Sensor3.Temperature10xMax) Sensor3.Temperature10xMax = Sensor3.Temperature10x;
          if (Sensor3.Temperature10x < Sensor3.Temperature10xMin) Sensor3.Temperature10xMin = Sensor3.Temperature10x;
          Sensor3.LowBat=BatteryDead;          
          Sensor3.Status=1;          
        break;
      }       
  }      
  #ifdef TRACEMODE
      printf_P(_RFPacket);
      print_dump(RXData, 4);  
      printf_P (_CRC);
      if (!CRCOK) printf_P(_Bad);
      else printf_P(_OK);
      printf_P (_Channel, Channel);
      if (Channel==0) // Channel #1 that is used for humidity measurements in my own sensor
      {
        printf_P(_Humidity, Humidity); printf_P(_Percent);
      }       
      else {printf_P(_Temperature, Temperature); printf_P(_Celsium);}
      printf_P(_Battery); 
      if (BatteryDead) printf_P(_Bad);
      else printf_P(_OK);
      printf_P(_Timestamp,Timestamp);
      printf_P(_Razer);
   // remove comment when you need to see RAW packet timings
  // print_dump(RAWData, 92);  
  #endif
       cbr (MCUCR, ISC10); // Falling edge on INT1 generate an interrupt 
       sbr(GICR, INT1);    // Enable INT1 processing
       sbr (GIFR, INTF1);  // Clear INT1 IRQ
}

//***************************************/
//  OC0 Timer routine (Overflow checker for V1.0)
/***************************************/

#pragma vector=TIMER0_COMP_vect
__interrupt void OC0_IRQ( void )
{ 
      StopT0(); 
      INT1_cnt=0;
      Synchronised=false;
      cbr (MCUCR, ISC10); // falling edge will generate next INT1
      GIFR=(1<<INTF1);  // Clear INT1 IRQ
}

//***************************************
//  INT1 edge interrupt
//***************************************

#pragma optimize = no_cse
#pragma vector=INT1_vect
__interrupt void  INT1_IRQ( void )
{  
    static char TimerValue=0;  // Reset time between edges counter
    static bool wait_sb2=false; // Don't wait second start bit
    char ErrorCnt=0;    // Reset Error counter
    unsigned char c=0;
//  static char PreambleCounter;
    static char BitValue;
    static unsigned char SyncCnt; // RX bits counter
    static char PreambleCnt;
    static unsigned char TraceCnt;

//Spike Filter
    if (MCUCR & (1<<ISC10)) {for (c=0;c<=3;c++) if ((PIND&(1<<PD3))==0) goto INT1_exit;} //if waiting for rising edge at INT1   
    else {for (c=0;c<=3;c++) if ((PIND&(1<<PD3))!=0) goto INT1_exit;} //if waiting for falling edge at INT1

    #ifdef TRACEMODE
      if (TraceCnt>=255) TraceCnt=254;    
    #endif
//******************** Protocol 1 encoder begins ****************************    
    if (Synchronised==false) // if Preamble not detected
    {
      BitValue=1;  SyncCnt=0;   
      switch (INT1_cnt)
      {
        case 0: //*** falling edge #1 
           StartT0();           //Start T0 at first falling edge
           sbr (MCUCR, ISC10);  // rising edge generate nex INT1
           PreambleCnt=0;       // Reset Preamble counter    
           wait_sb2=0;
           INT1_cnt++;          // Increment INT1 counter
           #ifdef TRACEMODE
             TraceCnt=0;          // Reset Trace counter
           #endif  
        break;
        case 1:  //*** rising edge #1
            StopT0();
            TimerValue=TCNT0;     // Read timer value
            StartT0();            // Start Timer again
            cbr (MCUCR, ISC10); // falling edge will generate next INT1
            #ifdef TRACEMODE
              RAWData[TraceCnt++]=TimerValue; //Update Debug Array
            #endif          
            if InRange(TimerValue, TminL, 10) // T == Tmin |_| (Preambule is Detected)  
            { 
              PreambleCnt++;  INT1_cnt++;
            } 
            else // not preamble pulse is detected
            { 
               if ((PreambleCnt>=10)&&(wait_sb2==false)) // preamble pulses are over and wait for 1st Low start bit
                 {                                                   
                        if InRange(TimerValue, TstartBitL, deltaT) // T == Tstartbit Low |__| (Startbit is Detected)
                          {                
                            PreambleCnt++; INT1_cnt++; 
                            cbr (MCUCR, ISC10); // falling edge generate next INT1
                          }
                        else goto   INT1_exit_w_error; // Not a startbit
                 } 
               else if ((PreambleCnt>=10)&&(wait_sb2==true)) // preamble pulses are over and wait for 2nd Low start bit
                     {            
                        if InRange(TimerValue, TstartBitL2long, deltaT)// T == TstartbitLow2Long |____| ( Long Startbit2 is Detected)
                          {                       
                              wait_sb2=false;
                              BitValue=0; SyncCnt++;
                              RXDataSet(SyncCnt-1,BitValue);
                              Synchronised=true; INT1_cnt=1; // continue from rising edge after syncrhonised   
                          }
                        else if InRange(TimerValue, TstartBitL2short, deltaT)// T == TstartbitLow2short |__| ( Short Startbit2 is Detected)
                          {                                              
                              wait_sb2=false;
                              BitValue=1;
                              Synchronised=true; INT1_cnt=1; // continue from rising edge after syncrhonised
                           }              
                       else goto   INT1_exit_w_error; // Not a startbit
                     }                             
              else goto INT1_exit_w_error; // Not a preamble and not a  Low start bit
            } // * not preamble pulse is detected
    break; // end of case 1:
    case 2: //*** falling edge #2 (preaumbule previosly detected)
        StopT0();
        TimerValue=TCNT0;
        StartT0(); // Start T0
        #ifdef TRACEMODE
          RAWData[TraceCnt++]=TimerValue;        
        #endif  
        if InRange(TimerValue,TminH, 10) // T == Tmin |^| (Preambule is Detected)  
        { 
          //if (PreambleCnt>=4) TurnOFF_v2(); // OK it seems to be v1.0 Packet - turn off v2.1 recognition                
          PreambleCnt++;
          sbr (MCUCR, ISC10); // rising edge will generate next INT1
          INT1_cnt=1;
        }
        else
        {   
          if ((PreambleCnt>=10)&&InRange(TimerValue, TstartBitH, deltaT)) // T == Tstartbit High |^^^| (Startbit is Detected)
           { 
                PreambleCnt++;
                wait_sb2=true; // we need 2nd low start bit before start data pulses reception
                INT1_cnt=1; // will wait for second low start bit
                sbr (MCUCR, ISC10); // rising edge generate next INT1                
           }
          else goto   INT1_exit_w_error;// error in stream
          } 
       } // end of INT1_switch
    } // end of (syncrohised = false)
    else // If pattern is synchronised *******************************
    {   
     sbr(PORTD, PD5); // LED ON
     switch (INT1_cnt)       
      {  
         case 0://*** rising edge #1 (Data reception begins)
            StopT0();
            TimerValue=TCNT0;
            StartT0(); // Start T0
            #ifdef TRACEMODE
              RAWData[TraceCnt++]=TimerValue; // store Tstart bit Low #2
            #endif  
            // ^_^
            if ((InRange(TimerValue,TminL,10)&&(BitValue==0))||(InRange(TimerValue,TmaxL,10)))
              {
                  BitValue=0;SyncCnt++;     
              }
            else if (!(InRange(TimerValue,TminL,10))) goto INT1_exit_w_error;    
            if (ErrorCnt==0) 
             { // if Timings are correct
              cbr (MCUCR, ISC10); // falling edge generate next INT1
              INT1_cnt=1;
             }
        break;
        case 1: //*** falling edge #1 (Data reception begins)
          StopT0(); TimerValue=TCNT0; StartT0(); // Start T0
          #ifdef TRACEMODE
            RAWData[TraceCnt++]=TimerValue;
          #endif  
          //^|_
          if ((InRange(TimerValue,TminH,10)&&(BitValue==1))||(InRange(TimerValue,TmaxH,10)))
          { 
           BitValue=1; SyncCnt++;
          }
          else if (!(InRange(TimerValue,TminH,10))) goto INT1_exit_w_error;
          
          INT1_cnt=0;   
          sbr (MCUCR, ISC10); // rising edge generate next INT1  
        break;         
      } //* end of switch T0 counter  
  if (ErrorCnt==0) // If no error in timing
    {      
      RXDataSet(SyncCnt-1,BitValue);
    }
     if (SyncCnt>=32)
    { 
      StopT0();           // Stop T0    
      cbr (PORTD, PD5);   // Turn OFF RX LED
      RXprocess();
      INT1_cnt=0;
      Synchronised=false;
      RFData_received=true;   // Process Received Bit stream        
    }
  }// ** If pattern is synchronised 
  
INT1_exit:
      sbr (GIFR, INTF1); //Reset INT1 request flag
      return;
      
INT1_exit_w_error:
       //T=??? (Error in a Manchester stream)
       cbr (PORTD, PD5); // LED OFF
       Synchronised=false;
       StopT0(); 
       INT1_cnt=0;
       cbr (MCUCR, ISC10); // falling edge generate next INT1
       //TurnON_v2();
       return;
//******************** Protocol 1 encoder ends ****************************       

}

/***********************************************************************************************
// This routine is used for detecting old signals that was not updating more than 120 seconds
***********************************************************************************************/
void CheckSensorAlive()
{
   if (Sensor1.TimeStamp > Counter10ms) 
           { // There was Counter10ms overflow 
             Sensor1.UpdatePeriod=(0xFFFF-Sensor1.TimeStamp+1)+Counter10ms;                        
           }
   else Sensor1.UpdatePeriod = (Counter10ms-Sensor1.TimeStamp);                    
   if (Sensor1.UpdatePeriod > 12000) Sensor1.Status= 0x02; // Signal loss      

   if (Sensor2.TimeStamp > Counter10ms) 
   { // There was Counter10ms overflow 
       Sensor2.UpdatePeriod=(0xFFFF-Sensor2.TimeStamp+1)+Counter10ms;                        
   }
   else Sensor2.UpdatePeriod = (Counter10ms-Sensor2.TimeStamp);                 
   if (Sensor2.UpdatePeriod > 12000) Sensor2.Status= 0x02; // Signal loss
   
   
   if (Sensor3.TimeStamp > Counter10ms) 
     { // There was Counter10ms overflow 
        Sensor3.UpdatePeriod=(0xFFFF-Sensor3.TimeStamp+1)+Counter10ms;                        
     }
    else Sensor3.UpdatePeriod = (Counter10ms-Sensor3.TimeStamp);                 
    if (Sensor3.UpdatePeriod > 12000) Sensor3.Status= 0x02; // Signal loss                      
}

