#include "MC33996.h"
#include <SPI.h>


// Defines to keep logical information symbolic go here

#ifndef HIGH
#define    HIGH          (1)
#endif

#ifndef LOW
#define    LOW           (0)
#endif

#ifndef ON
#define    ON            (1)
#endif

#ifndef OFF
#define    OFF           (0)
#endif


//#define CLOCK_DIVIDER    (4)

#define CMD_ON_OFF         (0b00000000) //ON , OFF Command
#define OP_OPENLOAD_EN     (0b00000100) //Enable open load detection
#define CMD_RESET          (0b00011000) //Rest outputs to off
#define OP_GLOBAL_SHUTDOWN (0b00001000) // Sets global shut and retry bits
#define THERMAL_BIT        (0b00000010) // thermal retry bit to be used with OP_GLOBAL_SHUTDOWN
#define OVER_VOLTAGE_BIT   (0b00000001) // over voltage retry bit to be used with OP_GLOBAL_SHUTDOWN
#define CMD_NULL           (0b0000000000000000) //Null command for last 16bits
#define CMD_ALL_ON         (0b1111111111111111)



MC33996::MC33996(uint8_t CSpin)
{
  _modeCache        = CMD_ON_OFF; //default cmd
  _outputCache      = 0x0000;
  _CSpin            = CSpin;
  _global_shutdown  = OP_GLOBAL_SHUTDOWN;
  mFaultCallback    = 0;
};

void MC33996::begin(){
  pinMode(_CSpin,OUTPUT);
  digitalWrite(_CSpin, HIGH);

  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
  write(CMD_RESET, CMD_NULL);

}

void MC33996::write(uint8_t reg, uint16_t value)
{
  //SPI.beginTransaction();
  digitalWrite(_CSpin, LOW);
  _modeCache   = SPI.transfer(reg);
  _outputCache = SPI.transfer16(value);
  digitalWrite(_CSpin, HIGH);

  _modeCache = 0;
  _outputCache = 0;


  //SPI.beginTransaction();
}

void MC33996::enableContinutyDetection()
{
  write(OP_OPENLOAD_EN,CMD_ALL_ON);
}

void MC33996::setOvervoltage(bool value)
{
  bool bit = (_global_shutdown >> 7) & 1U;

  if(bit != value)
  {
    _global_shutdown ^= 1UL << 7;
  }
  write(_global_shutdown, CMD_NULL);
}

void MC33996::setThermal(bool value)
{
  bool bit = (_global_shutdown >> 6) & 1U;

  if(bit != value)
  {
    _global_shutdown ^= 1UL << 6;
  }
  write(_global_shutdown,CMD_NULL);

}

void MC33996::continutyDetection()
{

  digitalWrite(_CSpin, LOW);
  _modeCache   = SPI.transfer(CMD_ON_OFF);
  _outputCache = SPI.transfer16(CMD_NULL);//turn off all cues
  digitalWrite(_CSpin, HIGH);
  if (_modeCache != 0x00){if (mFaultCallback !=0) mFaultCallback(_modeCache,_outputCache);}
  _modeCache   = 0;
  _outputCache = 0;
  //value = _loadDection+header;


  //ArrayToInteger convert = {0x00,_loadDection[2],_loadDection[1],_loadDection[0]};

}


void MC33996::digitalWrite(uint8_t pin, uint8_t value)
{
  if (pin < 1 | pin > 16) return;
  if (pin < 1 | pin > 16) return;
  if (value) {
    _outputCache |= 1 << (pin - 1);
  } else {
    _outputCache &= ~(1 << (pin - 1));
  }
  write(CMD_ON_OFF, _outputCache);

}
void MC33996::digitalWrite(unsigned int value)
{
  write(CMD_ON_OFF, value);
  _outputCache = value;
}

void MC33996::setFaultReport(void (*fptr)(uint8_t,uint16_t))              { mFaultCallback                 = fptr; }
