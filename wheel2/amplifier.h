#ifndef AMPLIFIER_H
#define AMPLIFIER_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "arm.h"


class Amplifier_ {
  private:
    Amplifier_() = default; // Make constructor private

  public:
    static Amplifier_& getInstance(); // Accessor for singleton instance

    Amplifier_(const Amplifier_&) = delete; // no copying
    Amplifier_& operator=(const Amplifier_&) = delete;
  
  private:
    Interval _interval = Interval(20, TM_MILLIS);
    Arm _arm;
    int _volumePrev;
    bool _isNeedleDownPrev = false;
    bool _volumeOverRidePrev = false;
    bool isNeedeDownLongEnough();
  public:
    bool volumeOverRide = false;
    int volume = 22;
    void init();
    void func();
}; // Amplifier_

extern Amplifier_& Amplifier;

#endif // AMPLIFIER_H
