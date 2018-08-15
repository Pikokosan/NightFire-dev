#include <MC33996.h>

#define CHIPSELECT (10) //chip select pin.

MC33996 mc(CHIPSELECT);

void setup() {
  mc.begin();

}

void loop() {
   for(int i=0; i<=16; i++)
   {
      mc.digitalWrite(i, HIGH);
      delay(200);
    }
    for(int i=0; i<=16; i++)
    {
      mc.digitalWrite(i, ON);
      delay(200);
      mc.digitalWrite(i, OFF);
    }
   delay(1000);


}
