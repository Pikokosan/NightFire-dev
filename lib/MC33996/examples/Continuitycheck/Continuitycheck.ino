#include <MC33996.h>

#define CHIPSELECT (10) //chip select pin.

MC33996 mc(CHIPSELECT);


void callbackOutputfault(uint8_t fault, uint16_t registry)
{
  //Debug
  Serial.print("Fault type= ");
  Serial.print(fault, BIN);
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
