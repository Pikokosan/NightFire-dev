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
  Add display backlight timout.
  proper send_response commands

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
boolean displayTimer = true;


LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display


boolean response = false;
boolean command_recv = false;
boolean fire_armed_remote = false;
boolean test_armed_remote = false;
byte rx_count;

//Menu variables
MenuSystem ms;
Menu mm("   Main Menu");
MenuItem mm_mi1("Module ID");
MenuItem mm_mi2("Continuity");
MenuItem mm_mi3("Connection");
MenuItem mm_mi4("DEV DIAG");

Menu mu1(" Settings Menu");
MenuItem mu1_mi1("ID Prog");
MenuItem mu1_mi2("Wired/Wireless");



//Module 255 is always the master
int MODULE_ID  = 1;
boolean MENU = false;
boolean eXit = false;

//serial debug enable
#define DEBUG

//System setup
#define CHANNEL_COUNT 16
#define FIRE_DELAY 10
String Version = "0.2";

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
#define key1 10
#define key2 11
#define key3 12
#define key4 8


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
/*
//Timecode design
//255,           00:00:00
//broadcast,     mm:ss:ms
//pc->module no responce

#define CMD_SET_TIMECODE 12 //fstc
#define CMD_START_TIMECODE 13 //tbeg
#define CMD_STOP_TIMECODE 14 //tdis
*/


void send_response(byte response_type);
void fire_channel();

//Radio setup
RF24 radio(8,10);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

//Packet layout for wireless

struct recv_packet_type_wireless{
  uint8_t Command;
  uint8_t Module;
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

void on_linktype_selected(MenuItem* p_menu_item)
{

}

void on_devdiag_selected(MenuItem* p_menu_item)
{

}

void on_modprog_selected(MenuItem* p_menu_item)
{

}

void on_Moduleid_selected(MenuItem* p_menu_item)
{
  lcd.setCursor(15,1);
  //rest
  lcd.rightToLeft();
  lcd.print(MODULE_ID);
  lcd.leftToRight();
  delay(3000);
  //Serial.print("module id");

}

void on_Settings_selected(MenuItem* p_menu_item)
{

}

void on_Contcheck_selected(MenuItem* p_menu_item)
{

}

void on_Connection_selected(MenuItem* p_menu_item)
{

}


void setup() {
  // put your setup code here, to run once:
  //enable serial

  lcd.init();
  lcd.backlight();
  lcd.setCursor(4,0);
  lcd.print("Starting");
  //EEPROM.write(0,1);
  Serial.begin(9600);
  //enable pins
  pinMode(statusLed,  OUTPUT);
  pinMode(firePin, OUTPUT);
  pinMode(testPin, OUTPUT);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  pinMode(rx_en, OUTPUT);

  //pinMode(key1, INPUT_PULLUP);
  //pinMode(key2, INPUT_PULLUP);
  //pinMode(key3, INPUT_PULLUP);
  //pinMode(key4, INPUT_PULLUP);
  previousMillis = millis();
  /*
  Menu structre
  Menu mm("  Status Menu");
  MenuItem mm_mi1("Module ID");
  MenuItem mm_mi2("Continuity");
  MenuItem mm_mi3("Connection");
  MenuItem mm_mi4("DEV DIAG");

  Menu mu1(" Settings Menu");
  MenuItem mu1_mi1("ID Prog");
  MenuItem mu1_mi2("Wired/Wireless");

  */


  lcd.setCursor(6,1);
  lcd.print("V");
  lcd.print(Version);
  delay(3000);
  mm.add_item(&mm_mi1, &on_Moduleid_selected);
  mm.add_item(&mm_mi2, &on_Contcheck_selected);
  mm.add_item(&mm_mi3, &on_Connection_selected);
  mm.add_item(&mm_mi4, &on_devdiag_selected);
  mm.add_menu(&mu1);
  mu1.add_item(&mu1_mi1, &on_Settings_selected);
  mu1.add_item(&mu1_mi2, &on_linktype_selected);
  ms.set_root_menu(&mm);

  //displayMenu();
  lcd.clear();
  lcd.setCursor(6,1);
  lcd.print("SAFE");
  //lcd.noBacklight();
  //mesh.begin();

}


byte CRC8(volatile byte *data, byte len)
{
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
  test = CRC8(recv_packet.Data, recv_packet.Datalength);
  #ifdef DEBUG
    Serial.print("crc cal = ");
    Serial.println(test);
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
      lcd.clear();
      lcd.setCursor(4,1);
      lcd.print("!ARMED!");
      displayTimer = false;
      fire_armed_remote = true;
      break;

    case CMD_TEST_ENABLE:
      //not implimented yet
      digitalWrite(firePin, LOW);
      digitalWrite(testPin, HIGH);
      lcd.clear();
      lcd.setCursor(5,1);
      lcd.print("!TEST!");
      displayTimer = false;
      test_armed_remote = true;
      break;

    case CMD_FIRE_DISABLE:
      //not implimented yet
      digitalWrite(firePin, LOW);
      lcd.clear();
      lcd.setCursor(6,1);
      lcd.print("SAFE");
      //Turn the display timer back on and reset the timer
      displayTimer = true;
      previousMillis = millis();
      fire_armed_remote = false;
      break;

    case CMD_TEST_DISABLE:
      //note implimented yet
      digitalWrite(testPin, LOW);
      lcd.clear();
      lcd.setCursor(6,1);
      lcd.print("SAFE");
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
      recv_packet.Data[3] = 2;//Minor software Version


      recv_packet.CRC = CRC8(recv_packet.Data, recv_packet.Datalength);
      send_response(RESPONSE_CHANNEL_COUNT);
      break;

    default:
      {
        recv_packet.Datalength = 0;
        send_response(RESPONSE_BAD_COMMAND);
      }
    }
  }
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
  //Serial.printf("170,%s,%s\n",recv_packet.Module, recv_packet.Command );
