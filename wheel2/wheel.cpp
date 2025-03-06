#include "log.h"
#include "wheel.h"
#include "pins.h"


Wheel::Wheel() :
    scanner(),
    buttons(bluetooth, scanner),
    storage(),
    display(buttons, scanner, storage),
    serialcomm(bluetooth, buttons, scanner, storage),
    bluetooth() {
} // Wheel()


void Wheel::init() {
  LOG_DEBUG("wheel.cpp", "[init]");
  serialcomm.init();

  storage.init();

  Amplifier.init();
  Orientation.init();
  Arm.init();
  buttons.init();
  Carriage.init();
  scanner.init();
  Plateau.init();
  SpeedComp.init();

  storage.read();

  // Set pinmodes
  pinMode(SLEEPMODE_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(SLEEPMODE_PIN, 1); // keep battery on
  // digitalWrite(LED_PIN, 1); // turn LED on
} // init()
