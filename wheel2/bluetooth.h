#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "carriage.h"
#include "plateau.h"

#define BT_WIRELESS_VERSION false;

//------------------------------ command starts with this character
#define BT_BUTTON_IN      "4"
#define BT_BUTTON_OUT     "c"

//------------------------------ command ends with this character
#define BT_NEXT_TRACK     "b"
#define BT_PREV_TRACK     "c"
#define BT_SKIP_FORWARD   "9"
#define BT_SKIP_REVERSE   "8"

#define BT_PLAY           "4"   // headset on
#define BT_PAUSE          "6"  // headset off?

// #define BT_PAUZE_LANG

//AT+
//AT+SCAN
//AT+REST
//AT+DELVMLINK


class Bluetooth_ {
  private:
    Bluetooth_() = default; // Make constructor private

  public:
    static Bluetooth_& getInstance(); // Accessor for singleton instance

    Bluetooth_(const Bluetooth_&) = delete; // no copying
    Bluetooth_& operator=(const Bluetooth_&) = delete;

  private:
    Interval _interval = Interval(200, TM_MILLIS);
    Interval _checkBeforeStartInterval = Interval(2000, TM_MILLIS);
    bool _initTodo = true;
    bool _wirelessVersion = false;
    String _buffer = "";
    void encode();
  public:
    const bool wirelessVersion = BT_WIRELESS_VERSION;
    bool debug = false;
    void init();
    void func();
    void write(String command);
}; // Bluetooth_

extern Bluetooth_& Bluetooth;

#endif // BLUETOOTH_H
