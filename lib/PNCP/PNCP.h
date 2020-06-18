#ifndef PNCP_h
#define PNCP_h

#include "Arduino.h"
//#include "Subpayload.h"

#define SOFB 0x55 // Boadcast
#define SOFG 0x47 // Group address
#define SOFU 0x78 // Unique address
#define SOFR 0x6A // Responce

#define TIMEOUT 200

#define BUFFERSIZE 60

class PNCP{
public:
  //friend class Subpayload;


  struct FRAME {
    union{
      struct{
        uint8_t CF  : 1;  // CRC flag
        uint8_t RSV : 1;  // Reserved
        uint8_t PT  : 1;  // payload type
        uint8_t PLI : 4;  // payload length index
        uint8_t FV  : 1;  // frame version
      };
      uint8_t PRMS;
    }header;
    union{

      uint8_t GADD;     // Group address
      uint8_t UADD[4];
      uint32_t LUADD;      // Unique address


    }address;
    union{
      struct{
        uint8_t RSV : 4; //This is used for subframe commands only
        uint8_t CMD : 4;
      };
    uint8_t PLD[BUFFERSIZE];  // Payload
  };
    union{
      /*
      struct{

        uint8_t CRC[2];

      };
      */
      uint8_t BCRC[2];
      uint16_t LCRC;
    }crc;  // CRC 16bit
  }frame;


  PNCP(uint8_t GADD, uint32_t UADD);
  void begin(long baud, size_t port = 0, uint8_t RE_DE_pin_def = 2);
  //void begin(Stream* serial);
  void update();
  bool available();
  uint8_t setGADD(uint8_t GADD);
  uint8_t getGADD();
  uint32_t getUADD();
  uint8_t getHeader();
  uint8_t getFramesize();

  void setCallback(void (*errorCallback)(byte PHY_error_status));
  bool write(uint8_t *PLD, uint8_t size);
  uint8_t _Address_type;




private:
  enum packet_status_t : uint8_t {SYNC,HEADER,ADDRESS,PAYLOAD,CRC_P,SUBPAYLOAD};
  enum packet_status_t _status;
  uint8_t _RE_DE_pin;
  //uint8_t _RE_pin;

  uint8_t _addresslength = 0;
  //uint8_t _status;
  uint32_t _UADD;
  uint8_t _GADD;
  uint8_t _PFI;
  uint8_t _CRCCount;
  uint16_t _CalCrc = 0x0000;
  uint16_t _start;
  uint16_t _timeout;
  uint16_t _GSlot;

  //bool _internal;

  uint16_t CRC15(const uint8_t *data, uint8_t length, uint16_t crc = 0x0000);
  uint16_t CRC16(const uint8_t *data, uint8_t length, uint16_t crc = 0x0000);
  uint16_t _crc_ccitt_update (uint16_t crc, uint8_t data);
  uint8_t PFLCal(uint8_t len, bool send = false);
  uint8_t decode(uint8_t data);
  uint16_t timedread();
  uint8_t Trandom();
  void begin(Stream* serial);
  void recieve();
  void headerdecode(uint8_t data);
  unsigned int bitOut(void);
  unsigned long seedOut(unsigned int noOfBits);
  void Subcommands(uint8_t*data, uint8_t size);





  bool _addressed;
  bool _discover;
  bool _sync;
  bool _Complete;
  bool _header;
  bool _forus;


  Stream* _serial;

  void (*_errorCallback)(byte PHY_error_status);


};

#endif
