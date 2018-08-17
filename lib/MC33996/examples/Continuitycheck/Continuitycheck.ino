#include <MC33996.h>

#define CHIPSELECT (10) //chip select pin.
#define RESTPIN    (9) //Chip reset pin.

MC33996 mc(CHIPSELECT,RESETPIN);



void callbackOutputfault(uint8_t fault, uint16_t registry)
{
  //Debug
  // Open circuit detected = 0b10000000
  // Over voltage detected = 0b01000000
  Serial.print("Fault type= ");
  Serial.print(fault, BIN);
  // each bit indicates which output had a fault
  // all output fault open = 0b1111111111111111
  Serial.print(" OUTPUTS= ");
  Serial.println(registry,BIN);
  //do something
}


void setup() {
  mc.begin();
  mc.enableContinutyDetection();
  mc.setFaultReport(callbackOutputfault);
  Serial.begin(115200);
  // put your setup code here, to run once:

}

void loop() {
  mc.continutyDetection();
  delay(2000);

}
