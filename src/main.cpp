#include <Arduino.h>
#include <PNCP.h>
#include <PNCPAPPL.h>
#include <EEPROM.h>
#include <Battery.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Config.h"



#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  #include <avr/wdt.h>
#endif

#if defined(USE_MCP23008)
  #include <Adafruit_MCP23008.h>
  Adafruit_MCP23008 outputCtrl;
#endif
//MC33996 instance
#if defined(USE_MC33996)
  #include <MC33996.h>
  MC33996 outputCtrl(10,9);//Define instance and CS pin
#endif

//PNCP DLL;



const String Version = "0.3 PNCP";

uint8_t groupAdd;
uint32_t uniqueAdd;



uint8_t pulsewidth = EEPROM.read(3) >>4;

const int testled  = 13;

//EEprom layout
/*
*----------------------------------------------
*address |     1-4      |  5   | 6-9  |  10   |
*data    | capabilities | GADD | UADD | pulse |
*
*
*/

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
//DEBUG_PRINT("LCD Initalized");


/*
 * 25v max li-ion/li-poly battery wired to A0, continuous sensing
 *
 *   +--------+
 *   |        |
 *   |       +--+
 *   |       |R1| 30k
 * -----     |  |
 *           +--+
 *  ---       |---------+ A0
 *   |       +--+
 *   |       |R2|
 *   |       |  | 7.5k
 *   |       +--+
 *   |        |
 *   +--------+---------+ GND
 *
 **/
#if defined(MCU_STM32F103C8)
  Battery battery(11200, 12600, PA0);
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  Battery battery(11200, 12600, A1);
#endif

//Battery battery()

//Custom lcd Char for battery
byte batfull[8] ={
	0b01110,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
};
byte bathalf[8] ={
	0b01110,
	0b11111,
	0b10001,
	0b10011,
	0b10111,
	0b11111,
	0b11111,
	0b11111
};
byte batlow[8] ={
	0b01110,
	0b11111,
	0b10001,
	0b10001,
	0b10001,
	0b10011,
	0b10111,
	0b11111
};

long EEPROMReadlong(long address);
void LCDSetup();
void fire(uint8_t cue);
void callbacksetup();
void report();
void SetPulse(uint8_t pulse);
void batteryCheck();
#if defined(USE_MC33996) && defined(USE_MC33996_CONTINUITY_CHECK)
  void callbackOutputfault(uint8_t fault, uint16_t registry);
  void callbackCuecontinuity();
#endif







#ifdef firstrun
  //DLL (Group address, Unique address)
  PNCP DLL(firstGADD, firstUADD);
  void EEPROMWritelong(int address, long value);
#else
  //DLL (Group address, Unique address)
  PNCP DLL(EEPROM.read(5),EEPROMReadlong(6));
#endif

PNCPAPPL APPL(DLL);





void setup()
{
  groupAdd = DLL.getGADD();
  uniqueAdd = DLL.getUADD();
  //pinMode(testled, OUTPUT);

  #if defined(__AVR_ATmega32U4__)
    DLL.begin(115200,2);
  #endif
  #if defined(__AVR_ATmega328P__)
    DLL.begin(115200, 1);
  #endif
  battery.begin(5000, 3.2);
  pinMode(13, OUTPUT);
  //pinMode(testled,OUTPUT);

  //setup i2c output pins
  outputCtrl.begin();
  #if defined(USE_MCP23008)
    for(int i=0 ; i<7; i++){
      outputCtrl.pinMode(i, OUTPUT);
    }
  #endif

  #if defined(USE_MC33996)
    outputCtrl.enableContinutyDetection();
  #endif

  callbacksetup();


  //EEPROM.update(6,10);
  //digitalWrite(testled,HIGH);
  #ifdef firstrun
    EEPROM.update(5, firstGADD);
    EEPROMWritelong(6, firstUADD);
    EEPROMWritelong(1, firstCapabilities);
  #endif
  delay(500);

  LCDSetup();

  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    wdt_enable(WDTO_2S);// the watchdog timer is used to detect if the system hangs and resets it
  #endif

}

void loop()
{


  batteryCheck();
  APPL.update();
  uint8_t temp = DLL.getGADD();
  if(temp != groupAdd)
  {
    EEPROM.update(5,temp);
    groupAdd = temp;
    lcd.setCursor(6,0);
    lcd.print(DLL.getGADD());
  }

}


