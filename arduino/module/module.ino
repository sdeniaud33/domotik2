#include "RS485Connector.h"
#include "ButtonHandler.h"
#include "LightHandler.h"
#include "TempSensor.h"
#include "buttonsToLights.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define MODULE_ID 0

#if MODULE_ID == 0
static const int buttonDeviceIds[] = {1, 2, 3};
static const int lightDeviceIds[] = {4, 5};
static const int tempSensorIds[] = {74};
#elif MODULE_ID == 1
static const int buttonDeviceIds[] = {7, 8, 9};
static const int lightDeviceIds[] = {6};
static const int tempSensorIds[] = {75};
#endif

static const int buttonsCount = sizeof(buttonDeviceIds) / sizeof(int);
static const int lightsCount = sizeof(lightDeviceIds) / sizeof(int);
static const int tempSensorsCount = sizeof(tempSensorIds) / sizeof(int);

LightHandler* lights[lightsCount];
ButtonHandler* buttons[buttonsCount];
TempSensor* tempSensors[tempSensorsCount];

void messageCallback(ModuleMessage message);

/*-----( Declare objects )-----*/
RS485Connector _connector(MODULE_ID);


int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void _declareDevices() {
  int buttonPins[] = {2, 5, 6, 7, 8, 9};
  // Declare buttons
  int buttonPinIdxOffset = (tempSensorsCount > 0 ? 1 : 0);

  int lightPins[] = {A0, 4, 3};
  // Declare lights

  for (int i = 0; i < lightsCount; i++) {
    lights[i] = new LightHandler(_connector, lightDeviceIds[i], lightPins[i]);
  }
  
  // Declare temperature sensors
  for (int i = 0; i < tempSensorsCount; i++) {
    tempSensors[i] = new TempSensor(_connector, tempSensorIds[i]);
  }

  for (int i = 0; i < buttonsCount; i++) {
    int buttonId = buttonDeviceIds[i];
    buttons[i] = new ButtonHandler(_connector, buttonId, buttonPins[i + buttonPinIdxOffset]);
    
    // Connect the button to its light (if any)
    int lightId = btnToLight[buttonId];
    if (lightId == 0) {
      // The button is not bound to any light (maybe a button for shutters ...)
    }
    else {
      LightHandler* light = _getLight(lightId);
      if (light == NULL) {
        // The light is not connected to this board
      }
      else {
        // The light is connected to the board : toggle it
        buttons[i]->setLight(light);
        Serial.print(F("Bind button to light #"));
        Serial.println(light->getLightId());
      }
    }
    
  }
}

// A RS-485 message has been received
void messageCallback(ModuleMessage message) {
  if (message.deviceClass == DeviceClass_System) {
    switch (message.payload.systemMessage) {
      case SystemMessage_Started:
        Serial.print(F("Module #"));
        Serial.print(message.moduleId);
        Serial.println(F(" started"));
        break;
    }
  } else if (message.deviceClass == DeviceClass_TemperatureSensor) {
    Serial.print(F("Received temperature from module #"));
    Serial.print(message.moduleId);
    Serial.print(" : ");
    Serial.println(message.payload.temperatureInCelsius);
  } else if (message.deviceClass == DeviceClass_Button) {
    switch (message.payload.buttonMessage) {
      case ButtonMessage_SingleClick:
        int buttonId = message.deviceId;
        Serial.print(F("Button "));
        Serial.print(buttonId);
        Serial.println(F(" : single"));

        int lightId = btnToLight[buttonId];
//        Serial.print(F("LightId = "));
//        Serial.println(lightId);
        if (lightId == 0) {
//          Serial.println(F("no light"));
          // The button is not bound to any light (maybe a button for shutters ...)
        }
        else {
          LightHandler* light = _getLight(lightId);
          if (light == NULL) {
            // The light is not connected to this board
//            Serial.println(F("light not on board"));
          }
          else {
            // The light is connected to the board : toggle it
//            Serial.println(F("light is on board"));
            light->toggle();
          }
        }
        break;
    }
  }
  else if (message.deviceClass == DeviceClass_Light) {
    switch (message.payload.lightMessage) {
      case LightMessage_SwitchOff:
        {
          Serial.println(F("Light off"));
          LightHandler* light = _getLight(message.deviceId);
          if (light != NULL)
            light->off();
          break;
        }
      case LightMessage_SwitchOn:
        {
          Serial.println(F("Light on"));
          LightHandler* light = _getLight(message.deviceId);
          if (light != NULL)
            light->on();
          break;
        }
      case LightMessage_ToggleStatus:
        {
          Serial.println(F("  light toggle"));
          LightHandler* light = _getLight(message.deviceId);
          if (light != NULL)
            light->toggle();
          break;
        }
      case LightMessage_StatusIsOn:
        { // Light status : on
          Serial.print(F("Status : light "));
          Serial.print(message.deviceId);
          Serial.println(F(" is on"));
          break;
        }
      case LightMessage_StatusIsOff:
        { // Light status : off
          Serial.print(F("Status : light "));
          Serial.print(message.deviceId);
          Serial.println(F(" is off"));
          break;
        }
    }
  }
}

// Returns a lightHandler from an id
LightHandler* _getLight(int lightId) {
  for (int i = 0; i < lightsCount; i++) {
    if (lights[i]->getLightId() == lightId) {
      return lights[i];
    }
  }
  return NULL;
}

void setup()   /****** SETUP: RUNS ONCE ******/
{

  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);

  Serial.print(F("********************** MODULE "));
  Serial.print(MODULE_ID);
  Serial.println(F(" **********************"));

  _declareDevices();

  _connector.init(messageCallback);
  _connector.sendSystemMessage(SystemMessage_Started);

  Serial.println(F("Ready"));

  Serial.println(freeRam());

}//--(end setup )---

void loop()
{
  for (byte i = 0; i < buttonsCount; i++) {
    buttons[i]->loop();
  }
  for (byte i = 0; i < lightsCount; i++) {
    lights[i]->loop();
  }
  for (byte i = 0; i < tempSensorsCount; i++) {
    tempSensors[i]->loop();
  }
  _connector.loop();

}//--(end main loop )---


