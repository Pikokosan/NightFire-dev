NightFire Rev 4 development board firmware.

<img src="https://travis-ci.org/Pikokosan/NightFire-dev.svg?branch=master" alt="Build Status" />

see https://pikokosan.github.io/NightFire-dev/ for Arduino-pncp library info

This firmware is for a hobby grade digital fireworks firing system. it currently uses RS-485 with a custom protocol for communicating with the Pyro Ignition Controller beta 2.0. which is currently not publicly available.

later iterations will use the NRFf24L01+ or RFM69HCW wireless transceiver.

Currently we are using a simple Python script to send and receive data from the modules using a cheap FTDI232 -> RS-485 module.

To use in the Arduino IDE download and copy the /lib/PNCP folder to your Arduino sketch library directory. rename main.cpp to Nightfire.ino

## RJ45 pinout

|RJ45 PIN| FUNCTION|
|--------|---------|
|1| RS-485 A|
|2| RS-485 B|
|3| TBD|
|4| 12-24V|
|5| 12-24V|
|6| TBD|
|7| GND|
|8| GND|
