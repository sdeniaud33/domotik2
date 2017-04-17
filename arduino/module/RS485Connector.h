#ifndef RS485Connector_h
#define RS485Connector_h
#include "LowLevelNonBlockingRS485Connector.h"
#include <SoftwareSerial.h>
#include "Arduino.h"


enum DeviceClass {
  System,
  Button,
  Light,
  TemperatureSensor
};

typedef struct
{
  int crc;
  int counter;
  int moduleId;
  int deviceId;
  DeviceClass deviceClass;
  char text[20];
}  ModuleMessage;

#define SEND_REPEAT_COUNT 3
#define SEND_REPEAT_INTERVAL 20
#define MAX_MODULE_COUNT 20

class RS485Connector : IReadByte, IWriteByte, IBytesAvailable {
    typedef void (*MessageCallback)  (const ModuleMessage message);

    const SoftwareSerial* _softwareSerial;
    const int _moduleId;
    const int _rxPin;
    const int _txPin;
    const int _enPin;
    const LowLevelNonBlockingRS485Connector _lowLevelConnector;
    int _globalCounter = 0;
    MessageCallback _messageCallback;
    int lastProcessedCounterByModule[MAX_MODULE_COUNT];


#define RS485Transmit    HIGH
#define RS485Receive     LOW

  private:
    // send data
    void sendMsg (const byte * data, const byte length) {
      _lowLevelConnector.sendMsg(data, length);
    }

    unsigned long ComputeMessageCrc2(void *message, int len)
    {
      unsigned char * data;
      unsigned long chk = 0;
      data = message;
      // Skip the crc
      data = data + sizeof(int);
      for (int i = sizeof(int); i < sizeof(ModuleMessage); i++) {
        chk += *data;
        data++;
      }
      return chk;
    }

    int readByte() {
      return _softwareSerial->read();
    }

    size_t writeByte(const int what) {
      return _softwareSerial->write(what);
    }

    int bytesAvailable() {
      return _softwareSerial->available();
    }


  public:
    RS485Connector::RS485Connector(int moduleId) : RS485Connector(moduleId, A1, A3, A2) {}

    RS485Connector::RS485Connector(int moduleId, int rxPin, int txPin, int enPin):
      _moduleId(moduleId),
      _rxPin(rxPin),
      _txPin(txPin),
      _enPin(enPin),
      _softwareSerial(new SoftwareSerial(rxPin, txPin)),
      _lowLevelConnector(this, this, this, 50)
    {
      for (int i = 0; i < MAX_MODULE_COUNT; i++)
        lastProcessedCounterByModule[i] = -1;
    }

    void init(MessageCallback messageCallback, int serialSpeed = 19200) {
      _messageCallback = messageCallback;
      pinMode(_enPin, OUTPUT);
      _softwareSerial->begin(serialSpeed);   // set the data rate
      digitalWrite(_enPin, RS485Transmit);  // Enable RS485 Transmit
      _lowLevelConnector.begin();
      this->sendMessage(0, System, "ready");
      this->enableReceive();
    }

    void enableTransmit() {
      digitalWrite(_enPin, RS485Transmit);  // Enable RS485 Transmit
    }
    void enableReceive() {
      digitalWrite(_enPin, RS485Receive);  // Enable RS485 Receive
    }

    int sendMessage(int deviceId, DeviceClass deviceClass, const char msg[]) {
      this->enableTransmit();
      ModuleMessage message;

      memset (&message, 0, sizeof message);
      message.moduleId = _moduleId;
      message.deviceClass = deviceClass;
      message.counter = this->_globalCounter++;
      message.deviceId = deviceId;
      memcpy(message.text, msg, strlen(msg));
      message.crc = this->ComputeMessageCrc2(&message, sizeof message);
      /*
        Serial.print("Sent ");
        Serial.print(message.text);
        Serial.print("/");
        Serial.print(message.deviceId);
        Serial.print(" (");
        Serial.print(message.counter);
        Serial.println(")");
      */
      for (int i = 0; i < SEND_REPEAT_COUNT; i++) {
        if (i > 0)
          delay(SEND_REPEAT_INTERVAL);
        this->sendMsg((byte *) &message, sizeof message);
      }
      this->enableReceive();
    }

    void loop() {
      if (_lowLevelConnector.update()) {
        if (_messageCallback != NULL) {
          ModuleMessage message;
          memset (&message, 0, sizeof message);
          memcpy(&message, _lowLevelConnector.getData(), _lowLevelConnector.getLength());

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
          if (lastProcessedCounterByModule[message.moduleId] == message.counter) {
            // This message has already been processed
            return;
          }

          Serial.print("Message #");
          Serial.print(message.counter);
          Serial.print(" received from module #");
          Serial.println(message.moduleId);
          lastProcessedCounterByModule[message.moduleId] = message.counter;
          _messageCallback(message);
        }
      }
    }

};
#endif
