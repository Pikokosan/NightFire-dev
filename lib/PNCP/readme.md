# PNCP for Arduino

PNCP is the Pyro Network Control Protocol. This is a open source networking protocol developed by  Leon S. Searl for use with hobbiest to professional firework firing systems.


### Warning:
This is to be used at your own risk, we are not responsible for any harm to you or your property.
This is currently in it's alpha stage and should not be used in the field.


##

### Supported Hardware.

* Arduino Nano
* Ardunio Pro
* Arduino Uno(untested)
* Arduino Mega(untested)





| [MAX485]( https://datasheets.maximintegrated.com/en/ds/MAX1487-MAX491.pdf )	| Arduino Nano|           
|:-------------:|:---------------------:|
| Pin 1(RO)   	 | Pin 0 (RXD)			|
| Pin 2(RE)   	 | Pin 2 				|
| Pin 3(DE)   	 | Pin 3 				|
| Pin 4(DO)	  	 | Pin 1 (TXD) 			|
| Pin 5(GND)	   | GND					|
| Pin 6(A)		   | To next modules(A) 	|
| Pin 7(B)		   | To next modules(B)	|
| Pin 8(VCC)	   | 5V					|

##

## PNCP Manuals
| Document | Version|
|:-----------------------------------------:|:------|
| [PyroNetworkControlProtocol-DataLinkLayer]( ./PyroNetworkControlProtocol-DLL.pdf) | V0.01 |
| [PyroNetworkControlProtocol-Applicationlayer](./PyroNetworkControlProtocol.pdf)| V0.02|

###NOTE:
  The PyroNetworkControlProtocol-Applicationlayer document is only used to reference the application layer. all other layers have been deprecated by use of the DataLinkLayer.
