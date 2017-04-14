#ifndef TempSensor_h
#define TempSensor_h

#include "Arduino.h"

// ----- Callback function types -----

class TempSensor
{
public:
  // ----- Constructor -----
  TempSensor(int sensorId) {
    _sensorId = sensorId;
  }

  void loop() {}
  float getTemperature() { return 0;}
  int getSensorId() { return _sensorId; }

private:
  int _sensorId; 
};

#endif