//This is only loaded on first run setups.
#ifdef firstrun
void EEPROMWritelong(int address, long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}
#endif

long EEPROMReadlong(long address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void LCDSetup()
{
  lcd.init();
  lcd.backlight();
  lcd.setCursor(4,0);
  lcd.print("Starting");
  //init custom lcd charters
  lcd.createChar(1,batfull);
  lcd.createChar(2,bathalf);
  lcd.createChar(3,batlow);
  lcd.setCursor(4,1);
  lcd.print("V");
  lcd.print(Version);
  delay(3000);
  lcd.clear();
  lcd.setCursor(15,0);
  lcd.write(1);
  lcd.setCursor(0,0);
  lcd.print("GADD:");
  lcd.setCursor(6,0);
  lcd.print(DLL.getGADD());
  lcd.setCursor(6,1);
  //rest
  //lcd.rightToLeft();
  lcd.print(DLL.getUADD(),HEX);
  lcd.leftToRight();
  lcd.setCursor(0,1);
  lcd.print("UADD: ");

}

void callbacksetup()
{
  APPL.setHandleSinglecue(fire);
  APPL.setHandleReport(report);
  APPL.setHandleSetPulse(SetPulse);
  #if defined(USE_MC33996) && defined(USE_MC33996_CONTINUITY_CHECK)
    outputCtrl.setFaultReport(callbackOutputfault);
    APPL.setHandleCueContinuity(callbackCuecontinuity);
  #endif
}

#if defined(USE_MC33996) && defined(USE_MC33996_CONTINUITY_CHECK)
  void callbackCuecontinuity(){outputCtrl.continutyDetection();}

  void callbackOutputfault(uint8_t fault, uint16_t registry)
  {
    //Debug
    #if defined(MAIN_DEBUG)
      Serial.print("Error= ");
      Serial.print(fault, BIN);
      Serial.print(" OUTPUTS= ");
      Serial.print(registry,BIN);
    #endif
    registry = ~registry;//invert outputs

    #if defined(MAIN_DEBUG)
      Serial.print(" inverted OUTPUTS= ");
      Serial.println(registry,BIN);
    #endif
    //uint32_t temp = registry;
    //temp = temp << 8;
    union
    {
      uint32_t bytes32;
      uint8_t bytes[4];

    }testing;

    //test_val testing;
    testing.bytes32 = registry >> 4;


    //testing.bytes[0] = (temp >> 0)  & 0xFF;
    //testing.bytes[1] = (temp >> 8)  & 0xFF;
    //testing.bytes[2] = (temp >> 16) & 0xFF;
    #if defined(MAIN_DEBUG)
      Serial.println("sent bytes:");
      Serial.println(testing.bytes[2],BIN);
      Serial.println(testing.bytes[1],BIN);
      Serial.println(testing.bytes[0],BIN);
    #endif

    DLL.write(testing.bytes, 3);


    //do something
  }
#endif
void report()
{
  long capabilities = EEPROMReadlong(1);
  #if defined(MAIN_DEBUG)
    Serial.print("capabilities = ");
    Serial.println(capabilities,HEX);
  #endif
   DLL.write((uint8_t*)&capabilities, 4);

}

void SetPulse(uint8_t pulse)
{
  pulsewidth = pulse;
  //Serial.print("pulse width = ");
  //Serial.println(pulsewidth,DEC);
  pulse = pulse <<4;
  //byte temp =


  //temp = EEPROM.read(3)
  EEPROM.update(3,pulse);
}


void fire(uint8_t cue)
{
  #if defined(MAIN_DEBUG)
    Serial.print("Firing cue: ");
    Serial.println(cue,DEC);
  #endif

  outputCtrl.digitalWrite(cue, HIGH);
  //digitalWrite(13, HIGH);
  delay(pulsewidth*10);
  //digitalWrite(13, LOW);
  outputCtrl.digitalWrite(cue, LOW);

}

void batteryCheck()
{
  if(battery.level() < 25)
  {
    lcd.setCursor(15,0);
    lcd.write(3);
    lcd.setCursor(0,0);

  }
  else if(battery.level() < 50)
  {
    lcd.setCursor(15,0);
    lcd.write(2);
    lcd.setCursor(0,0);

  }
  else
  {
    lcd.setCursor(15,0);
    lcd.write(1);
    lcd.setCursor(0,0);

  }
}
