#include "PNCP.h"
//#include "Subpayload.h"
#include <Arduino.h>
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  #include <util/crc16.h>
#endif

#define lo8(x) ((x)&0xff)
#define hi8(x) ((x)>>8)



/*!
  \brief intialize the library
  \details This is used to intialize your devices addresses.

   \param GADD is the group address of the device.
   \param UADD is the Unique address of the device.
   \sa setGADD()
  */
PNCP::PNCP(uint8_t GADD, uint32_t UADD)
{

  _UADD = UADD;
  _GADD = GADD;
  _discover = true;

  //packet_status_t packet_status;


}

/*! \brief Used to setup the rs-485 chip.
    \details This only needs to be called during setup()
    \param baud Baud rate for UART.
    \param port Hardware serial port. Defined 0-4
    \param RE_DE_pin_def Data enable pin used for the rs-485 chip.

*/


void PNCP::begin( long baud, size_t port, uint8_t RE_DE_pin_def)
{
  this->_timeout  = TIMEOUT;
  this->_RE_DE_pin = RE_DE_pin_def;
  //this->_RE_pin = RE_pin_def;
  //pinMode(_RE_pin, OUTPUT);
  pinMode(_RE_DE_pin, OUTPUT);
  //digitalWrite(_RE_pin, LOW);
  digitalWrite(_RE_DE_pin, LOW);
  switch(port)
        {
        #if defined(UBRR1H)
            case 1:
                Serial1.begin(baud);
                _serial = &Serial1;
                break;
        #endif
        #if defined(UBRR2H)
            case 2:
                Serial2.begin(baud);
                _serial = &Serial2;
                break;
        #endif
        #if defined(UBRR3H)
            case 3:
                Serial3.begin(baud);
                _serial = &Serial3;
                break;
        #endif
            default:
                Serial.begin(baud);
                _serial = &Serial;
        }

}
void PNCP::begin(Stream* serial)
{
  _serial = serial;
}

/*! \brief Used to check available packets.
    \details This should be called frequently to beable to use received packets.
    \code{.cpp}
      if(pncp.available){
      //Do stuff with the payload.
    }
    \endcode
    \return True if the packet is for us and CRC is correct.
    \sa update()
*/
bool PNCP::available()
{
  if(this->_Complete == true && this->_forus == true)
  {
    this->_Complete = false;
    this->_forus = false;
    //reststatus();
    return true;
  }else{
    return false;
  }

}

/*! \brief Collects and decodes packets.
    \details This should be called frequently so that packets are read and decoded.
    \code{.cpp}
      void loop()
      {
      //called every time the main loop starts
      pncp.update();
    }
    \endcode

    \sa available()
*/

void PNCP::update(){
  digitalWrite(_RE_DE_pin, LOW);
  //digitalWrite(_RE_pin, LOW);
  //Serial.println("in the phy idle");
  if(_serial == 0)
    {
      return;
    }
  else
    {
      if(this->_Complete == true)
      {
        //this->reststatus();
      }

      this->recieve();
    }


}

void PNCP::setCallback(void(*errorCallback)(byte PHY_error_status))
{
  _errorCallback = errorCallback;
}


/*! \brief Set group address.
    \details This can be called to change the group address of the device
  \code{.cpp}
  int temp = setGADD(10);
  if(temp == 10){
  //we changed the GADD
  }
  \endcode
  \param GADD new group address.
  \return The new group address.
    \sa PNCP(), getGADD()
*/
uint8_t PNCP::setGADD(uint8_t GADD)
{
  this->_GADD = GADD;
  return this->_GADD;
}

/*! \brief Get group address.
  \details This is used to get the currently set Group address
  \code{.cpp}
  int temp = getGADD();

  \endcode
  \return The current group address.
    \sa setGADD()
*/
uint8_t PNCP::getGADD()
{
  //this->_GADD = GADD;
  return this->_GADD;
}





