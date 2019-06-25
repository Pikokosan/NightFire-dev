//#include <arduino.h>

//#define APPL_DEBUG
//#define DLL_DEBUG
//#define MAIN_DEBUG
//#define CRC_DEBUG
//#define DEBUG
//Uncomment to set up the devices eeprom. then comment and reflash
// Pikoko Unique address 0x14CAC701-FF

//#define firstrun
#define firstGADD         0x01
#define firstUADD         0x14CAC704
#define firstCapabilities 0x10f0001a
//#define firstCapabilities 0xA0050200


/*
#ifdef firstrun
struct device_config
{
    union
    {
        long capabilities;
        unsigned char Bytecapabilities[4];
        struct
        {
            unsigned int :  4;          // ACK/NACK(this is for packeting purposes)
            bool CONT: 1;               // Report continuty flag
            bool REST: 1;               // Report cue resistance flag
            bool VOLT: 1;               // Report input voltage flag
            bool QSCH: 1;               // Time and cue scheduleing flag
            bool CHGF: 1;               // Charge cue flag 0 = no caps
            bool RSV2: 1;               // Reserved
            unsigned int CHGT: 6;       // Time to charge caps(capcitive discharge system)
            unsigned int PLSL: 4;       // Firing pulse time 0 = not adjustable
            unsigned int RSV:  4;       // Reserved
            unsigned int NCUE: 8;       // Number of cues
        };
    };

};
typedef struct device_config Device_config;


Device_config data;

// AREA FOR DEVICE CONFIGURATION
  //data.capabilities = 0xa005010;

  data->NCUE = 16; // number of cues
  data.PLSL = 5; //Cue pulse
  data.CHGF = 0; // charge cue flag
  data.CHGT = 0; // charge cue time 0 - 62
  data.QSCH = 0; // Cue scheduleing flag
  data.VOLT = 1; // volatge testing flag
  data.CONT = 1; // continuty testing flag
  long firstCapabilities = data;
#endif
*/



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
