#include "PNCPAPPL.h"
#include <PNCP.h>
#include <Arduino.h>
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
#include <avr/wdt.h>

#endif


//PNCPAPPL::PNCPAPPL();
uint8_t type = 1;//test type
#define SOFB 0x55 // Boadcast
#define SOFG 0x47 // Group address
#define SOFU 0x78 // Unique address

//high2
#define Singlecue 0x00
#define Multicue 0x01
//high4
#define Time 0x08
#define Report 0x09
#define ReportConinuity 0x0A
#define ReportResistance 0x0B
//full8
#define Chargecues 0xC0
#define SetPulse 0xC1
#define GetVoltage 0xC2
#define SetCueSchedule 0xC3

void PNCPAPPL::setHandleSinglecue(void (*fptr)(uint8_t cue))    { mSinglecueCallback              = fptr; }
void PNCPAPPL::setHandleReport(void (*fptr)(void))              { mReportCallback                 = fptr; }
void PNCPAPPL::setHandleChargecues(void (*fptr)(void))          { mChargecuesCallback             = fptr; }
void PNCPAPPL::setHandleSetPulse(void (*fptr)(uint8_t pulse))   { mSetPulseCallback               = fptr; }



PNCPAPPL::PNCPAPPL(PNCP& _DLL):DLL(_DLL)
{
  mSinglecueCallback    = 0;
  mReportCallback       = 0;
  mChargecuesCallback   = 0;
  mSetPulseCallback     = 0;

}

void PNCPAPPL::update()
{
  DLL.update();
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  wdt_reset();
  #endif
  if (DLL.available()){
    uint8_t cmdsize = DLL.frame.PLD[0]>>5;
    //uint8_t high2 = mac.frame.APPF[0]>>6;
    //uint8_t high4 = mac.frame.APPF[0] >>4;
    //uint8_t full8 = mac.frame.APPF[0];
    uint8_t temp = DLL._Address_type;
    if(temp == SOFG || temp == SOFU)
    {

    if(!bitRead(cmdsize,2))
    {
      //Serial.print("4bit");
      //Serial.println(cmdsize,DEC);

      //Fire cue command
      if(cmdsize>>1 == Singlecue)
      {
        uint8_t cue = DLL.frame.PLD[0] <<2;
        cue = cue >>2;
        if (mSinglecueCallback != 0)              mSinglecueCallback(cue/*cue number*/);


      }else{
        //Multicue command

      }
      //2 bit command
    }
    else if(bitRead(cmdsize,2) && !bitRead(cmdsize,1))
    {
      uint8_t temp = DLL.frame.PLD[0]>>4;
      if(temp == Report)
      {
        //sends device capabilities to master
        if (mReportCallback != 0)              mReportCallback();
        //long capabilities = EEPROMReadlong(1);
        //DLL.write((uint8_t*)&capabilities, 4);

      }
      //4 bit command

    }
    else
    {
      //Serial.print("8bit");
      //Serial.println(mac.frame.APPF[0],DEC);
      switch(DLL.frame.PLD[0])
          {

            case Chargecues:
            if (mChargecuesCallback != 0)            mChargecuesCallback();
            break;

            case SetPulse:
            if (mSetPulseCallback != 0)            mSetPulseCallback(DLL.frame.PLD[1]);
            //pulsewidth = DLL.frame.PLD[1];
            //Serial.print("pulse width = ");
            //Serial.println(pulsewidth,DEC);
            //EEPROM.update(6,pulsewidth);
            break;

            case GetVoltage:
            break;

            case SetCueSchedule:
            for(int i; i > DLL.frame.header.PLI; i++ )
            {

            }

            break;

          }

      //8 bit command
    }
  }

  }

}
