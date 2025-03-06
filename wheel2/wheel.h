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


class Wheel {
  public:
    Wheel();
    void init();
}; // Wheel


#endif // WHEEL_H
