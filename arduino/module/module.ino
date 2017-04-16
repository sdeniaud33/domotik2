#include "module_message2.h"
#include "ButtonHandler.h"
#include "LightHandler.h"
#include "TempSensor.h"
#include "buttonsToLights.h"


/*-----( Declare Constants and Pin Numbers )-----*/
#define SSerialRX        A1  //Serial Receive pin
#define SSerialTX        A3  //Serial Transmit pin

#define SSerialTxControl A2   //RS485 Direction control
#define MODULE_ID 0

// LightHandler* lights[] = { new LightHandler(xxxx), new LightHandler(xxx)};
// ButtonHandler* buttons[] = { new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), };

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

int buttonPins[] = {2, 5, 6, 7, 8, 9};
int lightPins[] = {A0, 4, 3};

void _declareDevices() {
  // Declare buttons
  int buttonPinIdxOffset = (tempSensorsCount > 0 ? 1 : 0);
  for (int i = 0; i < buttonsCount; i++) {
    Serial.print("Declare button ");
    Serial.println(i);
    Serial.print("    device : ");
    Serial.println(buttonDeviceIds[i]);
    Serial.print("    pin    : ");
    Serial.println(buttonPins[i]);
    buttons[i] = new ButtonHandler(buttonDeviceIds[i]);
    buttons[i]->init(buttonPins[i + buttonPinIdxOffset]);
    buttons[i]->attachDoubleClick(doubleClick);
    buttons[i]->attachClick(singleClick);
    buttons[i]->attachLongPressStart(longPress);
  }
  // Declare lights
  for (int i = 0; i < lightsCount; i++) {
    Serial.print("Declare light ");
    Serial.println(i);
    Serial.print("    device : ");
    Serial.println(lightDeviceIds[i]);
    Serial.print("    pin    : ");
    Serial.println(lightPins[i]);
    lights[i] = new LightHandler(lightDeviceIds[i]);
    lights[i]->init(lightPins[i]);
    for (int j = 0; j < 2; j++) {
      lights[i]->on();
      delay(50);
      lights[i]->off();
      delay(200);
    }
  }
  // Declare temperature sensors
  for (int i = 0; i < tempSensorsCount; i++) {
    Serial.print("Declare temperature sensor ");
    Serial.println(i);
    Serial.print("    device : ");
    Serial.println(tempSensorIds[i]);
    tempSensors[i] = new TempSensor(tempSensorIds[i]);
  }
}


void messageCallback(ModuleMessage message);

/*-----( Declare objects )-----*/
RS485Connector _connector(MODULE_ID, SSerialRX, SSerialTX, SSerialTxControl);

// A RS-485 message has been received
void messageCallback(ModuleMessage message) {
  if (strcmp(message.text, "b_s") == 0) {
    Serial.print("Button ");
    Serial.print(message.deviceId);
    Serial.println(" : single");
    tryToSwitchLightFromButton(message.deviceId);
  }
  else if (strcmp(message.text, "l_0") == 0) {
    Serial.println("Light off");
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->off();
  }
  else if (strcmp(message.text, "l_1") == 0) {
    Serial.println("Light on");
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->on();
  }
  else if (strcmp(message.text, "l_t") == 0) {
    Serial.println("  light tog");
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->toggle();
  }
  else if (strcmp(message.text, "l_s1") == 0) {
    // Light status : on
    Serial.print("Status : light ");
    Serial.print(message.deviceId);
    Serial.println(" is on");
  }
  else if (strcmp(message.text, "l_s0") == 0) {
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
  _connector.sendMessage(buttonId, "b_s");
}

// A button has been double clicked
void doubleClick(int buttonId) {
  _connector.sendMessage(buttonId, "b_d");
}

// A button has been long pressed
void longPress(int buttonId) {
  _connector.sendMessage(buttonId, "b_l");
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
    _connector.sendMessage(lightId, "l_s1");
  else
    _connector.sendMessage(lightId, "l_s0");

}

// Returns a lightHandler from an id
LightHandler* _getLight(int lightId) {
//  Serial.print("Looking for light # ");
//  Serial.println(lightId);
  for (int i = 0; i < lightsCount; i++) {
//    Serial.print("    testing");
//    Serial.println(lights[i]->getDeviceId());
    if (lights[i]->getDeviceId() == lightId) {
//      Serial.println("Found");
      return lights[i];
    }
  }
//  Serial.println("Not found");
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

  // Start the software serial port, to another device
  _connector.init(19200);
  _connector.setMessageCallback(messageCallback);
  _connector.enableReceive();

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
  _connector.loop();
  delay(10);

}//--(end main loop )---


