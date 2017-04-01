#include "module_message2.h"
#include "ButtonHandler.h"
#include "LightHandler.h"
#include "buttonsToLights.h"

const int buttonsCount = 1;
ButtonHandler* buttons[] = {
  new ButtonHandler(108, 9),
};

const int lightCount = 1;
LightHandler* lights[] = {
  new LightHandler(50, 10),
};



/*-----( Declare Constants and Pin Numbers )-----*/
#define SSerialRX        11  //Serial Receive pin
#define SSerialTX        12  //Serial Transmit pin

#define SSerialTxControl 4   //RS485 Direction control

#define MODULE_ID 0
#define RS485_SPEED 57600
#define SERIAL_SPEED 9600

void messageCallback(ModuleMessage message);

/*-----( Declare objects )-----*/
RS485Connector _connector(MODULE_ID, SSerialRX, SSerialTX, SSerialTxControl);

void messageCallback(ModuleMessage message) {
  Serial.print("Received message ");
  Serial.println(message.text);
  if (strcmp(message.text, "btn_s")) {
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

void singleClick(int buttonId) {
  tryToSwitchLightFromButton(buttonId);
  _connector.enableTransmit();
  _connector.sendMessage(buttonId, "btn_s");
  _connector.enableReceive();
}

void doubleClick(int buttonId) {
  _connector.enableTransmit();
  _connector.sendMessage(buttonId, "bt_d");
  _connector.enableReceive();
}

void longPress(int buttonId) {
  _connector.enableTransmit();
  _connector.sendMessage(buttonId, "bt_l");
  _connector.enableReceive();
}

void tryToSwitchLightFromButton(int buttonId) {
  if (btnToLight[buttonId] == 0)
    return;
  LightHandler* light = _getLight(btnToLight[buttonId]);
  if (light == NULL)
    return;
  // The light bound to this button is connected to the board
  light->toggle();
}

LightHandler* _getLight(int lightId) {
  for (int i = 0; i < lightCount; i++) {
    if (lights[i]->getDeviceId() == lightId)
      return lights[i];
  }
  return NULL;
}

void setup()   /****** SETUP: RUNS ONCE ******/
{

  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(SERIAL_SPEED);

  // link the click functions to buttons.
  for (byte i = 0; i < buttonsCount; i++) {
    buttons[i]->setClickTicks(300);
    buttons[i]->setPressTicks(3000);
    buttons[i]->attachDoubleClick(doubleClick);
    buttons[i]->attachClick(singleClick);
    buttons[i]->attachLongPressStart(longPress);
  }

  // Start the software serial port, to another device
  _connector.init(RS485_SPEED);
  _connector.setMessageCallback(messageCallback);
  _connector.enableReceive();
}//--(end setup )---

void loop()
{
  for (byte i = 0; i < buttonsCount; i++) {
    buttons[i]->tick();
  }
  _connector.loop();
  delay(10);

}//--(end main loop )---


