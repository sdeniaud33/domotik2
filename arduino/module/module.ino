#include "module_message2.h"
#include "ButtonHandler.h"
#include "LightHandler.h"
#include "TempSensor.h"
#include "buttonsToLights.h"

#define MODULE_ID 0

// LightHandler* lights[] = { new LightHandler(xxxx), new LightHandler(xxx)};
// ButtonHandler* buttons[] = { new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), };

#if MODULE_ID == 0
static const int buttonDeviceIds[] = {1, 2, 3};
static const int lightDeviceIds[] = {4, 5, 6};
static const int tempSensorIds[] = {74};
#elif MODULE_ID == 1
int lightDeviceIds[] = {6};
int buttonDeviceIds[] = {36, 37};
#elif MODULE_ID == 2
LightHandler* lights[] = { new LightHandler(9)};
ButtonHandler* buttons[] = { new ButtonHandler(34), new ButtonHandler(38)};
#elif MODULE_ID == 3
LightHandler* lights[] = { new LightHandler(4), new LightHandler(13)};
ButtonHandler* buttons[] = { new ButtonHandler(30), new ButtonHandler(45), new ButtonHandler(65), new ButtonHandler(72)};
#elif MODULE_ID == 4
LightHandler* lights[] = { new LightHandler(2), new LightHandler(14)};
ButtonHandler* buttons[] = { new ButtonHandler(28), new ButtonHandler(31), new ButtonHandler(32), new ButtonHandler(46), new ButtonHandler(66), new ButtonHandler(71), };
#elif MODULE_ID == 5
LightHandler* lights[] = { new LightHandler(xxxx), new LightHandler(xxx)};
ButtonHandler* buttons[] = { new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), };
#elif MODULE_ID == 6
LightHandler* lights[] = { new LightHandler(xxxx), new LightHandler(xxx)};
ButtonHandler* buttons[] = { new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), };
#elif MODULE_ID == 7
LightHandler* lights[] = { new LightHandler(xxxx), new LightHandler(xxx)};
ButtonHandler* buttons[] = { new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), new ButtonHandler(xxxx), };
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
  for(int i = 0; i < buttonsCount; i++) {
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
  for(int i = 0; i < lightsCount; i++) {
  Serial.print("Declare light ");
  Serial.println(i);
  Serial.print("    device : ");
  Serial.println(lightDeviceIds[i]);
  Serial.print("    pin    : ");
  Serial.println(lightPins[i]);
    lights[i] = new LightHandler(lightDeviceIds[i]); 
    lights[i]->init(lightPins[i]);
  }
  // Declare temperature sensors
  for(int i = 0; i < tempSensorsCount; i++) {
  Serial.print("Declare temperature sensor ");
  Serial.println(i);
  Serial.print("    device : ");
  Serial.println(tempSensorIds[i]);
    tempSensors[i] = new TempSensor(tempSensorIds[i]); 
  }
}


/*-----( Declare Constants and Pin Numbers )-----*/
#define SSerialRX        A3  //Serial Receive pin
#define SSerialTX        A1  //Serial Transmit pin

#define SSerialTxControl A2   //RS485 Direction control

void messageCallback(ModuleMessage message);

/*-----( Declare objects )-----*/
RS485Connector _connector(MODULE_ID, SSerialRX, SSerialTX, SSerialTxControl);

// A RS-485 message has been received
void messageCallback(ModuleMessage message) {
  Serial.print("Received message ");
  Serial.println(message.text);
  if (strcmp(message.text, "bt_s")) {
    tryToSwitchLightFromButton(message.deviceId);
  }
  else if (strcmp(message.text, "light_off")) {
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->off();
  }
  else if (strcmp(message.text, "light_on")) {
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->on();
  }
  else if (strcmp(message.text, "light_tog")) {
    LightHandler* light = _getLight(message.deviceId);
    if (light != NULL)
      light->toggle();
  }
}

// A button has been single clicked
void singleClick(int buttonId) {
  Serial.print("Button clicked ");
  Serial.println(buttonId);
  tryToSwitchLightFromButton(buttonId);
  _connector.sendMessage(buttonId, "bt_s");
}

// A button has been double clicked
void doubleClick(int buttonId) {
  _connector.sendMessage(buttonId, "bt_d");
}

// A button has been long pressed
void longPress(int buttonId) {
  _connector.sendMessage(buttonId, "bt_l");
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
    _connector.sendMessage(lightId, "lt_1");
  else
    _connector.sendMessage(lightId, "lt_0");
 
}

// Returns a lightHandler from an id
LightHandler* _getLight(int lightId) {
  Serial.print("Looking for light # ");
  Serial.println(lightId);
  for (int i = 0; i < lightsCount; i++) {
    Serial.print("    testing");
    Serial.println(lights[i]->getDeviceId());
    if (lights[i]->getDeviceId() == lightId) {
      Serial.println("Found");
      return lights[i];
    }
  }
  Serial.println("Not found");
  return NULL;
}

void setup()   /****** SETUP: RUNS ONCE ******/
{

  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);

  _declareDevices();
/*
  // Start the software serial port, to another device
  _connector.init(57600);
  _connector.setMessageCallback(messageCallback);
  _connector.enableReceive();
*/  
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


