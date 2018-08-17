#include <MC33996.h>

#define CHIPSELECT (10) //chip select pin.
#define RESETPIN   (9) //Chip reset pin.

MC33996 mc(CHIPSELECT,RESETPIN);

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
      mc.digitalWrite(i, HIGH);
      delay(200);
      mc.digitalWrite(i, LOW);
    }
   delay(1000);


}
