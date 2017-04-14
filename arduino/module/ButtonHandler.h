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
  ButtonHandler(int buttonId);

  void init(int pin);
  
  // ----- Set runtime parameters -----

  // set # millisec after single click is assumed.
  void setClickTicks(int ticks);

  // set # millisec after press is assumed.
  void setPressTicks(int ticks);

  // attach functions that will be called when button was pressed in the specified way.
  void attachClick(callbackFunction newFunction);
  void attachDoubleClick(callbackFunction newFunction);
  void attachPress(callbackFunction newFunction); // DEPRECATED, replaced by longPressStart, longPressStop and duringLongPress
  void attachLongPressStart(callbackFunction newFunction);
  void attachLongPressStop(callbackFunction newFunction);
  void attachDuringLongPress(callbackFunction newFunction);

  // ----- State machine functions -----

  // call this function every some milliseconds for handling button events.
  void loop(void);
  bool isLongPressed();
  int getPin();
  int getButtonId();

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


