#ifndef RS485Connector_h
#define RS485Connector_h
#include "LowLevelNonBlockingRS485Connector.h"
#include <SoftwareSerial.h>
#include "Arduino.h"

#define RS485_SERIAL_SPEED 19200

enum SystemMessage
{
  SystemMessage_Started,
  SystemMessage_Heartbeat
};

enum DeviceClass
{
  DeviceClass_System,
  DeviceClass_Button,
  DeviceClass_Light,
  DeviceClass_TemperatureSensor
};

union MessagePayload {
  float temperatureInCelsius;
  int buttonMessage;
  int lightMessage;
  int systemMessage;
};

typedef struct
{
  int crc;
  int counter;
  byte moduleId;
  byte deviceId;
  DeviceClass deviceClass;
  MessagePayload payload;
} ModuleMessage;

#define SEND_REPEAT_COUNT 10
#define SEND_REPEAT_INTERVAL 10
#define MAX_MODULE_COUNT 18

class RS485Connector : IReadByte, IWriteByte, IBytesAvailable
{
  typedef void (*MessageCallback)(const ModuleMessage message);

  const SoftwareSerial *_softwareSerial;
  const byte _moduleId;
  const byte _rxPin;
  const byte _txPin;
  const byte _enPin;
  const LowLevelNonBlockingRS485Connector _lowLevelConnector;
  int _globalCounter = 0;
  MessageCallback _messageCallback;
  int lastProcessedCounterByModule[MAX_MODULE_COUNT];

#define RS485Transmit HIGH
#define RS485Receive LOW

private:
  // send data
  void _sendMsg(const byte *data, const byte length)
  {
    _lowLevelConnector.sendMsg(data, length);
  }

  unsigned long _computeMessageCrc(void *message, int len)
  {
    unsigned char *data;
    unsigned long chk = 0;
    data = message;
    // Skip the crc
    data = data + sizeof(int);
    for (byte i = sizeof(int); i < sizeof(ModuleMessage); i++)
    {
      chk += *data;
      data++;
    }
    return chk;
  }

  int readByte()
  {
    return _softwareSerial->read();
  }

  size_t writeByte(const int what)
  {
    return _softwareSerial->write(what);
  }

  int bytesAvailable()
  {
    return _softwareSerial->available();
  }

  void _enableTransmit()
  {
    digitalWrite(_enPin, RS485Transmit); // Enable RS485 Transmit
  }
  void _enableReceive()
  {
    digitalWrite(_enPin, RS485Receive); // Enable RS485 Receive
  }

public:
  RS485Connector::RS485Connector(byte moduleId) : RS485Connector(moduleId, A1, A3, A2) {}

  RS485Connector::RS485Connector(byte moduleId, byte rxPin, byte txPin, byte enPin) : _moduleId(moduleId),
                                                                                      _rxPin(rxPin),
                                                                                      _txPin(txPin),
                                                                                      _enPin(enPin),
                                                                                      _softwareSerial(new SoftwareSerial(rxPin, txPin)),
                                                                                      _lowLevelConnector(this, this, this, 20)
  {
    for (byte i = 0; i < MAX_MODULE_COUNT; i++)
      lastProcessedCounterByModule[i] = -1;
  }

  void init(MessageCallback messageCallback, long serialSpeed = RS485_SERIAL_SPEED)
  {
    _messageCallback = messageCallback;
    pinMode(_enPin, OUTPUT);
    _softwareSerial->begin(serialSpeed); // set the data rate
    this->_enableTransmit();
    _lowLevelConnector.begin();
    this->_enableReceive();
  }

  int sendMessage(byte deviceId, DeviceClass deviceClass, MessagePayload payload)
  {
#if DEBUGMSG
    unsigned long d0 = millis();
#endif
    this->_enableTransmit();
    ModuleMessage message;

    memset(&message, 0, sizeof message);
    message.moduleId = _moduleId;
    message.deviceClass = deviceClass;
    message.counter = this->_globalCounter++;
    message.deviceId = deviceId;
    message.payload = payload;
    message.crc = this->_computeMessageCrc(&message, sizeof message);
    // Send the message more than once to make sure it will be delivered
    // Note : there is no collision management
    for (byte i = 0; i < SEND_REPEAT_COUNT; i++)
    {
      if (i > 0)
      {
        // Use a random delay to maximize collision avoidance
        delay(random(SEND_REPEAT_INTERVAL - 5, SEND_REPEAT_INTERVAL + 5));
      }
      this->_sendMsg((byte *)&message, sizeof message);
    }
    this->_enableReceive();
#if DEBUGMSG
    unsigned long d1 = millis() - d0;
    Serial.print(F("Sent duration : "));
    Serial.print(d1);
    Serial.println(F(" ms"));
#endif
  }

  int sendSystemMessage(SystemMessage systemMessage)
  {
    union MessagePayload payload;
    payload.systemMessage = systemMessage;
    return sendMessage(0, DeviceClass_System, payload);
  }

  void loop()
  {
    if (!_lowLevelConnector.update())
    {
      // No RS485 message is available
      return;
    }
    if (_messageCallback == NULL)
    {
      // There is no callback, no need to process the message
      return;
    }
    // Let's decode the message
    ModuleMessage message;
    // Build a message from the RS-485 data
    memset(&message, 0, sizeof message);
    memcpy(&message, _lowLevelConnector.getData(), _lowLevelConnector.getLength());

    // Check its CRC
    int expectedCrc = this->_computeMessageCrc(&message, sizeof message);
    if (expectedCrc != message.crc)
    {
#if DEBUGMSG
      Serial.println(F("Message received with invalid CRC"));
#endif
      return;
    }
    // RS-485 are sent more than once to make sure they will be transmitted (avoid collision management)
    // Let's check if this message has not benn already processed
    if (lastProcessedCounterByModule[message.moduleId] == message.counter)
    {
      // This message has already been processed
      return;
    }

#if DEBUGMSG
    Serial.print(F("Message #"));
    Serial.print(message.counter);
    Serial.print(F(" received from module #"));
    Serial.println(message.moduleId);
#endif
    lastProcessedCounterByModule[message.moduleId] = message.counter;
    // Invoke the callback
    _messageCallback(message);
  }
};
#endif
