/*
* DEV
* NightFire slat 16 cue
*
* Version 0.0.1
*
* Info:
*  This is the night fire 16 cue dev unit
*
* Libraries:
*  RF24 by TMrh20 (in library manager)
*  RF24Network by TMrh20 (in library manager)
*
*  Pinout:[
*
*    ]
*
Packet example:
170,2,1,0,0,0
170 = start of frame
2 = module 2
1 = ping command
0 = no Datalength
0 = no data
0 = crc

*

ToDo:
firing voltage indication.
add onboard arm and safe switch.
Hardware:

add good 3.3v regulator.
swith to arduino micro pro or 32u4?
swith to rfm69 433mhz(encryption,better packet handling, range)
add i/o for arm/safe pin
i2c io expander for menu buttons?
add 4mbit flash(rfm69 can flash firmware);
see moteinomega for ideas


Done:
Add display backlight timout
proper send_response commands

+ = done

*/


#include <Arduino.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <MenuSystem.h>
#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

//delay to turn off the lcd display
const long interval = 5000;
unsigned long previousMillis = 0;
boolean displayOn = true;
boolean displayTimer = false;





LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display
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


boolean response = false;
boolean command_recv = false;
boolean SOF = true;
boolean fire_armed_remote = false;
boolean test_armed_remote = false;
//byte rx_count;

//Menu variables




//Module 255 is always the master
int MODULE_ID  = EEPROM.read(0);

//serial debug enable
//#define DEBUG

//System setup
#define CHANNEL_COUNT 16
#define FIRE_DELAY 100
String Version = "0.3";

uint8_t buf[30];
uint8_t rx_count = 0;
int packet_error = 0;

//Pin defintions
#define latchPin 7
#define clockPin 6
#define dataPin 5


#define testPin 4
#define testAdc A0
#define firePin 3
#define statusLed 9
#define rx_en 2
//#define test_enable 3
//#define fire_enable 4

//buttons



//Protocol define
#define CMD_UNKNOWN 0
#define CMD_PING 1
#define CMD_FIRE 2
#define CMD_TEST 3
#define CMD_TESTALL 4
#define CMD_FIRE_ENABLE 5
#define CMD_FIRE_DISABLE 6
#define CMD_TEST_ENABLE 7
#define CMD_TEST_DISABLE 8
#define CMD_GET_FIRE_SW 9
#define CMD_GET_TEST_SW 10
#define CMD_GET_CHANNEL_COUNT 11
#define CMD_SET_ID 12
#define RESPONSE_BAD_COMMAND 101
#define RESPONSE_BUSY 102
#define RESPONSE_PONG 103
#define RESPONSE_TEST_RESULT 104
#define RESPONSE_TEST_SW_STATE 105
#define RESPONSE_FIRE_SW_STATE 106
#define RESPONSE_FIRING_DISABLED 107
#define RESPONSE_TESTING_DISABLED 108
#define RESPONSE_CHANNEL_OUT_OF_RANGE 109
#define RESPONSE_CHANNEL_COUNT 110
#define RESPONSE_MODULE_ACTIVE 111
#define RESPONSE_ID_SET 112
/*
//Timecode design
//255,           00:00:00
//broadcast,     mm:ss:ms
//pc->module no responce

#define CMD_SET_TIMECODE 13 //fstc
#define CMD_START_TIMECODE 14 //tbeg
#define CMD_STOP_TIMECODE 15 //tdis
*/


void send_response(byte response_type);
void fire_channel();
void setlcdModule();