void PNCP::recieve()
{


  while(_serial->available() > 0)
  //if(data != -1)
  {

    this->_start = millis();
    uint8_t data = _serial->read();

    //_serial->println("recieve loop");
    delay(10);

    //if(!data){
    //  _serial->println("no data timeout");

    // return;
   //}



    if(this->_status == SYNC)
    {

      switch(data)
      {
        case SOFB:
        this->_status = HEADER;
        //_serial->println("got Boadcast sync");
        this->_Address_type = data;
        break;

        case SOFG:
        this->_status = ADDRESS;
        //_serial->println("got group sync");
        this->_Address_type = data;
        break;

        case SOFR:
        //Ignore responce commands
        this->_status = SYNC;
        //this->_Address_type = data;
        break;

        case SOFU:
        this->_status = ADDRESS;
        //_serial->println("got unique sync");
        this->_Address_type = data;
        break;

        default:
        break;

      }
      return;
    }

    if(this->_status == ADDRESS)
    {
      //Check to make sure the byte isn't an encoded byte
      data = this->decode(data);

      if(this->_Address_type == SOFG){
        //_serial->println("got group address");
        //Check to make sure the byte isn't an encoded byte
        data = this->decode(data);
        this->frame.address.GADD = data;
        if(this->frame.address.GADD == _GADD) this->_forus = true;
        //this->_CalCrc = this->CRC15(&this->frame.address.GADD, 1, this->_CalCrc);
        //this->_CalCrc =   _crc_ccitt_update(this->_CalCrc, data);
        this->_CalCrc = this->CRC16(&this->frame.address.GADD, 1);
      }
      if(this->_Address_type == SOFU)
      {
        //_serial->println("got unique address");
        if(_addresslength != 4)
        {
          //Check to make sure the byte isn't an encoded byte
          data = this->decode(data);
          this->frame.address.UADD[_addresslength] = data;
          //this->_CalCrc = this->CRC16(data, 1, this->_CalCrc);

          _addresslength++;
          if(this->_addresslength == 4)
          {
            if(this->frame.address.LUADD == _UADD) this->_forus = true;

            //DEBUG
            /*
            _serial->print("LUADD = ");
            _serial->println(this->frame.address.LUADD,HEX);
            _serial->print("_UADD = ");
            _serial->println(this->_UADD,HEX);
            _serial->print("UADD[] = ");
            */

            this->_CalCrc = this->CRC16(this->frame.address.UADD, 4, this->_CalCrc);
            this->_addresslength = 0;
          }else{

            return;
          }
        }

      }

      this->_status = HEADER;
      return;


    }

    if(_status == HEADER )
      {
        //_serial->println("got header");
        //this->headerdecode(data);
        //Check to make sure the byte isn't an encoded byte
        data = this->decode(data);


        this->frame.header.PRMS = data;

        //crc-16 calculate
        this->_CalCrc = this->CRC16(&data, 1, this->_CalCrc);
        //this->_CalCrc = _crc_ccitt_update(0, data);
        if(this->frame.header.FV)
        {
          //_errorCallback;
          _serial->println("got header error");
          this->_status = SYNC;
          return;
        }

        if(this->frame.header.PT)
        {
          //_serial->println("payload");
        // Check to see if its version 1.0 if ot call error.
        this->_internal = false;
        this->_status = PAYLOAD;
        return;
      }else
      {
        //_serial->println("subpayload");
        this->_internal = true;
        this->_status = PAYLOAD;
        return;
      }
      return;

      }




      if(this->_status == PAYLOAD)
      {
        //Check to make sure the byte isn't an encoded byte
        data = this->decode(data);
        uint8_t size = PFLCal(this->frame.header.PLI);



        /*
        _serial->print("cal size = ");
        _serial->println(size,DEC);
        _serial->print("PFI count = ");
        _serial->println(this->_PFI, HEX);
        */
        if(this->_PFI != size)
        {
          this->frame.PLD[this->_PFI] = data;
          //_serial->print(data,HEX);
          this->_PFI++;
          return;

        }else{
          this->_PFI = 0;
          this->_status = CRC_P;


        }

      }

      if(this->_status == CRC_P)
      {
        // CRC command loop.
        //_serial->println("CRC called");

        if(this->_CRCCount != 2)
        {
          this->frame.crc.BCRC[this->_CRCCount] = data;
          this->_CRCCount++;
          if(this->_CRCCount != 2)
          {
            return;
          }
        }

        //when crc is collected we check to see that it matches
        uint8_t size = PFLCal(this->frame.header.PLI);

        //uint16_t check = this->CRC15(&this->frame.header.PRMS, size);
        //check = this->CRC15(this->frame.address.UADD, uint8_t length, uint16_t crc)
        this->_CalCrc = this->CRC16(this->frame.PLD, size, this->_CalCrc);

        /*
        _serial->print("data packet = ");
        for(int i =0; i < size; i++)
        {
          _serial->print(" ");
          _serial->print(this->frame.PLD[i],HEX);
        }
        //this->_CalCrc = this->CRC15(this->frame.PLD, size, this->_CalCrc);
        _serial->println("CRC calculated = ");
        _serial->print(this->_CalCrc,HEX);
        _serial->print(",");
        _serial->println(this->frame.crc.LCRC,HEX);
        */

        //_serial->println(this->frame.crc.CRC[1],HEX);



        if(this->_CalCrc == this->frame.crc.LCRC)
        {
          if(!this->frame.header.PT)
          {
            // call sub commands
            //Library Sub payload
            // using call back functions so users can save commands

            //_serial->print("sub CMD = ");
            //_serial->println(this->frame.CMD, HEX);
            //_serial->print("random number = ");
            //_serial->println(Trandom(), DEC);
            //this->_Complete = true;
            //_serial->println("good crc");
            this->_CRCCount = 0;
            this->_status = SYNC;
            this->_CalCrc = 0;
            uint8_t size = PFLCal(this->frame.header.PLI);

            this->Subcommands(this->frame.PLD,size);

            return;
          }

          //User payload.
          this->_Complete = true;
          //_serial->println("good crc");
          this->_CRCCount = 0;
          this->_CalCrc = 0;
          this->_status = SYNC;
          return;
        }
        else
        {
          this->_Complete = false;
          this->_CRCCount = 0;
          #if defined(CRC_DEBUG)
            _serial->print("bad crc =");
            _serial->println(this->frame.crc.LCRC,HEX);
            _serial->print("CRC calculated = ");
            _serial->println(this->_CalCrc,HEX);
          #endif
          this->_CalCrc = 0;
          this->_status = SYNC;
          //CRC error
          return;

        }

        return;
      }



  }
  if(millis() -_start > TIMEOUT && this->_status != SYNC)
  {
    this->_status = SYNC;
    //_serial->flush();
    this->_Complete = false;
    this->_CRCCount = 0;
    _start = millis();
    _serial->print(this->_status,DEC);
    _serial->println("Timeout");

    return;
  }
    //this->_status = SYNC;
    //_serial->println("Timeout hit on read");

  }


  /*! \brief Get current UADD.
    \details This is used to get the devices unique address.
    \code{.cpp}
    int temp = pncp.getUADD();

    }

    \endcode
    \return The devices UADD.
  */
  uint32_t PNCP::getUADD()
  {
  return this->_UADD;

  }
  /*! \brief Get current payload size.
    \details This is used to get the current payload size.
    \note This can only be called when a packet is available.
    \code{.cpp}
    if(pncp.available())
    {
      int temp = pncp.getFramesize();
    }
    \endcode
    \return The current packets payload size.
    \sa available()
  */
  uint8_t PNCP::getFramesize()
  {
    uint8_t size = PFLCal(this->frame.header.PLI);
    return size;

  }


  /*! \brief Get current header.
    \details This is used to get the current header
    \note this is for debugging purposes.
    \code{.cpp}
    if(pncp.available())
    {
    int temp = pncp.getHeader();
    //do stuff with the header.
    }

    \endcode
    \return The current header.
    \sa available()
  */
