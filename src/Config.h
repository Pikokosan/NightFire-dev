//#define APPL_DEBUG
//#define MAIN_DEBUG
//#define CRC_DEBUG
//#define DEBUG
//Uncomment to set up the devices eeprom. then comment and reflash
// Pikoko Unique address 0x14CAC701-FF
//#define firstrun
#define firstGADD         0x01
#define firstUADD         0x14CAC704
#define firstCapabilities 0xA0050200

//Chip used for controlling Cue ouputs
#define USE_MC33996 // if this is not defined use MCP23008

//#define USE_MCP23008


#if defined(USE_MC33996)
  #define USE_MC33996_CONTINUITY_CHECK//This is optional only for the MC33996
#endif

//*******************Packet information*********************//
//high2
/*
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

#define SOFB 0x55 // Boadcast
#define SOFG 0x47 // Group address
#define SOFU 0x78 // Unique address
#define SOFR 0x6A // Responce
*/
//*******************End packet information******************//
