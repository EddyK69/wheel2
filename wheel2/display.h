#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "amplifier.h"
#include "arm.h"
#include "buttons.h"
#include "carriage.h"
#include "orientation.h"
#include "plateau.h"
#include "scanner.h"
#include "speedcomp.h"
#include "storage.h"

#define DISPLAY_LENGTH 120


class Display_ {
 private:
    Display_() = default; // Make constructor private

  public:
    static Display_& getInstance(); // Accessor for singleton instance

    Display_(const Display_&) = delete; // no copying
    Display_& operator=(const Display_&) = delete;

  private:
    Interval _interval = Interval(10000, TM_MICROS);
    int _trackCounter = 0;
    uint64_t _delay = 0;
    const int _dispHalf = DISPLAY_LENGTH / 2;
    float _data[DISPLAY_LENGTH];
    void clear();
    int mapRealPos2Display(float pos);
    void drawBlock(int start, int end, float color);
    void drawPoint(int pos, float color);
    void flipData();
    void print(float time);
    void commit();
  public:
    void init();
    void update();
    void bootLED();
}; // Display_

extern Display_& Display;

#endif // DISPLAY_H
