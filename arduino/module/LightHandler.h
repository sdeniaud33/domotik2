#ifndef LightHandler_h
#define LightHandler_h

#include "Arduino.h"

// ----- Callback function types -----

class LightHandler
{
public:
  // ----- Constructor -----
  LightHandler(int deviceId);

  void init(int pin);
  int getDeviceId();
  bool isOn();
  void toggle();
  void on();
  void off();
  void loop();

private:
  int _pin;        // hardware pin number. 
  int _deviceId;
  bool _isOn = false;
};

#endif


