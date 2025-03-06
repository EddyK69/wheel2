#ifndef SERIALCOMM_H
#define SERIALCOMM_H

#define SERIAL_BAUDRATE 115200

#include <Arduino.h>
#include "enums.h"
#include "interval.h"
#include "shared.h"
#include "amplifier.h"
#include "arm.h"
#include "bluetooth.h"
#include "buttons.h"
#include "carriage.h"
#include "orientation.h"
#include "plateau.h"
#include "scanner.h"
#include "speedcomp.h"
#include "storage.h"


class SerialComm_ {
 private:
    SerialComm_() = default; // Make constructor private

  public:
    static SerialComm_& getInstance(); // Accessor for singleton instance

    SerialComm_(const SerialComm_&) = delete; // no copying
    SerialComm_& operator=(const SerialComm_&) = delete;

  private:
    Interval _interval = Interval(10000, TM_MICROS);
    Interval _uptimeInterval = Interval(60, TM_MINS);
    String _line = "";
    String _lineRaw = "";
    String _lastCommand = "";
    bool _graphicData = false;
    bool _headerShown = false;

    void checkReceivedLine(String line, eCheckMode mode = CM_NONE);
    bool checkLineCommand(String command, String description, eCheckMode mode);
    bool checkLine(String command, String description, eCheckMode mode);
    bool checkLineInt(String command, String description, eCheckMode mode, int& value);
    bool checkLineFloat(String command, String description, eCheckMode mode, float& value);
    bool checkLineBool(String command, String description, eCheckMode mode, bool& value);
    void println(eCheckMode mode);
    void printCommando(String command, String description);
    void printValue(String command, String description, String value);
    void printGraphicData();
    void report();
    void info();
    void version();
  public:
    void init();
    void func();
}; // SerialComm_

extern SerialComm_& SerialComm;

#endif // SERIALCOMM_H
