#include "log.h"
#include "wheel.h"
#include "pins.h"


Wheel::Wheel() :
    orientation(),
    speedcomp(), // carriage & plateau are passed via init()
    plateau(speedcomp),
    scanner(plateau), // carriage is passed via init()
    carriage(plateau, scanner), // speedcomp is passed via init()
    buttons(bluetooth, carriage, orientation, plateau, scanner),
    storage(carriage, orientation, plateau),
    display(buttons, carriage, orientation, plateau, scanner, speedcomp, storage),
    serialcomm(bluetooth, buttons, carriage, orientation, plateau, scanner, speedcomp, storage),
    bluetooth(carriage, plateau) {
} // Wheel()


void Wheel::init() {
  LOG_DEBUG("wheel.cpp", "[init]");
  serialcomm.init();

  storage.init();

  Amplifier.init();
  orientation.init();
  Arm.init();
  buttons.init();
  carriage.init(&speedcomp); // to prevent circular reference
  scanner.init(&carriage); // to prevent circular reference
  plateau.init();
  speedcomp.init(&carriage, &plateau); // to prevent circular reference

  storage.read();

  // Set pinmodes
  pinMode(SLEEPMODE_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(SLEEPMODE_PIN, 1); // keep battery on
  // digitalWrite(LED_PIN, 1); // turn LED on
} // init()
