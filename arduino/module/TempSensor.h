#ifndef TempSensor_h
#define TempSensor_h

#include "Arduino.h"
#include <OneWire.h>

#define PIN_ONE_WIRE 2

class TempSensor
{
    typedef void (*TemperatureAvailableCallback)  (int sensorId, float temperatureInCelsius);

  public:

    TempSensor(int sensorId, TemperatureAvailableCallback cb) :
      _ds(PIN_ONE_WIRE),
      _tempCb(cb)
    {
      _sensorId = sensorId;
      if (_selectFirstAddr()) {
        _requestTemperatureReading();
      }
      else {
        Serial.println("No temperature sensor");
      }
    }

    void loop() {
      long delta = millis() - _lastRequestTime;
      if (_waitForNextPeriod) {
        if (delta > _readPeriodInMs) {
          _requestTemperatureReading();
          _waitForNextPeriod = false;
        }      
      }
      else {
        if (delta > 1000) {
          _readTemperature();
          // Do nothing until the next period
          _waitForNextPeriod = true;
        }        
      }
    }

    float getTemperature() {
      return _temperature;
    }

    int getSensorId() {
      return _sensorId;
    }

  private:
    int _sensorId;
    long _readPeriodInMs = 60000; // 300000;
    unsigned long _lastRequestTime = 0;
    bool _waitForNextPeriod = false;
    TemperatureAvailableCallback _tempCb;

    float _temperature;
    OneWire  _ds;
    byte _addr[8];

    void _readTemperature() {
      byte present = _ds.reset();
      _ds.select(_addr);
      _ds.write(0xBE);         // Read Scratchpad

      byte data[12];
      for (int i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = _ds.read();
      }
      int16_t raw = (data[1] << 8) | data[0];
      byte cfg = (data[4] & 0x60);

      float celsius = (float)raw / 16.0;
      if (_tempCb != NULL) {
        _tempCb(_sensorId, celsius);
      }
    }

    void _requestTemperatureReading() {
      _ds.reset();
      _ds.select(_addr);
      _ds.write(0x44);        // start conversion, use ds.write(0x44,1) with parasite power on at the end
      _lastRequestTime = millis();
    }

    bool _selectFirstAddr() {
      if ( !_ds.search(_addr)) {
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
      if (OneWire::crc8(_addr, 7) != _addr[7]) {
        Serial.println("CRC is not valid!");
        return false;
      }

      return true;
    }

};

#endif
