#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "amplifier.h"
#include "arm.h"
#include "bluetooth.h"
#include "carriage.h"
#include "orientation.h"
#include "plateau.h"
#include "scanner.h"

#define BUTTON_COUNT            8

#define BUTTON_NEXT             5
#define BUTTON_PLAY             6
#define BUTTON_PREV             7

#define BUTTON_PRESS            1
#define BUTTON_LONG_PRESS       2
#define BUTTON_SUPERLONG_PRESS  3
#define BUTTON_RELEASE          0

#define BUTTON_LONG_CLICK       700
#define BUTTON_SUPERLONG_CLICK  3000


class Buttons_ {
  private:
    Buttons_() = default; // Make constructor private

  public:
    static Buttons_& getInstance(); // Accessor for singleton instance

    Buttons_(const Buttons_&) = delete; // no copying
    Buttons_& operator=(const Buttons_&) = delete;

  private:
    Interval _interval = Interval(10000, TM_MICROS);
    Interval _allButtonsInterval = Interval(0, TM_MILLIS);
    int _buttonIn[BUTTON_COUNT];
    uint64_t _buttonInterval[BUTTON_COUNT];
    int potVal = 0;
    int potValPrev = 0;
    float belt = 0;
    float beltPrev = 0;
    float beltFilter;
    float beltFilterPrev;
    float beltDiff;
    void readData();
    void logic(int button);
    void ledBlink();
    bool isButtonNext(int button);
    bool isButtonPrev(int button);
    int buttonNextComp();
    int buttonPrevComp();
    void log(int button, String action);
    String getButton(int button);
  public:
    Interval rpmDisplayActionInterval = Interval(0, TM_MILLIS);
    Interval volumeDisplayActionInterval = Interval(0, TM_MILLIS);
    Interval ledBlinkInterval = Interval(0, TM_MILLIS);
    int state[BUTTON_COUNT];
    void init();
    void update();
    void info();
}; // Buttons_

extern Buttons_& Buttons;

#endif // BUTTONS_H
