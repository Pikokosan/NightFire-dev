#ifndef _MC33996_H_
#define _MC33996_H_

#include <Arduino.h>
//#include <SPI.h>

class MC33996{
  public:

    MC33996(uint8_t,uint8_t);
    void begin();
  //void digitalWrite(uint8_t p, uint8_t d);

    void setOvervoltage(bool value = true);
    void setThermal(bool value = true);
    void enableContinutyDetection();
    void continutyDetection();
    void digitalWrite(uint8_t,uint8_t);
    void digitalWrite(unsigned int);
    void setFaultReport(void (*fptr) (uint8_t,uint16_t));

  private:
    void write(uint8_t, uint16_t);
    uint8_t   responceheader;
    uint8_t   _modeCache;
    uint16_t  _outputCache;
    uint8_t   _CSpin;
    uint8_t   _Resetpin;
    uint8_t   _global_shutdown;
    void (*mFaultCallback)(uint8_t fault, uint16_t registry);

    //SPISettings settingsMC33996(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE1);
    //uint8_t   _loaddection[3];
    union ArrayToInteger {

      byte array[4];
      uint32_t integer;
    };

};
//us "OR" logic to get both overcurrent and thermal retry
//#define enable_overcurrent_thermal_retry 0x0F;


#endif
