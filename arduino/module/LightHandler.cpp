#include "LightHandler.h"

// ----- Initialization and Default Values -----

LightHandler::LightHandler(int deviceId)
{
  _deviceId = deviceId;
} // LightHandler

void LightHandler::init(int pin)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
}


int LightHandler::getDeviceId() {
  return _deviceId;
}

bool LightHandler::isOn() {
  return _isOn;
}

void LightHandler::on() {
  _isOn = true;
  digitalWrite(_pin, HIGH);
}

void LightHandler::off() {
  _isOn = false;
  digitalWrite(_pin, LOW);
}

void LightHandler::toggle() {
  Serial.println("Toggle");
  if (_isOn)
    off();
  else
    on();
}

void LightHandler::loop() {
}

// end.

