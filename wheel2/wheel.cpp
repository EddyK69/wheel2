#include "log.h"
#include "wheel.h"
#include "pins.h"


Wheel::Wheel() :
    scanner(), // carriage is passed via init()
    carriage(scanner),
    buttons(bluetooth, carriage, scanner),
    storage(carriage),
    display(buttons, carriage, scanner, storage),
    serialcomm(bluetooth, buttons, carriage, scanner, storage),
    bluetooth(carriage) {
} // Wheel()


void Wheel::init() {
  LOG_DEBUG("wheel.cpp", "[init]");
  serialcomm.init();

  storage.init();

  Amplifier.init();
  Orientation.init();
  Arm.init();
  buttons.init();
  carriage.init();
  scanner.init(&carriage); // to prevent circular reference
  Plateau.init();
  SpeedComp.init(&carriage); // to prevent circular reference

  storage.read();

  // Set pinmodes
  pinMode(SLEEPMODE_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(SLEEPMODE_PIN, 1); // keep battery on
  // digitalWrite(LED_PIN, 1); // turn LED on
} // init()