uint8_t PNCP::getHeader()
{
  return this->frame.header.PRMS;

}

uint16_t PNCP::CRC16(const uint8_t *data, uint8_t length, uint16_t crc )
{
  //This function is build into avrlibc.
  for(int i = 0; i < length; i++)
  {
    crc = _crc_ccitt_update(crc, data[i]);
    #if defined(CRC_DEBUG)
    Serial.print("data= ");
    Serial.print(data[i],HEX);
    Serial.print(" crc check ");
    Serial.print(i ,DEC);
    Serial.print("= ");
    Serial.println(crc,HEX);
    #endif
  }

  return crc;
}

uint16_t PNCP::_crc_ccitt_update (uint16_t crc, uint8_t data)
{
    data ^= lo8 (crc);
    data ^= data << 4;

    return ((((uint16_t)data << 8) | hi8 (crc)) ^ (uint8_t)(data >> 4)
            ^ ((uint16_t)data << 3));
}

/*

old crc function. couldn't verify if it was giving the correct checksum

uint16_t PNCP::CRC15(const uint8_t *data, uint8_t length, uint16_t crc )
{
  //uint16_t crc = 0x0000;
  for(int i = 0; i < length; i++)
  {
    uint8_t j;
    crc ^= *data << 7;
    for(j=0; j < 8; j++)
    {
      crc <<= 1;
      if (crc & 0x8000)
      {
        //temp ^= 0xc599; //This is the polynominal used
        crc ^= 0x62CC;
      }
    }
    crc = crc & 0x7FFF;

  }
  return crc;
}
*/



