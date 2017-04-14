#ifndef RS485Connector_h
#define RS485Connector_h
#include "RS485_non_blocking.h"
#include <SoftwareSerial.h>
#include "Arduino.h"

typedef struct
{
  int crc;
  int counter;
  int moduleId;
  int deviceId;
  char text[20];
}  ModuleMessage;

#define SEND_REPEAT_COUNT 5
#define SEND_REPEAT_INTERVAL 50

class RS485Connector : IReadByte, IWriteByte, IBytesAvailable {
    typedef void (*MessageCallback)  (const ModuleMessage message);

    const SoftwareSerial* _softwareSerial;
    const int _moduleId;
    const int _rxPin;
    const int _txPin;
    const int _enPin;
    const RS485 _myChannel;
    int _globalCounter = 0;
    MessageCallback _messageCallback;


#define RS485Transmit    HIGH
#define RS485Receive     LOW

  private:
    // send data
    void sendMsg (const byte * data, const byte length);

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

    void DumpModuleMessage2(ModuleMessage message) {
      /*
        Serial.print("CRC : ");
        Serial.println(message.crc);
        Serial.print("moduleId : ");
        Serial.println(message.moduleId);
        Serial.print("text : ");
        Serial.println(message.text);
        Serial.print("counter : ");
        Serial.println(message.counter);
      */
    }

    bool CheckModuleMessageCrc2(ModuleMessage message) {
      return (ComputeMessageCrc2(&message, sizeof message) == message.crc);
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
    RS485Connector(int moduleId, int rxPin, int txPin, int enPin);

    void init(int serialSpeed);

    void enableTransmit();
    void enableReceive();

    void setMessageCallback(MessageCallback messageCallback);

    int sendMessage(int deviceId, const char msg[]);

    void loop();

};
#endif
