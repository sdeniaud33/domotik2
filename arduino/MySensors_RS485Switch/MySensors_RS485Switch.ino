/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   DESCRIPTION

   Simple binary switch example
   Connect button or door/window reed switch between
   digitial I/O pin 3 (BUTTON_PIN below) and GND.
   http://www.mysensors.org/build/binary
*/


// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RS485
#define MY_RS485_DE_PIN 2
#define MY_RS485_BAUD_RATE 9600
#define MY_NODE_ID 2
#define PIN_BTN 4
#include <MySensors.h>
#include <OneButton.h>

int rollerPos = 50;
int rollerStep = 0;
float temp = 20;
bool incTemp;
bool lastRollerDirectionWasUp = true;

OneButton btn(PIN_BTN, false);

#define CHILD_ID 3

// Change to V_LIGHT if you use S_LIGHT in presentation below
MyMessage msgRollerPos(3, V_PERCENTAGE);
MyMessage msgTemp(4, V_TEMP);
MyMessage msgButton(5, V_TRIPPED);
MyMessage msgLightStatus(6, V_STATUS);

// MyMessage msgUp(CHILD_ID, V_UP);
// MyMessage msgDown(CHILD_ID, V_DOWN);

void setup()
{
  pinMode(13, OUTPUT);
  sendBatteryLevel(73);
  btn.attachClick(onClickBtn);
  send(msgLightStatus.set(digitalRead(13) == HIGH ? 1 : 0));
}

void presentation() {
  // Register binary input sensor to gw (they will be created as child devices)
  // You can use S_DOOR, S_MOTION or S_LIGHT here depending on your usage.
  // If S_LIGHT is used, remember to update variable type you send in. See "msg" above.
  sendSketchInfo("My rollers", "1.8");
  //  present(CHILD_ID, S_DOOR);
  present(3, S_COVER);
  present(4, S_TEMP);
  present(5, S_DOOR);
  present(6, S_BINARY);
}


void receive(const MyMessage &message) {
  Serial.print("Received message !!! : ");
  Serial.print(message.sensor);
  Serial.print(" # ");
  Serial.print(message.type);
  Serial.print(" = ");
  Serial.print(message.getByte());
  Serial.println();
  if (message.sensor == 3) {
    // Volet roulant
    // t = message.getByte();
    if (message.type == V_UP) {
      Serial.println("UP");
      lastRollerDirectionWasUp = true;
      rollerStep = 10;
    }
    else if (message.type == V_DOWN) {
      Serial.println("DOWN");
      lastRollerDirectionWasUp = false;
      rollerStep = -10;
    }
    else if (message.type == V_STOP) {
      Serial.println("STOP");
      rollerStep = 0;
    }
    else if (message.type == V_VAR1) {
      Serial.println("PUSH BUTTON");
      if (rollerStep == 0) {
        if (lastRollerDirectionWasUp)
          rollerStep = -10;
        else
          rollerStep = 10;
        lastRollerDirectionWasUp = !lastRollerDirectionWasUp;
      }
      else
        rollerStep = 0;

    }
  }
  else if (message.sensor == 6) {
    // Light
    if (message.type == V_STATUS) {
      if (message.getByte() == 2) {
        // switch
        Serial.println(">>>>>> SWITCH LIGHT");
        bool lightWasOn = (digitalRead(13) == HIGH);
        digitalWrite(13, lightWasOn ? LOW : HIGH);
        send(msgLightStatus.set(digitalRead(13) == HIGH ? 1 : 0));
      }
      else if (message.getByte() == 1) {
        // light on
        digitalWrite(13, HIGH);
        send(msgLightStatus.set(digitalRead(13) == HIGH ? 1 : 0));
      }
      else if (message.getByte() == 0) {
        // light off
        digitalWrite(13, LOW);
        send(msgLightStatus.set(digitalRead(13) == HIGH ? 1 : 0));
      }
    }
  }
}

void onClickBtn()
{
  send(msgButton.set(1));
  Serial.println("CLICK");
}

//  Check if digital input has changed and send in new value
int waitIdx = 0;
void loop()
{
  btn.tick();
  waitIdx++;
  if (waitIdx < 2000) {
    wait(1);
    return;
  }
  waitIdx = 0;
  if (rollerStep != 0) {
    rollerPos += rollerStep;
    if (rollerPos > 100) {
      rollerStep = 0;
      rollerPos = 100;
    }
    else if (rollerPos < 0) {
      rollerStep = 0;
      rollerPos = 0;
    }
    send(msgRollerPos.set(rollerPos));
  }
  temp += (incTemp ? 0.5 : -0.5);
  if (temp > 30) {
    incTemp = !incTemp;
    temp = 29;
  }
  else if (temp < -10) {
    incTemp = !incTemp;
    temp = -9;
  }
  Serial.print(temp, 2);
  Serial.print("/");
  Serial.print(rollerPos);
  Serial.println();
  send(msgTemp.set(temp, 2));
  send(msgRollerPos.set(rollerPos));
}
