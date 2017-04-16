#ifndef LightHandler_h
#define LightHandler_h

#include "Arduino.h"

// ----- Callback function types -----

class LightHandler
{
  public:
    // ----- Constructor -----
    LightHandler(int deviceId) {
      _deviceId = deviceId;
    }

    void init(int pin) {
      pinMode(pin, OUTPUT);
      _pin = pin;
    }
    
    int getDeviceId() {
      return _deviceId;
    }
    
    bool isOn() {
      return _isOn;
    }
    
    void toggle() {
      if (_isOn)
        off();
      else
        on();
      Serial.print("Toggle light(");
      Serial.print(_deviceId);
      Serial.print(") -> ");
      if (_isOn)
        Serial.println("on");
      else
        Serial.println("off");
    }
    
    void on() {
      _isOn = true;
      digitalWrite(_pin, HIGH);
    }
    
    void off() {
      _isOn = false;
      digitalWrite(_pin, LOW);
    }
    
    void loop() {}

  private:
    int _pin;        // hardware pin number.
    int _deviceId;
    bool _isOn = false;
};

#endif


