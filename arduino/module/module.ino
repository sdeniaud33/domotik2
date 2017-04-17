#include "RS485Connector.h"
#include "ButtonHandler.h"
#include "LightHandler.h"
#include "TempSensor.h"
#include "buttonsToLights.h"

/*-----( Declare Constants and Pin Numbers )-----*/
#define MODULE_ID 0

#define RS845_MSG_BUTTON_SINGLE_CLICK "b_s"
#define RS845_MSG_BUTTON_DOUBLE_CLICK "b_d"
#define RS845_MSG_BUTTON_LONG_CLICK "b_l"

#define RS845_MSG_LIGHT_STATUS_ON "l_s1"
#define RS845_MSG_LIGHT_STATUS_OFF "l_s0"
#define RS845_MSG_LIGHT_ACTION_TOGGLE "l_t"
#define RS845_MSG_LIGHT_ACTION_ON "l_0"
#define RS845_MSG_LIGHT_ACTION_OFF "l_1"


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

void _declareDevices() {
  int buttonPins[] = {2, 5, 6, 7, 8, 9};
  // Declare buttons
  int buttonPinIdxOffset = (tempSensorsCount > 0 ? 1 : 0);
  for (int i = 0; i < buttonsCount; i++) {
    Serial.print("Declare button ");
    Serial.println(i);
    Serial.print("    device : ");
    Serial.println(buttonDeviceIds[i]);
    Serial.print("    pin    : ");
    Serial.println(buttonPins[i]);
    buttons[i] = new ButtonHandler(buttonDeviceIds[i], buttonPins[i + buttonPinIdxOffset]);
    buttons[i]->attachDoubleClick(doubleClick);
    buttons[i]->attachClick(singleClick);
    buttons[i]->attachLongPressStart(longPress);
  }

  int lightPins[] = {A0, 4, 3};
  // Declare lights
  for (int i = 0; i < lightsCount; i++) {
    Serial.print("Declare light ");
    Serial.println(i);
    Serial.print("    device : ");
    Serial.println(lightDeviceIds[i]);
    Serial.print("    pin    : ");
    Serial.println(lightPins[i]);
    lights[i] = new LightHandler(lightDeviceIds[i], lightPins[i]);
  }
  // Declare temperature sensors
  for (int i = 0; i < tempSensorsCount; i++) {
    Serial.print("Declare temperature sensor ");
    Serial.println(i);
    Serial.print("    device : ");
    Serial.println(tempSensorIds[i]);
    tempSensors[i] = new TempSensor(tempSensorIds[i], temperatureAvailableCallback);
  }
}

void messageCallback(ModuleMessage message);

/*-----( Declare objects )-----*/
RS485Connector _connector(MODULE_ID);

// A RS-485 message has been received
void messageCallback(ModuleMessage message) {
  if (strcmp(message.text, RS845_MSG_BUTTON_SINGLE_CLICK) == 0) {
    Serial.print("Button ");
    Serial.print(message.deviceId);
    Serial.println(" : single");
    tryToSwitchLightFromButton(message.deviceId);
  }
  else if (strcmp(message.text, RS845_MSG_LIGHT_ACTION_OFF) == 0) {
    Serial.println("Light off");
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->off();
  }
  else if (strcmp(message.text, RS845_MSG_LIGHT_ACTION_ON) == 0) {
    Serial.println("Light on");
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->on();
  }
  else if (strcmp(message.text, RS845_MSG_LIGHT_ACTION_TOGGLE) == 0) {
    Serial.println("  light tog");
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->toggle();
  }
  else if (strcmp(message.text, RS845_MSG_LIGHT_STATUS_ON) == 0) {
    // Light status : on
    Serial.print("Status : light ");
    Serial.print(message.deviceId);
    Serial.println(" is on");
  }
  else if (strcmp(message.text, RS845_MSG_LIGHT_STATUS_OFF) == 0) {
    // Light status : off
    Serial.print("Status : light ");
    Serial.print(message.deviceId);
    Serial.println(" is off");
  }
  else {
    Serial.print("Received message ");
    Serial.print(message.text);
    Serial.print(", device = ");
    Serial.println(message.deviceId);
  }
}

// A button has been single clicked
void singleClick(int buttonId) {
  Serial.print("Button clicked ");
  Serial.println(buttonId);
  int lightId = btnToLight[buttonId];
  tryToSwitchLightFromButton(buttonId);
  _connector.sendMessage(buttonId, Button, RS845_MSG_BUTTON_SINGLE_CLICK);
}

// A button has been double clicked
void doubleClick(int buttonId) {
  _connector.sendMessage(buttonId, Button, RS845_MSG_BUTTON_DOUBLE_CLICK);
}

// A button has been long pressed
void longPress(int buttonId) {
  _connector.sendMessage(buttonId, Button, RS845_MSG_BUTTON_LONG_CLICK);
}

void temperatureAvailableCallback(int sensorId, float temperatureInCelsius) {
  Serial.print(">>> Temperature(");
  Serial.print(sensorId);
  Serial.print(") = ");
  Serial.print(temperatureInCelsius);
  Serial.println(" C");

  // TODO : Envoyer un msg RS485 avec la temperature.
}

// Tries to switch the light bound to a button
void tryToSwitchLightFromButton(int buttonId) {
  int lightId = btnToLight[buttonId];
  if (lightId == 0) {
    // The button is not bound to any light (strange ...)
    return;
  }
  LightHandler* light = _getLight(lightId);
  if (light == NULL) {
    // The light is not bound to this board
    return;
  }
  // The light bound to this button is connected to the board
  light->toggle();
  if (light->isOn())
    _connector.sendMessage(lightId, Light, RS845_MSG_LIGHT_STATUS_ON);
  else
    _connector.sendMessage(lightId, Light, RS845_MSG_LIGHT_STATUS_OFF);

}

// Returns a lightHandler from an id
LightHandler* _getLight(int lightId) {
  for (int i = 0; i < lightsCount; i++) {
    if (lights[i]->getDeviceId() == lightId) {
      return lights[i];
    }
  }
  return NULL;
}

void setup()   /****** SETUP: RUNS ONCE ******/
{

  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(57600);

  Serial.print("********************** MODULE ");
  Serial.print(MODULE_ID);
  Serial.println(" **********************");

  _declareDevices();

  _connector.init(messageCallback);

  Serial.println("Ready");

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
  delay(10);

}//--(end main loop )---


