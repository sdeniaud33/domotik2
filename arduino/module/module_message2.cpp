#include "module_message2.h"


/*
   int (RS485Connector::*pt2Read)() const = NULL;
  pt2Read = &RS485Connector::fRead;
  //  _myChannel(RS485(fRead, fAvailable, fWriteBytes, 50)) {

*/
RS485Connector::RS485Connector(int moduleId, int rxPin, int txPin, int enPin):
  _moduleId(moduleId),
  _rxPin(rxPin),
  _txPin(txPin),
  _enPin(enPin),
  _softwareSerial(new SoftwareSerial(rxPin, txPin)),
  _myChannel(this, this, this, 50)
{
}

void RS485Connector::init(int serialSpeed) {
  pinMode(_enPin, OUTPUT);
  _softwareSerial->begin(57600);   // set the data rate
  digitalWrite(_enPin, RS485Transmit);  // Enable RS485 Transmit
  _myChannel.begin();
  this->enableTransmit();
  this->sendMessage(0, "ready");
  this->enableReceive();
}

void RS485Connector::enableTransmit() {
  digitalWrite(_enPin, RS485Transmit);  // Enable RS485 Transmit
}

void RS485Connector::enableReceive() {
  digitalWrite(_enPin, RS485Receive);  // Enable RS485 Receive
}

void RS485Connector::sendMsg (const byte * data, const byte length) {
  _myChannel.sendMsg(data, length);
}

int RS485Connector::sendMessage(int deviceId, const char msg[]) {

  ModuleMessage message;

  memset (&message, 0, sizeof message);
  message.moduleId = _moduleId;
  message.counter = this->_globalCounter++;
  message.deviceId = deviceId;
  memcpy(message.text, msg, strlen(msg));
  message.crc = this->ComputeMessageCrc2(&message, sizeof message);
  Serial.print("Sent ");
  Serial.print(message.text);
  Serial.print("/");
  Serial.print(message.deviceId);
  Serial.print(" (");
  Serial.print(message.counter);
  Serial.println(")");
  for (int i = 0; i < SEND_REPEAT_COUNT; i++) {
    if (i > 0)
      delay(SEND_REPEAT_INTERVAL);
    this->sendMsg((byte *) &message, sizeof message);
  }
}

void RS485Connector::loop() {
  if (_myChannel.update()) {
    if (_messageCallback != NULL) {
      ModuleMessage message;
      memset (&message, 0, sizeof message);
      memcpy(&message, _myChannel.getData(), _myChannel.getLength());

      int expectedCrc = this->ComputeMessageCrc2(&message, sizeof message);
      if (expectedCrc != message.crc) {
        Serial.println("Message received with invalid CRC");
        return;
      }
      /*
            Serial.print("Received message #");
            Serial.print(message.counter);
            Serial.print(" from device #");
            Serial.print(message.moduleId);
      */
      _messageCallback(message);
    }
  }
}

void RS485Connector::setMessageCallback(MessageCallback messageCallback) {
  _messageCallback = messageCallback;
}




