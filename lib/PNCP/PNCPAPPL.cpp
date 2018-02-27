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
#define ShowTime 0x08
#define Report 0x09
#define ReportCueConinuity 0x0A
#define ReportCueResistance 0x0B
//full8
#define Chargecues 0xC0
#define SetPulseWidth 0xC1
#define GetVoltage 0xC2
#define SetCueSchedule 0xC3
/*!
  \brief Single cue handle
  \details This is used to set the callback for single cue commands

   \param Function to be triggered when command received
   \sa setHandleReport()
   \sa setHandleChargecues()
   \sa setHandleSetPulse()
  */
void PNCPAPPL::setHandleSinglecue(void (*fptr)(uint8_t cue))    { mSinglecueCallback              = fptr; }
/*!
  \brief Report handle
  \details This is used to set the callback for reporting commands
  \code{.cpp}
  void report()
  {
    long capabilities = EEPROMReadlong(1);
    DLL.write((uint8_t*)&capabilities, 4);

  }
  //set callback to report function.
  APPL.setHandleReport(report);
  \endcode

   \param Function to be triggered when command received
   \sa setHandleSinglecue()
   \sa setHandleChargecues()
   \sa setHandleSetPulse()
  */
void PNCPAPPL::setHandleReport(void (*fptr)(void))              { mReportCallback                 = fptr; }

/*!
  \brief charge cue handle
  \details This is used to set the callback for charge cue commands in a capacitive discharge system

   \param Function to be triggered when command received
   \sa setHandleReport()
   \sa setHandleSinglecue()
   \sa setHandleSetPulse()
  */
void PNCPAPPL::setHandleChargecues(void (*fptr)(void))          { mChargecuesCallback             = fptr; }
/*!
  \brief pulse width handle
  \details This is used to set the callback for pulsewidth command. which sets how long the cue will be held high

   \param Function to be triggerd when  command received
   \sa setHandleReport()
   \sa setHandleChargecues()
   \sa setHandleSinglecue()
  */
void PNCPAPPL::setHandleSetPulse(void (*fptr)(uint8_t pulse))   { mSetPulseCallback               = fptr; }


/*!
  \brief intialize the library
  \details This is used to intialize the application layer of the library.
  \code{.cpp}
   PNCP DLL(firstGADD, firstUADD);
   PNCPAPPL APPL(DLL);
   \endcode
  
   \param DLL PNCP data link layer passthru
   \sa PNCP()
  */
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
    //uint8_t cmdsize = DLL.frame.PLD[0]>>5;
    uint8_t cmdsize = DLL.frame.PLD[0];
    Serial.print("got a packet");
    //uint8_t high2 = mac.frame.APPF[0]>>6;
    //uint8_t high4 = mac.frame.APPF[0] >>4;
    //uint8_t full8 = mac.frame.APPF[0];
    uint8_t temp = DLL._Address_type;
    if(temp == SOFG || temp == SOFU)
    {

    if(!bitRead(cmdsize,7))
    {
      Serial.print("4bit");
      //Serial.println(cmdsize,DEC);

      //Fire cue command
      if(!bitRead(cmdsize,6))
      {
        uint8_t cue = DLL.frame.PLD[0] <<2;
        cue = cue >>2;
        if (mSinglecueCallback != 0)              mSinglecueCallback(cue/*cue number*/);


      }else{
        //Multicue command

      }
      //2 bit command
    }
    else if(bitRead(cmdsize,7) && !bitRead(cmdsize,6))
    {
      //4 bit command
      uint8_t temp = DLL.frame.PLD[0]>>4;
      switch(temp)
      {
        case Report:
          //sends device capabilities to master
          if (mReportCallback != 0)              mReportCallback();
          break;

          case ShowTime:
          break;

          case ReportCueConinuity:
          break;

          case ReportCueResistance:
          break;

          default:
          //not implamented
          break;

      }



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

            case SetPulseWidth:
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

            default:
              //not implamented
            break;
          }

      //8 bit command
    }
  }

  }

}
