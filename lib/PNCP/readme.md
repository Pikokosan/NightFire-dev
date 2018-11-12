# PNCP for Arduino

PNCP is the Pyro Network Control Protocol. This is a open source networking protocol developed by  Leon S. Searl for use with hobbiest firework firing systems.


### Warning:
This is currently in it's alpha stage and should not be used in the field. Use at your own risk, we are not responsible for any harm to you or your property.



##

### Supported Hardware.

* Arduino Nano
* Ardunio Pro
* Arduino Uno(untested)
* Arduino Mega(untested)



## Libraies to download if using Arduino IDE

| Name/Link | What its for |
|:-----:|:----------|
|[Battery Sense](https://github.com/rlogiacco/BatterySense)|This is used for sensing the main batteries voltage. **note:** this is available in the arduino library manager.|
|[Liquidcrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C) | This library is used for controlling the I2C 16x2 lcd. **note:** this is available in the arduino library manager.|
| [MC33996](https://github.com/Pikokosan/MC33996) | This library is include but is also available in the platfromio library manager.|

## Pinouts


 Arduino | [MAX485]( https://datasheets.maximintegrated.com/en/ds/MAX1487-MAX491.pdf )	|          
|:-------------:|---------------------|
|Pin 0 (RXD)			| Pin 1(RO)   	 |
| Pin 3 				| Pin 3(RE_DE)   |
|Pin 1 (TXD) 			| Pin 4(DO)	  	 |
|GND					| Pin 5(GND)	   |
|Pin 1 of RJ-45	        | Pin 6(A)		   |
| Pin 2 of RJ-45		| Pin 7(B)		   |
| 5V					| Pin 8(VCC)	   |

## MC33996
| Arduino 	           | [MC33996ek](https://www.nxp.com/docs/en/data-sheet/MC33996.pdf) |
|:--------------------:|:-----|
| 5v		           | SOPWR |
| D10(USER DEFINABLE)  | CS    |
| MISO      		   | SI    |
| MOSI      		   | SO    |
| N/A       		   | PWM   |
| D9(USER DEFINABLE)   | RST   |
| 12-24v               | VPWR  |

For output pinout see the dataseet linked above for more information.
## RJ-45 pinout

|PIN| FUNCTION|
|:--------:|---------|
|1| RS-485 A|
|2| RS-485 B|
|3| TBD|
|4| 12-24V|
|5| 12-24V|
|6| TBD|
|7| GND|
|8| GND|


##

## PNCP Manuals
**NOTE:** The PyroNetworkControlProtocol-Applicationlayer document is only used to reference the application layer. all other layers have been deprecated by use of the DataLinkLayer.

| Document | Version|
|:-----------------------------------------:|:------|
| [PyroNetworkControlProtocol-DataLinkLayer]( ./docs/PyroNetworkControlProtocol-DLL.pdf) | V0.01 |
| [PyroNetworkControlProtocol-Applicationlayer](./docs/PyroNetworkControlProtocol.pdf)| V0.02|
