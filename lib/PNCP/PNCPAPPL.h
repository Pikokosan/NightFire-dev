
#ifndef PNCPAPPL_h
#define PNCPAPPL_h
#include "Arduino.h"


class PNCP;

class PNCPAPPL{
//input callbacks
public:
  PNCPAPPL(PNCP& DLL);
  void begin(long baud, size_t port = 0,uint8_t RE_pin_def = 2, uint8_t DE_pin_def = 3);
  void update();

  void setHandleSinglecue(void (*fptr)(uint8_t cue));
  void setHandleReport(void(*fptr)(void));
  void setHandleChargecues(void (*fptr)(void));
  void setHandleSetPulse(void (*fptr)(uint8_t cue));
  void setHandleCueContinuity(void (*fptr)(void));
  void setHandleGetVoltage(byte (*fptr)(void));



private:
  PNCP& DLL;
  void launchCallback();
  void (*mSinglecueCallback)(uint8_t cue);
  void (*mSetPulseCallback)(uint8_t pulse);
  void (*mReportCallback)(void);
  void (*mChargecuesCallback)(void);
  void (*mCueContinuity)(void);
  void (*mGetVoltageCallback)(void);




};






#endif
