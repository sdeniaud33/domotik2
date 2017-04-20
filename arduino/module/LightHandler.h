#ifndef LightHandler_h
#define LightHandler_h

#include "RS485Connector.h"

#include "Arduino.h"

// ----- Callback function types -----

enum LightMessage
{
  LightMessage_StatusIsOn,
  LightMessage_StatusIsOff,
  LightMessage_SwitchOn,
  LightMessage_SwitchOff,
  LightMessage_ToggleStatus
};

class LightHandler
{
public:
  // ----- Constructor -----
  LightHandler(RS485Connector connector, byte lightId, byte pin) : _connector(connector)
  {
    _lightId = lightId;
    _pin = pin;
#if DEBUGMSG
    Serial.print("Declare light #");
    Serial.print(lightId);
    Serial.print(F(" on pin #"));
    Serial.println(pin);
#endif
    pinMode(pin, OUTPUT);
    // Make the light blink
    for (byte i = 0; i < 2; i++)
    {
      this->on(false);
      delay(50);
      this->off(false);
      delay(200);
    }
  }

  byte getLightId()
  {
    return _lightId;
  }

  bool isOn()
  {
    return _isOn;
  }

  void toggle(bool notifyRS845 = true)
  {
    if (_isOn)
      off(notifyRS845);
    else
      on(notifyRS845);
  }

  void on(bool notifyRS845 = true)
  {
    _isOn = true;
    digitalWrite(_pin, HIGH);
#if DEBUGMSG
    Serial.print(F("Light #"));
    Serial.print(_lightId);
    Serial.println(F(" : on"));
#endif
    if (notifyRS845)
      _sendMessage(LightMessage_StatusIsOn);
  }

  void off(bool notifyRS845 = true)
  {
    _isOn = false;
    digitalWrite(_pin, LOW);
#if DEBUGMSG
    Serial.print(F("Light #"));
    Serial.print(_lightId);
    Serial.println(F(" : off"));
#endif
    if (notifyRS845)
      _sendMessage(LightMessage_StatusIsOff);
  }

  void loop() {}

private:
  byte _pin; // hardware pin number.
  byte _lightId;
  bool _isOn = false;
  RS485Connector _connector;

  int _sendMessage(LightMessage lightMessage)
  {
    union MessagePayload payload;
    payload.lightMessage = lightMessage;
    return _connector.sendMessage(_lightId, DeviceClass_Light, payload);
  }
};

#endif