uint8_t PNCP::PFLCal(uint8_t len)
{

uint8_t afli;
if(len < 5) {
  afli = len + 1;
}
else if (len < 5) {
  afli = ((len + 1) * 4) - 20;
}
else if (len < 8) {
  afli = (len -4) * 4;
}
else if (len < 12) {
  afli = (len - 8) *16;
}
else if (len < 15) {
  afli = (len -10) * 32;
}
else {
  // This is an error.
}
return afli;
}

void bufferwait(){

  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    #if defined(UBRR0H)
      while (!(UCSR0A & (1<<UDRE0)))   //  Wait for empty transmit buffer
      UCSR0A |= 1<<TXC0;          //  mark transmission not complete
      while (!(UCSR0A & (1<<TXC0))); //  wait for transmission to complete
    #endif

  #endif

  #if defined(__STM32F103C8T6__)
    Serial.println("Waiting to finish sending");
    usart_reg_map *regs = USART1->regs;
    while ( !(regs->SR & USART_SR_TC) ); // Wait for Transmission Complete to set
  #endif
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  #if defined(UBRR0H)
    while (!(UCSR0A & (1<<UDRE0)))   //  Wait for empty transmit buffer
    UCSR0A |= 1<<TXC0;          //  mark transmission not complete
    while (!(UCSR0A & (1<<TXC0))); //  wait for transmission to complete
  #endif
  #if defined(UBRR1H)
    while (!(UCSR1A & (1<<UDRE1)))   //  Wait for empty transmit buffer
    UCSR1A |= 1<<TXC1;          //  mark transmission not complete
    while (!(UCSR1A & (1<<TXC1))); //  wait for transmission to complete
  #endif
  #if defined(UBRR2H)
    while (!(UCSR2A & (1<<UDRE2)))   //  Wait for empty transmit buffer
    UCSR2A |= 1<<TXC2;          //  mark transmission not complete
    while (!(UCSR2A & (1<<TXC2))); //  wait for transmission to complete

  #endif
  #if defined(UBRR3H)
  while (!(UCSR3A & (1<<UDRE3)))   //  Wait for empty transmit buffer
  UCSR3A |= 1<<TXC3;          //  mark transmission not complete
  while (!(UCSR3A & (1<<TXC3))); //  wait for transmission to complete

  #endif

  #endif

}


uint8_t PNCP::decode(uint8_t data)
{
  //uint8_t dump;
  if(data == 0xFF)
  {
    //_serial->println("data was 0xFF");
    uint8_t tloop = 0;
while(tloop<20){
  //_serial->println("loop");
    if(_serial->available()>0){

    switch(_serial->peek()){
    _serial->print("peek() = ");
    _serial->println(_serial->peek(),HEX);
      case 0xFA:
      _serial->read();
      _serial->println("case FA");
      return 0x5A;
      break;

      case 0xFB:
      _serial->read();
      _serial->println("case FB");
      return 0x78;
      break;

      case 0xFC:
      _serial->read();
      _serial->println("case FC");
      return 0x47;
      break;

      case 0xFD:
      _serial->read();
      _serial->println("case FD");
      return 0x55;
      break;

      case 0xFE:
      _serial->read();
      _serial->println("case FE");
      return 0xFF;
      break;

      default:
      return data;
      break;

  }

}else
{
  tloop++;
}
}
  }
  return data;
}

