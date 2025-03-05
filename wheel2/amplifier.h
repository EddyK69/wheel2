#ifndef AMPLIFIER_H
#define AMPLIFIER_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "arm.h"


class Amplifier {
  private:
    Interval _interval;
    Arm& _arm;
    int _volumePrev;
    bool _isNeedleDownPrev = false;
    bool _volumeOverRidePrev = false;
    bool isNeedeDownLongEnough();
  public:
    bool volumeOverRide = false;
    int volume = 22;
    Amplifier(Arm& arm);
    void init();
    void func();
}; // Amplifier


#endif // AMPLIFIER_H
