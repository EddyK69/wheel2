#include "log.h"
#include "wheel.h"
#include "pins.h"


void Wheel_::init() {
  LOG_DEBUG("wheel.cpp", "[init]");
  SerialComm.init();

  Storage.init();

  Amplifier.init();
  Orientation.init();
  Arm.init();
  Buttons.init();
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


Wheel_ &Wheel_::getInstance() {
  static Wheel_ instance;
  return instance;
} // getInstance()


Wheel_ &Wheel = Wheel.getInstance();
