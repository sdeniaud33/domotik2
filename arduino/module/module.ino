#include "RS485Connector.h"
#include "ButtonHandler.h"
#include "LightHandler.h"
#include "TempSensor.h"
#include "Commons.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define MODULE_ID 1

#if MODULE_ID == 0
static const byte buttonDeviceIds[] = {1, 2, 3};
static const byte lightDeviceIds[] = {4, 5};
static const byte tempSensorIds[] = {74};
#elif MODULE_ID == 1
static const byte buttonDeviceIds[] = {7, 8, 9};
static const byte lightDeviceIds[] = {6};
static const byte tempSensorIds[] = {75};
#endif

static const byte buttonsCount = sizeof(buttonDeviceIds) / sizeof(byte);
static const byte lightsCount = sizeof(lightDeviceIds) / sizeof(byte);
static const byte tempSensorsCount = sizeof(tempSensorIds) / sizeof(byte);

LightHandler *lights[lightsCount];
ButtonHandler *buttons[buttonsCount];
TempSensor *tempSensors[tempSensorsCount];

void messageCallback(ModuleMessage message);

/*-----( Declare objects )-----*/
RS485Connector _connector(MODULE_ID);

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void _declareDevices()
{
  byte buttonPins[] = {2, 5, 6, 7, 8, 9};
  // Declare buttons
  byte buttonPinIdxOffset = (tempSensorsCount > 0 ? 1 : 0);

  byte lightPins[] = {A0, 4, 3};
  // Declare lights

  for (byte i = 0; i < lightsCount; i++)
  {
    lights[i] = new LightHandler(_connector, lightDeviceIds[i], lightPins[i]);
  }

  // Declare temperature sensors
  for (byte i = 0; i < tempSensorsCount; i++)
  {
    tempSensors[i] = new TempSensor(_connector, tempSensorIds[i]);
  }

  for (byte i = 0; i < buttonsCount; i++)
  {
    byte buttonId = buttonDeviceIds[i];
    buttons[i] = new ButtonHandler(_connector, buttonId, buttonPins[i + buttonPinIdxOffset]);

    // Connect the button to its light (if any)
    byte lightId = btnToLight[buttonId];
    if (lightId == 0)
    {
      // The button is not bound to any light (maybe a button for shutters ...)
    }
    else
    {
      LightHandler *light = _getLight(lightId);
      if (light == NULL)
      {
        // The light is not connected to this board
      }
      else
      {
        // The light is connected to the board : toggle it
        buttons[i]->setLight(light);
#if DEBUGMSG
        Serial.print(F("Bind button to light #"));
        Serial.println(light->getLightId());
#endif
      }
    }
  }
}

// A RS-485 message has been received
void messageCallback(ModuleMessage message)
{
  if (message.deviceClass == DeviceClass_System)
  {
    switch (message.payload.systemMessage)
    {
    case SystemMessage_Started:
#if DEBUGMSG
      Serial.print(F("Module #"));
      Serial.print(message.moduleId);
      Serial.println(F(" started"));
#endif
      break;
    }
  }
  else if (message.deviceClass == DeviceClass_TemperatureSensor)
  {
#if DEBUGMSG
    Serial.print(F("Received temperature from module #"));
    Serial.print(message.moduleId);
    Serial.print(" : ");
    Serial.println(message.payload.temperatureInCelsius);
#endif
  }
  else if (message.deviceClass == DeviceClass_Button)
  {
    switch (message.payload.buttonMessage)
    {
    case ButtonMessage_SingleClick:
      byte buttonId = message.deviceId;
#if DEBUGMSG
      Serial.print(F("Button "));
      Serial.print(buttonId);
      Serial.println(F(" : single"));
#endif

      byte lightId = btnToLight[buttonId];
      //        Serial.print(F("LightId = "));
      //        Serial.println(lightId);
      if (lightId == 0)
      {
        //          Serial.println(F("no light"));
        // The button is not bound to any light (maybe a button for shutters ...)
      }
      else
      {
        LightHandler *light = _getLight(lightId);
        if (light == NULL)
        {
          // The light is not connected to this board
          //            Serial.println(F("light not on board"));
        }
        else
        {
          // The light is connected to the board : toggle it
          //            Serial.println(F("light is on board"));
          light->toggle();
        }
      }
      break;
    }
  }
  else if (message.deviceClass == DeviceClass_Light)
  {
    switch (message.payload.lightMessage)
    {
    case LightMessage_SwitchOff:
    {
#if DEBUGMSG
      Serial.println(F("Light off"));
#endif
      LightHandler *light = _getLight(message.deviceId);
      if (light != NULL)
        light->off();
      break;
    }
    case LightMessage_SwitchOn:
    {
#if DEBUGMSG
      Serial.println(F("Light on"));
#endif
      LightHandler *light = _getLight(message.deviceId);
      if (light != NULL)
        light->on();
      break;
    }
    case LightMessage_ToggleStatus:
    {
#if DEBUGMSG
      Serial.println(F("  light toggle"));
#endif
      LightHandler *light = _getLight(message.deviceId);
      if (light != NULL)
        light->toggle();
      break;
    }
    case LightMessage_StatusIsOn:
    { // Light status : on
#if DEBUGMSG
      Serial.print(F("Status : light "));
      Serial.print(message.deviceId);
      Serial.println(F(" is on"));
#endif
      break;
    }
    case LightMessage_StatusIsOff:
    { // Light status : off
#if DEBUGMSG
      Serial.print(F("Status : light "));
      Serial.print(message.deviceId);
      Serial.println(F(" is off"));
#endif
      break;
    }
    }
  }
}

// Returns a lightHandler from an id
LightHandler *_getLight(byte lightId)
{
  for (byte i = 0; i < lightsCount; i++)
  {
    if (lights[i]->getLightId() == lightId)
    {
      return lights[i];
    }
  }
  return NULL;
}

void setup() /****** SETUP: RUNS ONCE ******/
{

  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);

#if DEBUGMSG
  Serial.print(F("********************** MODULE "));
  Serial.print(MODULE_ID);
  Serial.println(F(" **********************"));
#endif

  _declareDevices();

  _connector.init(messageCallback);
  _connector.sendSystemMessage(SystemMessage_Started);

#if DEBUGMSG
  Serial.println(F("Ready"));
#endif

  Serial.println(freeRam());

} //--(end setup )---

void loop()
{
  for (byte i = 0; i < buttonsCount; i++)
  {
    buttons[i]->loop();
  }
  for (byte i = 0; i < lightsCount; i++)
  {
    lights[i]->loop();
  }
  for (byte i = 0; i < tempSensorsCount; i++)
  {
    tempSensors[i]->loop();
  }
  _connector.loop();

} //--(end main loop )---
