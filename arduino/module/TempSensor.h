#ifndef TempSensor_h
#define TempSensor_h

#include "Arduino.h"
#include <OneWire.h>
#include "RS485Connector.h"

#define PIN_ONE_WIRE 2

class TempSensor
{
public:
  TempSensor(RS485Connector connector, byte sensorId) : _connector(connector),
                                                        _ds(PIN_ONE_WIRE)
  {
#if DEBUGMSG
    Serial.print(F("Declare temperature sensor #"));
    Serial.print(sensorId);
#endif
    _sensorId = sensorId;
    _temperatureSensorExists = _selectFirstAddr();
    if (_temperatureSensorExists)
    {
      _requestTemperatureReading();
#if DEBUGMSG
      Serial.println(F(" : OK, found"));
#endif
    }
    else
    {
#if DEBUGMSG
      Serial.println(F(" : Error : no temperature sensor could be found"));
#endif
    }
  }

  void loop()
  {
    if (!_temperatureSensorExists)
      return;
    long delta = millis() - _lastRequestTime;
    if (_waitForNextPeriod)
    {
      if (delta > _readPeriodInMs)
      {
        _requestTemperatureReading();
        _waitForNextPeriod = false;
      }
    }
    else
    {
      if (delta > 1000)
      {
        // The DS18B20 needs 750ms to read the temperature. Leave it 1000ms to make sure it's OK.
        _readTemperature();
        // Do nothing until the next period
        _waitForNextPeriod = true;
      }
    }
  }

  float getTemperature()
  {
    return _temperature;
  }

  byte getSensorId()
  {
    return _sensorId;
  }

private:
  byte _sensorId;
  long _readPeriodInMs = 60000; // 300000;
  unsigned long _lastRequestTime = 0;
  bool _waitForNextPeriod = false;
  bool _temperatureSensorExists = false;
  RS485Connector _connector;
  float _temperature;
  OneWire _ds;
  byte _addr[8];

  void _readTemperature()
  {
    byte present = _ds.reset();
    _ds.select(_addr);
    _ds.write(0xBE); // Read Scratchpad

    byte data[12];
    for (int i = 0; i < 9; i++)
    { // we need 9 bytes
      data[i] = _ds.read();
    }
    int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);

    float celsius = (float)raw / 16.0;
    _sendTemperatureSensorValue(celsius);
  }

  void _requestTemperatureReading()
  {
    _ds.reset();
    _ds.select(_addr);
    _ds.write(0x44); // start conversion, use ds.write(0x44,1) with parasite power on at the end
    _lastRequestTime = millis();
    // From now, we have to wait at least for 750ms before reading the temperature
  }

  bool _selectFirstAddr()
  {
    if (!_ds.search(_addr))
    {
      _ds.reset_search();
      return false;
    }

    /*
        Serial.print("ROM =");
        for (int i = 0; i < 8; i++) {
          Serial.write(' ');
          Serial.print(_addr[i], HEX);
        }
        Serial.println();
      */
    if (OneWire::crc8(_addr, 7) != _addr[7])
    {
#if DEBUGMSG
      Serial.println(F("CRC is not valid!"));
#endif
      return false;
    }

    return true;
  }

  int _sendTemperatureSensorValue(float temperatureInCelsius)
  {
    union MessagePayload payload;
    payload.temperatureInCelsius = temperatureInCelsius;
    return _connector.sendMessage(_sensorId, DeviceClass_TemperatureSensor, payload);
  }
};

#endif
