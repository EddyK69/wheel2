#include "log.h"
#include "wheel.h"
#include "pins.h"


Wheel::Wheel() :
    arm(),
    orientation(arm),
    speedcomp(arm), // carriage & plateau are passed via init()
    plateau(speedcomp),
    scanner(plateau), // carriage is passed via init()
    carriage(arm, plateau, scanner), // speedcomp is passed via init()
    buttons(arm, bluetooth, carriage, orientation, plateau, scanner),
    storage(arm, carriage, orientation, plateau),
    display(arm, buttons, carriage, orientation, plateau, scanner, speedcomp, storage),
    serialcomm(arm, bluetooth, buttons, carriage, orientation, plateau, scanner, speedcomp, storage),
    bluetooth(carriage, plateau) {
} // Wheel()


void Wheel::init() {
  LOG_DEBUG("wheel.cpp", "[init]");
  serialcomm.init();

  storage.init();

  Amplifier.init();
  orientation.init();
  arm.init();
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
