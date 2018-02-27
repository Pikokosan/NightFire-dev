#include <Arduino.h>
#include <PNCP.h>
#include <PNCPAPPL.h>
#include <EEPROM.h>
#include <Battery.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MCP23008.h>

  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  #include <avr/wdt.h>

#endif



//PNCP DLL;

#define SOFB 0x55 // Boadcast
#define SOFG 0x47 // Group address
#define SOFU 0x78 // Unique address
//Uncomment to set up the devices eeprom. then comment and reflash
// Pikoko Unique address 0x14CAC701-FF
//#define firstrun
#define firstGADD         0x01
#define firstUADD         0x14CAC702
#define firstCapabilities 0xA0050200

String Version = "0.2 PNCP";


//high2
#define FIRE 0x00
#define FIREMULTI 0x01
//high4
#define TIME 0x08
#define REPORT 0x09
#define REPORTCONT 0x0A
#define REPORTRES 0x0B
//full8
#define CHARGECUES 0xC0
#define SETPULSE 0xC1
#define GETVOLTAGE 0xC2
#define SETCUESCHEDULE 0xC3

#if defined(MCU_STM32F103C8)
  #define testled PC13
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  #define testled 13
#endif



uint8_t pulsewidth = EEPROM.read(10);

//EEprom layout
/*
*----------------------------------------------
*address |     1-4      |  5   | 6-9  |  10   |
*data    | capabilities | GADD | UADD | pulse |
*
*
*/

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
Adafruit_MCP23008 mcp;


/*
 * 2 cells (2S1P) li-ion/li-poly battery wired to A0, continuous sensing
 *
 *   +--------+
 *   |        |
 *   |       +--+
 *   |       |R1| 22k
 * -----     |  |
 *           +--+
 *  ---       |---------+ A0
 *   |       +--+
 *   |       |R2|
 *   |       |  | 10k
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

//Custom lcd Char
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

  callbacksetup();
  #if defined(__AVR_ATmega32U4__)
    DLL.begin(115200,2);
  #endif
  #if defined(__AVR_ATmega328P__)
    DLL.begin(115200, 1);
  #endif
  battery.begin(5000, 3.2);
  //pinMode(13, OUTPUT);
  //pinMode(testled,OUTPUT);

  //setup i2c output pins
  mcp.begin();      // use default address 0
  for(int i=0 ; i<7; i++){
    mcp.pinMode(i, OUTPUT);
  }


  //EEPROM.update(6,10);
  //digitalWrite(testled,HIGH);
  #ifdef firstrun
  EEPROM.update(5, firstGADD);
  EEPROMWritelong(6, firstUADD);
  EEPROMWritelong(1, firstCapabilities);
  #endif
  delay(500);
  digitalWrite(testled, LOW);
  LCDSetup();
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  wdt_enable(WDTO_2S);

#endif

//Wire.begin();
//Wire.beginTransmission(0x20);
//Wire.write(0x00);
//Wire.write(0x00);
//Wire.endTransmission();



}

void loop()
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

  APPL.update();
  EEPROM.update(5,DLL.getGADD());

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
}


void report()
{
  long capabilities = EEPROMReadlong(1);
  DLL.write((uint8_t*)&capabilities, 4);

}

void SetPulse(uint8_t pulse)
{
  pulsewidth = pulse;
  //Serial.print("pulse width = ");
  //Serial.println(pulsewidth,DEC);
  EEPROM.update(6,pulsewidth);
}


void fire(uint8_t cue)
{
  //Serial.println("Cue!");
  /*
  switch(cue)
  {

    case 1:
      Serial.println("cue 1");
      mcp.digitalWrite(0,HIGH);
      delay(pulsewidth);
      mcp.digitalWrite(0, LOW);
      //mcp.writeGPIO(1);
    break;

    case 2:
    Serial.println("cue 2inst");
    digitalWrite(testled, HIGH);
    delay(pulsewidth);
    digitalWrite(testled,LOW);
    break;

  }
  */
  //uint8_t address = 0x20;
  //uint8_t truecue = 0x01;
  cue = cue - 1;
  /*

  if( cue > 8 )
  {
    address++;
    cue = cue -8;

  }
  if( cue > 16 )
  {
    address++;
    cue = cue - 8;
  }

  if( cue > 24 ){
    address++;
    cue = cue - 8;
  }

  truecue = truecue << cue;
  */

  mcp.digitalWrite(cue, HIGH);
  delay(pulsewidth);
  mcp.digitalWrite(cue, LOW);

  //mcp.writeGPIO(truecue);
  //Wire.beginTransmission(address);
  //Wire.write(0x09);
  //Wire.write(truecue);
  //Wire.endTransmission();
  //delay(pulsewidth);
  //Wire.beginTransmission(address);
  //Wire.write(0x09);
  //Wire.write(0x00);
  //Wire.endTransmission();


}
