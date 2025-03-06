#include "log.h"
#include "wheel.h"
#include "pins.h"


Wheel::Wheel() :
    buttons(),
    display(buttons),
    serialcomm(buttons) {
} // Wheel()


void Wheel::init() {
  LOG_DEBUG("wheel.cpp", "[init]");
  serialcomm.init();

  Storage.init();

  Amplifier.init();
  Orientation.init();
  Arm.init();
  buttons.init();
  Carriage.init();
  Scanner.init();
  Plateau.init();
  SpeedComp.init();

  Storage.read();

  // Set pinmodes
  pinMode(SLEEPMODE_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(SLEEPMODE_PIN, 1); // keep battery on
  // digitalWrite(LED_PIN, 1); // turn LED on
} // init()
