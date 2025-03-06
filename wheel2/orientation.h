#ifndef ORIENTATION_H
#define ORIENTATION_H

/**
Makes use of QMA7981, a Single-Chip 3-Axis Accelerometer
https://datasheet.lcsc.com/lcsc/2004281102_QST-QMA7981_C457290.pdf

I2C Slave Address: b0010010
*/

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "arm.h"


class Orientation_ {
  private:
    Orientation_() = default; // Make constructor private

  public:
    static Orientation_& getInstance(); // Accessor for singleton instance

    Orientation_(const Orientation_&) = delete; // no copying
    Orientation_& operator=(const Orientation_&) = delete;

  private:
    Interval _interval = Interval(10, TM_MILLIS);
    Interval _isOkInterval = Interval(0, TM_MILLIS);
    const byte _i2cAdress = 0b0010010;
    bool _firstTime = true;
    bool _error = false;
    bool _errorPrev = false;
    bool _isStandingPrev = false;
    bool _headerShown = false;
    float _rawX, _rawY, _rawZ;
    void printGraphicData();
  public:
    bool graphicData = false;
    bool isStanding = false;
    float x, y, z;
    float offsetX = 0;
    float offsetY = 0;
    float offsetZ = 0;
    void init();
    void calibrate();
    void reset();
    void update();
    void info();
}; // Orientation_

extern Orientation_& Orientation;

#endif // ORIENTATION_H
