#ifndef ButtonHandler_h
#define ButtonHandler_h

#include "Arduino.h"

// ----- Callback function types -----

extern "C" {
  typedef void (*callbackFunction)(int);
}


class ButtonHandler
{
  public:
    // ----- Constructor -----
    ButtonHandler(int buttonId, int pin) {
      _buttonId = buttonId;
      _clickTicks = 300;        // number of millisec that have to pass by before a click is detected.
      _pressTicks = 3000;       // number of millisec that have to pass by before a long button press is detected.

      _state = 0; // starting with state 0: waiting for button to be pressed
      _isLongPressed = false;  // Keep track of long press state

      _doubleClickFunc = NULL;
      _pressFunc = NULL;
      _longPressStartFunc = NULL;
      _longPressStopFunc = NULL;
      _duringLongPressFunc = NULL;

      pinMode(pin, INPUT);      // sets the MenuPin as input
      _pin = pin;

      _buttonReleased = HIGH; // notPressed
      _buttonPressed = LOW;
      digitalWrite(pin, HIGH);   // turn on pullUp resistor
    }

    // ----- Set runtime parameters -----

    // set # millisec after single click is assumed.
    void setClickTicks(int ticks) {
      _clickTicks = ticks;

    }

    // set # millisec after press is assumed.
    void setPressTicks(int ticks) {
      _pressTicks = ticks;

    }

    // attach functions that will be called when button was pressed in the specified way.
    void attachClick(callbackFunction newFunction) {
      _clickFunc = newFunction;
    }
    void attachDoubleClick(callbackFunction newFunction) {
      _doubleClickFunc = newFunction;
    }
    void attachLongPressStart(callbackFunction newFunction) {
      _longPressStartFunc = newFunction;

    }
    void attachLongPressStop(callbackFunction newFunction) {
      _longPressStopFunc = newFunction;
    }
    void attachDuringLongPress(callbackFunction newFunction) {
      _duringLongPressFunc = newFunction;
    }

    // ----- State machine functions -----

    // call this function every some milliseconds for handling button events.
    void loop() {
      // Detect the input information
      int buttonLevel = digitalRead(_pin); // current button signal.
      unsigned long now = millis(); // current (relative) time in msecs.

      // Implementation of the state machine
      if (_state == 0) { // waiting for menu pin being pressed.
        if (buttonLevel == _buttonPressed) {
          _state = 1; // step to state 1
          _startTime = now; // remember starting time
        } // if

      } else if (_state == 1) { // waiting for menu pin being released.

        if ((buttonLevel == _buttonReleased) && ((unsigned long)(now - _startTime) < _debounceTicks)) {
          // button was released to quickly so I assume some debouncing.
          // go back to state 0 without calling a function.
          _state = 0;

        } else if (buttonLevel == _buttonReleased) {
          _state = 2; // step to state 2

        } else if ((buttonLevel == _buttonPressed) && ((unsigned long)(now - _startTime) > _pressTicks)) {
          _isLongPressed = true;  // Keep track of long press state
          if (_pressFunc) _pressFunc(_buttonId);
          if (_longPressStartFunc) _longPressStartFunc(_buttonId);
          if (_duringLongPressFunc) _duringLongPressFunc(_buttonId);
          _state = 6; // step to state 6

        } else {
          // wait. Stay in this state.
        } // if

      } else if (_state == 2) { // waiting for menu pin being pressed the second time or timeout.
        if ((unsigned long)(now - _startTime) > _clickTicks) {
          // this was only a single short click
          if (_clickFunc) _clickFunc(_buttonId);
          _state = 0; // restart.

        } else if (buttonLevel == _buttonPressed) {
          _state = 3; // step to state 3
        } // if

      } else if (_state == 3) { // waiting for menu pin being released finally.
        if (buttonLevel == _buttonReleased) {
          // this was a 2 click sequence.
          if (_doubleClickFunc) _doubleClickFunc(_buttonId);
          _state = 0; // restart.
        } // if

      } else if (_state == 6) { // waiting for menu pin being release after long press.
        if (buttonLevel == _buttonReleased) {
          _isLongPressed = false;  // Keep track of long press state
          if (_longPressStopFunc) _longPressStopFunc(_buttonId);
          _state = 0; // restart.
        } else {
          // button is being long pressed
          _isLongPressed = true; // Keep track of long press state
          if (_duringLongPressFunc) _duringLongPressFunc(_buttonId);
        } // if

      } // if
    }
    bool isLongPressed() {
      return _isLongPressed;

    }
    int getPin() {
      return _pin;
    }
    int getButtonId() {
      return _buttonId;
    }

  private:
    int _pin;        // hardware pin number.
    int _buttonId;   // buttonId.
    int _clickTicks; // number of ticks that have to pass by before a click is detected
    int _pressTicks; // number of ticks that have to pass by before a long button press is detected
    const int _debounceTicks = 50; // number of ticks for debounce times.

    int _buttonReleased;
    int _buttonPressed;

    bool _isLongPressed;

    // These variables will hold functions acting as event source.
    callbackFunction _clickFunc;
    callbackFunction _doubleClickFunc;
    callbackFunction _pressFunc;
    callbackFunction _longPressStartFunc;
    callbackFunction _longPressStopFunc;
    callbackFunction _duringLongPressFunc;

    // These variables that hold information across the upcoming tick calls.
    // They are initialized once on program start and are updated every time the tick function is called.
    int _state;
    unsigned long _startTime; // will be set in state 1
};

#endif


