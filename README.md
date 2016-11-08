NightFire Rev 4 development board firmware.


This firmware is for a hobby grade digital fireworks firing system. it currently uses rs-485 with a custom protocol for communicating with the Pyro Ingnition Controler beta 2.0. which is currently not publicly available. 

later iterations will use the nrf24l01+ or rfm69hcw wireless transceiver.

Currently we are using a simple Python script to send and recieve data from the modules using a cheap ftdi232 -> rs485 module.

New's and update info can be found at nightflyerfireworks.com/blog


Harware:
Arduino Nano atmega328@16Mhz
NightFire-rev 4 Dev board.
1 x 2x16 i2c lcd

IDE:
atom with platformio*

NOTE: Arduino ide can be used but libraries will need to be installed. see platformio.ini for needed libraries



Finished:
Arm and disarm.
firing commands.
Module ID setting.
device information responce.

Not finshed:
Wireless communication.
Timecode usage.
pyro Ignition control plugin.