//Serial.write((char * )&recv_packet, sizeof(recv_packet));
  sendStructure((char *)&recv_packet, sizeof(recv_packet));
  //Serial.write(recv_packet, sizeof(recv_packet));


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
  shiftOut(dataPin, clockPin, MSBFIRST, 0);
  shiftOut(dataPin, clockPin, MSBFIRST, 0);
  digitalWrite(latchPin,HIGH);
  digitalWrite(latchPin, LOW);
}

void displayMenu() {
  lcd.clear();
  lcd.setCursor(0,0);
  // Display the menu
  Menu const* cp_menu = ms.get_current_menu();

  //lcd.print("Current menu name: ");
  lcd.print(cp_menu->get_name());

  lcd.setCursor(0,1);

  lcd.print(cp_menu->get_selected()->get_name());
}

int readline(int readch, char *payload, int len)
{
  static int pos = 0;
  int rpos;
  //Serial.println("Readline called");

  if (readch > 0) {
    //Serial.println("Got data");
    switch (readch) {
      case '\n': // Ignore new-lines
        //Serial.println("found \n");
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        //Serial.println("found \r");
        return rpos;
      default:
        if (pos < len-1) {
          payload[pos++] = readch;
          payload[pos] = 0;
        }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
}


  void loop() {
    // put your main code here, to run repeatedly:
    //mesh update must be called every cycle to keep the radio up
    //mesh.update();
    //Serial.print("LOOOOOPING");
    unsigned long currentMillis = millis();
    if (displayTimer){
      if (currentMillis - previousMillis >= interval) {
        //set the old millis to the current millis so it doesn't get confused
        lcd.noBacklight();
        previousMillis = currentMillis;
      }
    }else
    {
      lcd.backlight();

    }

    digitalWrite(rx_en, LOW);


    //if (Serial.available() > 0) {
      static char payload[30];
      if(readline(Serial.read(), payload, 30) > 0)
      {

          #ifdef DEBUG
          digitalWrite(rx_en, HIGH);
          Serial.print("You entered: >");
          Serial.print(payload);
          Serial.println("<");

          #endif
      //payload = Serial.readString();
      //TEST+++++++++=++++++++++++++++++++++++++
           char delimiter[] =",";
           char* valPosition;
           int i = 0;
           int offset = 0;
           int packed[10];
           //char* test = payload;

           //This initializes strtok with string to tokenize
           valPosition = strtok((char*)payload, delimiter);
           //Serial.println("=======tokenizing==========");

           while(valPosition != NULL){
             //Serial.println(valPosition);
             packed[i++] = atoi(valPosition);

             //Here we pass in a NULL value, which tells strtok to continue working with the previous string
             valPosition = strtok(NULL, delimiter);
           }

           //Find packet offset
           for(int y = 0; payload[y] == 170; y++ ){
             offset++;
           }
           recv_packet.Module = packed[1];
           recv_packet.Command = packed[2];
           recv_packet.Datalength = packed[3];
           offset = offset +3;
           for(int x = 0; x < recv_packet.Datalength; x++ )
           {

             recv_packet.Data[x] = packed[x+4];
             //Serial.print("Tick = ");
             //Serial.println(recv_packet.Data[x]);
           }

           recv_packet.CRC = packed[recv_packet.Datalength+4];
          #ifdef DEBUG
            Serial.println("======Packet Start======");
            Serial.print("Module id = ");
            Serial.println(recv_packet.Module);
            Serial.print("Command = ");
            Serial.println(recv_packet.Command);
            Serial.print("Datalength = ");
            Serial.println(recv_packet.Datalength);
            Serial.print("Data = ");
            for(int i =0; i < recv_packet.Datalength; i++){
              Serial.print(recv_packet.Data[i]);
              Serial.print(",");
            }



            Serial.println();

            Serial.print("CRC = ");
            Serial.println(recv_packet.CRC);
            Serial.println("=======Packet End=======");

           #endif
           if(packet_decode()){
             do_command();
           }


      //if (packet_decode() == true) do_command();

    }

  /*
while(network.available()){
  RF24NetworkHeader header;
  network.read(header, &recv_packet, sizeof(recv_packet));
  do_command();
}
*/

  }
