#ifndef WHEEL_H
#define WHEEL_H

#include <Arduino.h>
#include "shared.h"
#include "amplifier.h"
#include "arm.h"
#include "bluetooth.h"
#include "buttons.h"
#include "carriage.h"
#include "display.h"
#include "orientation.h"
#include "plateau.h"
#include "scanner.h"
#include "serialcomm.h"
#include "speedcomp.h"
#include "storage.h"


class Wheel_ {
  private:
    Wheel_() = default; // Make constructor private

  public:
    static Wheel_& getInstance(); // Accessor for singleton instance

    Wheel_(const Wheel_&) = delete; // no copying
    Wheel_& operator=(const Wheel_&) = delete;

  public:
    void init();
}; // Wheel_

extern Wheel_& Wheel;

#endif // WHEEL_H