//Radio setup
RF24 radio(8,10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

//Packet layout for wireless

struct recv_packet_type_wireless{
  uint8_t Command;
  uint8_t Module;
  uint8_t Datalength;
  uint8_t Data;
};

//Packet layout for serial.
struct recv_packet_type_serial {
  uint8_t Module;
  uint8_t Command;
  uint8_t Datalength;
  uint8_t Data[10];
  uint8_t CRC;
};

//New packet type?
//Header
/*
typedef struct{
uint8_t Module;
uint8_t Command;
}__attribute__((__packed__))packet_header_t;

typedef struct{
packet_header_t Header;
uint8_t Datalength;
uint8_t Data[];
uint8_t CRC;
}__attribute__((__packed__)) data_packet_t;
*/



recv_packet_type_serial recv_packet;






void setup() {
  // put your setup code here, to run once:
  //enable serial

  lcd.init();
  lcd.backlight();
  lcd.setCursor(4,0);
  lcd.print("Starting");
  EEPROM.write(0,1);
  Serial.begin(9600);
  //enable pins
  pinMode(statusLed,  OUTPUT);
  pinMode(firePin, OUTPUT);
  pinMode(testPin, OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  pinMode(rx_en, OUTPUT);

  //init custom lcd charters
  lcd.createChar(1,batfull);
  lcd.createChar(2,bathalf);
  lcd.createChar(3,batlow);


  //LCD timer event
  previousMillis = millis();



  lcd.setCursor(6,1);
  lcd.print("V");
  lcd.print(Version);
  delay(3000);


  //displayMenu();
  lcd.clear();
  lcd.setCursor(15,0);
  lcd.write(1);
  lcd.setCursor(0,0);
  lcd.print("Status:");
  lcd.setCursor(9,0);
  lcd.print("SAFE ");
  lcd.setCursor(15,1);
  //rest
  lcd.rightToLeft();
  lcd.print(MODULE_ID);
  lcd.leftToRight();
  //lcd.noBacklight();
  //mesh.begin();

}


byte CRC8(volatile byte *data, byte len)
{
  //CRC8-maxim
  byte crc = 0x00;
  while (len--)
  {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--)
    {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        //crc ^=0xE0;
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}


boolean packet_decode()
{
  int test = 0;


  test = CRC8((byte*)&recv_packet, recv_packet.Datalength+3);
  #ifdef DEBUG
  Serial.print("crc cal = ");
  Serial.println(test,HEX);
  #endif
  if(test == recv_packet.CRC)
  {
    return true;
  }

  return false;

}




void do_command()
{
  if (recv_packet.Module == MODULE_ID){


    switch (recv_packet.Command)
    {
      case CMD_PING:
      //send_response(RESPONSE_PONG);
      //Serial.println("PONG");
      recv_packet.Datalength = 0; //no data is sent just the pong responce command
      send_response(RESPONSE_PONG);
      break;

      case CMD_FIRE:
      //Serial.println("Fire!");
      fire_channel();
      break;

      case CMD_TEST:
      //test_channel(recv_packet.Data, recv_packet, Datalength);
      break;

      case CMD_TESTALL:
      //test_all_channels();
      break;

      case CMD_FIRE_ENABLE:
      //not implimented yet
      digitalWrite(testPin, LOW);
      digitalWrite(firePin, HIGH);
      //lcd.clear();
      lcd.backlight();
      lcd.setCursor(9,0);
      lcd.print("ARMED");
      displayTimer = false;
      fire_armed_remote = true;
      break;

      case CMD_TEST_ENABLE:
      //not implimented yet
      digitalWrite(firePin, LOW);
      digitalWrite(testPin, HIGH);
      //lcd.clear();
      lcd.backlight();
      lcd.setCursor(9,0);
      lcd.print("TEST ");
      displayTimer = false;
      test_armed_remote = true;
      break;

      case CMD_FIRE_DISABLE:
      //not implimented yet
      digitalWrite(firePin, LOW);
      //lcd.clear();
      lcd.setCursor(9,0);
      lcd.print("SAFE ");
      //Turn the display timer back on and reset the timer
      displayTimer = true;
      previousMillis = millis();
      fire_armed_remote = false;
      break;

      case CMD_TEST_DISABLE:
      //note implimented yet
      digitalWrite(testPin, LOW);
      //lcd.clear();
      lcd.setCursor(9,0);
      lcd.print("SAFE ");
      //Turn the display timer back on and reset the timer
      displayTimer = true;
      previousMillis = millis();
      test_armed_remote = false;
      break;

      case CMD_GET_FIRE_SW:
      //send_response(RESPONSE_FIRE_SW_STATE);
      break;

      case CMD_GET_TEST_SW:
      //send_response(RESPONSE_TEST_SW_STATE);
      break;

      case CMD_GET_CHANNEL_COUNT:
      // = CHANNEL_COUNT;
      recv_packet.Datalength = 4;
      recv_packet.Data[0] = CHANNEL_COUNT;
      recv_packet.Data[1] = 1;//Hardware Version
      recv_packet.Data[2] = 0;//Major software Version
      recv_packet.Data[3] = 3;//Minor software Version


      //recv_packet.CRC = CRC8(recv_packet.Data, recv_packet.Datalength);
      send_response(RESPONSE_CHANNEL_COUNT);
      break;

      case CMD_SET_ID:
      EEPROM.update(0,recv_packet.Data[0]);
      MODULE_ID = recv_packet.Data[0];
      recv_packet.Datalength = 1;
      recv_packet.Data[0]=EEPROM.read(0);
      setlcdModule();
			send_response(RESPONSE_ID_SET);
      break;

      default:
      {
        recv_packet.Datalength = 0;
        send_response(RESPONSE_BAD_COMMAND);
      }
    }
  }
}

void setlcdModule()
  {
    if(MODULE_ID <= 9){
      lcd.setCursor(15,1);
    }else if(MODULE_ID<=99){
      lcd.setCursor(14,1);
    }else if(MODULE_ID <=255){
      lcd.setCursor(13,1);
    }
    lcd.print(MODULE_ID);
  }

void sendStructure( char *structurePointer, int structureLength)
{
  digitalWrite(rx_en, HIGH);
  int i;

  for (i = 0 ; i < 3+recv_packet.Datalength ; i++){

    Serial.print(structurePointer[i], DEC);

    Serial.print(",");



  }
  //Serial.print(recv_packet.CRC);

  //Serial.println();
}



void send_response(byte response_type)
{

  recv_packet.Module = 255;
  recv_packet.Command = response_type;
  //CRC the whole packet
  recv_packet.CRC = CRC8((byte*)&recv_packet, sizeof(recv_packet)-1);
  //write the packet to the serial port
	digitalWrite(rx_en, HIGH);
  Serial.write((byte*)&recv_packet, sizeof(recv_packet));
	delay(100);



}

void fire_channel()
{
  if(!fire_armed_remote){
    send_response(RESPONSE_FIRING_DISABLED);
    return;
  }
  //uint16_t value = recv_packet.Data[0] *256 + recv_packet.Data[1];

  //Serial.println(value);

  //Set latchpin and shift data out to the cues
  digitalWrite(latchPin, LOW);
  //shiftOut(dataPin, clockPin, MSBFIRST, value);
  shiftOut(dataPin, clockPin, MSBFIRST, recv_packet.Data[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, recv_packet.Data[1]);
  digitalWrite(latchPin, HIGH);
  delay(FIRE_DELAY);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, recv_packet.Data[0] - recv_packet.Data[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, recv_packet.Data[1] - recv_packet.Data[1]);
  digitalWrite(latchPin, HIGH);
  //digitalWrite(latchPin, LOW);
}



void Rx_Decode(){ //  We process sending received on UART.

  //  For exchange indication on UART.
  //digitalWrite(TX_RX_LED, HIGH);
  //time_VD_stop = millis() +  time_set_VD;
  //TX_RX_SET = 1;

  buf[rx_count] = Serial.read();
  //Serial.println(rx_bayt, DEC);

  //Serial.rpint
  //recv_packet.header=buf[rx_count];

  if (buf[rx_count] == 0xAA && rx_count ==0)
  {
    //CRC8=0;
    //recv_packet.Header = buf[0];

    SOF = false;
    rx_count++;
  }
  else if(rx_count == 1 && SOF==false)
  {
    recv_packet.Module = buf[1];
    //CRC_8(recv_packet.slat);

    rx_count++;
  }
  else if(rx_count == 2 && SOF==false)
  {
    recv_packet.Command = buf[2];
    //CRC_8(recv_packet.cue);
    rx_count++;
  }
  else if(rx_count == 3 && SOF==false)
  {
    recv_packet.Datalength = buf[3];
    //Serial.println("data length");
    //Clear the data array and zero it out so we get the correct crc
    for(int i = 0; i>10; i++)
    {
      recv_packet.Data[i] = 0;
    }
    rx_count++;
  }else if(rx_count < recv_packet.Datalength+4 && SOF==false)
  {
    //Serial.println("DAta");
    recv_packet.Data[rx_count-4] = buf[rx_count];
    rx_count++;
  }else if(rx_count == recv_packet.Datalength+4 && SOF==false)
  {
    recv_packet.CRC = buf[rx_count];
    #ifdef DEBUG
			digitalWrite(rx_en,HIGH);
      Serial.println("======Packet Start======");
      Serial.print("Module id = ");
      Serial.println(recv_packet.Module,DEC);
      Serial.print("Command = ");
      Serial.println(recv_packet.Command,DEC);
      Serial.print("Datalength = ");
      Serial.println(recv_packet.Datalength,DEC);
      Serial.print("Data = ");

      //for(int i =0; i < recv_packet.Datalength; i++){
      for(int i =0; i < 10; i++){
        Serial.print(recv_packet.Data[i],DEC);
        Serial.print(",");
      }

      Serial.println();
      Serial.print("CRC = ");
      Serial.println(recv_packet.CRC,HEX);
      Serial.println("=======Packet End=======");
    #endif

    if (packet_decode()){
      do_command();
      SOF = true;
      memset(&recv_packet,0,sizeof recv_packet );
      rx_count = 0;
    }else{

      //This is just for debuging.
      packet_error ++;
      //memset(recv_packet.Data,0,sizeof(recv_packet.Data));
      memset(&recv_packet,0,sizeof recv_packet);
      lcd.setCursor(0,1);
      lcd.println(packet_error,DEC);
      Serial.println(packet_error,DEC);
      SOF = true;
      rx_count = 0;
    }

    rx_count = 0;
    SOF = true;
  }



}








void loop() {
  // put your main code here, to run repeatedly:
  //mesh update must be called every cycle to keep the radio up
  //mesh.update();
  //Serial.print("LOOOOOPING");
  unsigned long currentMillis = millis();
  /* Check to see if the displaytimer is enabled. if it is we check to
  * check to see if we hit the time out. if we do then we want to turn
  * off the display to save power.
  */
  if (displayTimer){
    if (currentMillis - previousMillis >= interval) {
      //set the old millis to the current millis so it doesn't get confused
      lcd.noBacklight();
      previousMillis = currentMillis;
    }
  }

	//digitalWrite(rx_en, HIGH);
	//Serial.println("loop!");


  digitalWrite(rx_en, LOW);

  if(Serial.available() > 0) Rx_Decode();


  /*
  while(network.available()){
  RF24NetworkHeader header;
  network.read(header, &recv_packet, sizeof(recv_packet));
  do_command();
}
*/

}
