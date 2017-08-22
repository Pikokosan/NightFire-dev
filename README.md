NightFire Rev 4 development board firmware.

see https://pikokosan.github.io/NightFire-dev/ for Arduino-pncp library info

This firmware is for a hobby grade digital fireworks firing system. it currently uses rs-485 with a custom protocol for communicating with the Pyro Ingnition Controler beta 2.0. which is currently not publicly available.

later iterations will use the nrf24l01+ or rfm69hcw wireless transceiver.

Currently we are using a simple Python script to send and recieve data from the modules using a cheap ftdi232 -> rs485 module.