/*! \brief Used to send packets.
    \details call this function when writing a packet back to the master.
    \warning This is currently broken.

    \attention Slave modules should not send packets unless asked to.
    \param *PLD should be a byte array of data based on the PNCP application layer
    \param size this is the size of the array being sent
    \return True if the packet was sent correctly.
*/
bool PNCP::write(uint8_t *PLD, uint8_t size)
{
  //uint8_t temp;
  //if(this->_GSlot<0)
  //{
  byte buf[100];



  union{
    struct{
      bool CF  : 1;  // CRC flag
      bool RSV : 1;  // Reserved
      bool PT  : 1;  // payload type
      uint8_t PLI : 4;  // payload length index
      bool FV  : 1;  // frame version
      };
    uint8_t PRMS;
  }write_header;
  write_header.CF = true;
  write_header.PT = true;
  write_header.FV = false;
  write_header.PLI = PFLCal(size);

  this->frame.address.LUADD = _UADD;

  buf[0] = this->frame.address.UADD[0];
  buf[1] = this->frame.address.UADD[1];
  buf[2] = this->frame.address.UADD[2];
  buf[3] = this->frame.address.UADD[3];
  buf[4] = write_header.PRMS;


  for(int e = size; e > 0; e--)
  {
    //int d = size-1;
    buf[e+4] = PLD[size-e];
  }

  this->_CalCrc =0;
  this->_CalCrc = this->CRC16(buf, size+5, this->_CalCrc);

  //buf[size+5] = this->_CalCrc;



    unsigned long seed=seedOut(31);

    randomSeed(seed);
    this->_GSlot = random(1,100);


  uint8_t dump = _serial->read();
  delay(this->_GSlot * 3);
  while(dump < 0){
    //uint8_t dump = _serial->read();
    _serial->flush();
    //_serial->println("stuck");
    dump = _serial->read();
  delay(this->_GSlot * 3);
}
  digitalWrite(_RE_DE_pin, HIGH);
  //digitalWrite(_RE_pin, HIGH);

  _serial->write(SOFR);
  _serial->write(buf,size+5);//test write
  _serial->write((byte*)&this->_CalCrc,sizeof(this->_CalCrc));

  bufferwait();
  this->_CalCrc =0;




  return true;

}

//void PNCP::setCallback(void(*errorCallback)(byte PHY_error_status))
//{
//  _errorCallback = errorCallback;
//}

uint8_t PNCP::Trandom()
{
  uint16_t temp;
  uint8_t ranout;
  for(int i = 0; i < 7; i++)
  {
    temp = analogRead(0);
    temp = temp << 1;

    ranout = ranout+(uint8_t)temp;

  }

  return ranout;


}

unsigned int PNCP::bitOut(void)
{
  static unsigned long firstTime=1, prev=0;
  unsigned long bit1=0, bit0=0, x=0, port=0, limit=99;
  if (firstTime)
  {
    firstTime=0;
    prev=analogRead(port);
  }
  while (limit--)
  {
    x=analogRead(port);
    bit1=(prev!=x?1:0);
    prev=x;
    x=analogRead(port);
    bit0=(prev!=x?1:0);
    prev=x;
    if (bit1!=bit0)
      break;
  }
  return bit1;
}
//------------------------------------------------------------------------------
unsigned long PNCP::seedOut(unsigned int noOfBits)
{
  // return value with 'noOfBits' random bits set
  unsigned long seed=0;
  for (int i=0;i<noOfBits;++i)
    seed = (seed<<1) | bitOut();
  return seed;
}




/*! \brief This is used for Sub payload command.
  \details We use this to run sub command outside the users space without the user having to write code.
  \note this is a work in progress.
  \return Nothing.
*/

void PNCP::Subcommands(uint8_t*data, uint8_t size)
{
  uint8_t packet[2];
  switch(this->frame.CMD){

    case 0x01:
      //Get GADD
      packet[0] = 0;//ACK/NACk
      packet[1] = this->getGADD();
      this->write(packet,2);
      return;

    case 0x02:
      //Set GADD
      this->setGADD(data[1]);
      this->write(0,1);
      //_serial->print("set command = ");
      //_serial->println(data[1]);
      //this->write(uint8_t *PLD, uint8_t size)
  }



}
