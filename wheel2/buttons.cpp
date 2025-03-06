#include "log.h"
#include "buttons.h"
#include "pins.h"
#include "helper.h"


void Buttons_::init() {
  LOG_DEBUG("buttons.cpp", "[init]");
  // Nothing to do really ;)
} // init()


void Buttons_::update() {
  if (_interval.tick()) {
    readData();

    for (int button = BUTTON_NEXT; button <= BUTTON_PREV; button++) {
      logic(button);
    }

    potVal = analogRead(DISPLAY_POTMETER_PIN);
    if (!isApprox(potVal, potValPrev, ARM_AMAX / 2)) {
      if (potVal > potValPrev) {
        potValPrev += ARM_AMAX;
      } else {
        potValPrev -= ARM_AMAX;
      }
    }

    belt +=  (float(potVal - potValPrev) * 80 ) / ARM_AMAX;
    potValPrev = potVal;
    beltFilter += (belt - beltFilter) / 3;

    if (!isApprox(beltFilter, beltFilterPrev, 1)) {
      beltDiff = beltFilter - beltFilterPrev;
      beltFilterPrev = beltFilter;

      if (millisSinceBoot() < 1000) { // belt action only after 1 sec.
        return;
      }

      if (Shared.state == S_NEEDLE_CLEAN && Arm.motorOn) {
        Arm.targetWeight += beltDiff * 0.0333;
        Arm.targetWeight = limitFloat(Arm.targetWeight, ARM_MIN_WEIGHT, ARM_MAX_WEIGHT);
      }
      if (!Orientation.isStanding) {
        beltDiff = -beltDiff; // flip
      }

      if (Shared.state == S_CALIBRATE) {
        Arm.force += beltDiff * 0.001;
        Arm.force = limitFloat(Arm.force, 0, 1);

      } else if(Shared.state == S_PAUSE) {
        Carriage.targetTrack -= beltDiff * 0.25;
        Carriage.targetTrack = limitFloat(Carriage.targetTrack, CARRIAGE_RECORD_END, Scanner.recordStart);

      } else {
        // to prevent volume popping up after button press while skipping
        if (Shared.state != S_SKIP_FORWARD && Shared.state != S_SKIP_REVERSE && Shared.state != S_GOTO_TRACK && Shared.state != S_PAUSE) { 
          volumeDisplayActionInterval.reset();
        }

        Amplifier.volume += round(beltDiff);
        Amplifier.volume = limitInt(Amplifier.volume, 0, 63);
      }
    }
  } // _interval.tick()
} // update()


void Buttons_::readData() {
  gpio_put(DISPLAY_LATCH_PIN, 0);
  delayMicroseconds(1);
  gpio_put(DISPLAY_LATCH_PIN, 1);

  for (int button = 0; button < BUTTON_COUNT; button++) {
    delayMicroseconds(1);
    _buttonIn[button] = digitalRead(DISPLAY_OUT_PIN);

    gpio_put(DISPLAY_CLOCK_PIN, 1);
    delayMicroseconds(1);
    gpio_put(DISPLAY_CLOCK_PIN, 0);
  }  
} // readData()


void Buttons_::logic(int button) {
  //--------------------------------------------- SHORT PRESS
  if (state[button] == BUTTON_RELEASE && _buttonIn[button] == BUTTON_PRESS) {
    state[button] = BUTTON_PRESS;

    _allButtonsInterval.reset();
    _buttonInterval[button] = millisSinceBoot();

    ledBlink();

    log(button, "PRESS");

    if (button == BUTTON_PLAY) {
      if (Shared.state == S_HOMING_BEFORE_PLAYING || Shared.state == S_GOTO_RECORD_START) {
        Shared.puristMode = true;
        Serial.println("PURIST MODE: ON");
      }

      if (Shared.state == S_HOME ) {
        Plateau.play();
        state[button] = BUTTON_LONG_PRESS; // To prevent stopping by long press
      }
    }

    if (Shared.state == S_NEEDLE_CLEAN || Shared.state == S_RECORD_CLEAN) { // Stop clean mode
      Plateau.stop();
    }
    return;
  }

  //--------------------------------------------- SHORT RELEASE
  if (state[button] == BUTTON_PRESS && _buttonIn[button] == BUTTON_RELEASE) {
    state[button] = BUTTON_RELEASE;
    _allButtonsInterval.reset();
    log(button, "RELEASE");

    if (isButtonNext(button)) {
      if (Shared.state == S_PLAYING || Shared.state == S_PAUSE || Shared.state == S_GOTO_TRACK) {
        Carriage.gotoNextTrack();
      }
    }

    if (isButtonPrev(button)) {
      if (Shared.state == S_PLAYING || Shared.state == S_PAUSE || Shared.state == S_GOTO_TRACK) {
        Carriage.gotoPreviousTrack();
      }
    }

    if (button == BUTTON_PLAY) {
      if (Shared.state == S_PAUSE || Shared.state == S_PLAYING) {
        Carriage.pause();
      }
    }

    //--------------------------------------------- RPM
    if (button == BUTTON_PREV && Shared.state == S_HOME) {
      if (Plateau.rpmMode == RPM_AUTO) {
        Plateau.rpmMode = RPM_33;
      } else if (Plateau.rpmMode == RPM_33) {
        Plateau.rpmMode = RPM_45;
      } else if (Plateau.rpmMode == RPM_45) {
        if (PLATEAU_ENABLE_RPM78) {
          Plateau.rpmMode = RPM_78;
        } else {
          Plateau.rpmMode = RPM_AUTO;
        }
      } else if (Plateau.rpmMode == RPM_78) {
        Plateau.rpmMode = RPM_AUTO;
      }
      Plateau.updateRpm();
      rpmDisplayActionInterval.reset();
    }

    //--------------------------------------------- BLUETOOTH
    if (button == BUTTON_NEXT && Shared.state == S_HOME) {
       LOG_DEBUG("buttons.cpp", "[logic] Bluetooth");
      Bluetooth.write("AT+DELVMLINK");
    }
    return;
  }

  //--------------------------------------------- LONG PRESS
  if (state[button] == BUTTON_PRESS && (millisSinceBoot() - _buttonInterval[button]) > BUTTON_LONG_CLICK) {
    state[button] = BUTTON_LONG_PRESS;
    _allButtonsInterval.reset();

    log(button, "LONG_PRESS");

    // >> FORWARD
    if ((Shared.state == S_PLAYING || Shared.state == S_PAUSE || Shared.state == S_GOTO_TRACK) && isButtonNext(button)) {
      Shared.setState(S_SKIP_FORWARD);
    }

    // << REVERSE
    if ((Shared.state == S_PLAYING || Shared.state == S_PAUSE || Shared.state == S_GOTO_TRACK) && isButtonPrev(button)) {
      Shared.setState(S_SKIP_REVERSE);
    }

    if (button == BUTTON_PLAY && Shared.state != S_HOME) {
      Plateau.stop();
    }

    //--------------------------------------------- BLUETOOTH RESET
    if (button == BUTTON_NEXT && Shared.state == S_HOME) {
      LOG_DEBUG("buttons.cpp", "[logic] Bluetooth reset");
      Bluetooth.write("AT+REST");
    }
    return;
  }

  //--------------------------------------------- LONG RELEASE
  if (state[button] == BUTTON_LONG_PRESS && _buttonIn[button] == BUTTON_RELEASE) {
    state[button] = BUTTON_RELEASE;
    _allButtonsInterval.reset();

    log(button, "LONG_PRESS RELEASE");

    if ((Shared.state == S_SKIP_FORWARD || Shared.state == S_SKIP_REVERSE) &&
      (button == BUTTON_NEXT || button == BUTTON_PREV)) { // Resume after forward/reverse
      Carriage.targetTrack = Carriage.position;
      Shared.setState(S_RESUME_AFTER_SKIP);
    }
    return;
  }

  //--------------------------------------------- SUPER LONG PRESS
  if (state[button] == BUTTON_LONG_PRESS && (millisSinceBoot() - _buttonInterval[button]) > BUTTON_SUPERLONG_CLICK) {
    state[button] = BUTTON_SUPERLONG_PRESS;
    _allButtonsInterval.reset();

    log(button, "SUPERLONG_PRESS");

    if (button == BUTTON_PLAY) {
      if (Shared.state == S_HOMING_BEFORE_PLAYING || Shared.state == S_GOTO_RECORD_START) { // Repeat
        Carriage.repeat = true;
        Serial.println("REPEAT: ON");
      }
    }

    if (Shared.state == S_HOME && button == BUTTON_PREV) { // Clean mode
      Plateau.cleanMode();
      ledBlink();
    }
    return;
  }

  //--------------------------------------------- SUPER LONG RELEASE
  if (state[button] == BUTTON_SUPERLONG_PRESS && _buttonIn[button] == BUTTON_RELEASE) {
    state[button] = BUTTON_RELEASE;
    _allButtonsInterval.reset();

    log(button, "SUPERLONG_PRESS RELEASE");

    if ((Shared.state == S_SKIP_FORWARD || Shared.state == S_SKIP_REVERSE)  &&  
      (button == BUTTON_NEXT || button == BUTTON_PREV)) { // Resume after forward/reverse
      Carriage.targetTrack = Carriage.position;
      Shared.setState(S_RESUME_AFTER_SKIP);
    }
    return;
  }

  if (_buttonIn[button] == BUTTON_RELEASE) { // last check for button release to prevent long press loop
    state[button] = BUTTON_RELEASE;
    return;
  }
} // logic()


void Buttons_::ledBlink() {
  ledBlinkInterval.reset();
} // ledBlink()


bool Buttons_::isButtonNext(int button) {
  return button == buttonNextComp();
} // isButtonNext()


bool Buttons_::isButtonPrev(int button) {
  return button == buttonPrevComp();
} // isButtonPrev()


int Buttons_::buttonNextComp() {
  if (Orientation.isStanding) {
    return BUTTON_PREV;
  }
  return BUTTON_NEXT;
} // buttonNextComp()


int Buttons_::buttonPrevComp() {
  if (Orientation.isStanding) {
    return BUTTON_NEXT;
  }
  return BUTTON_PREV;
} // buttonPrevComp()


void Buttons_::log(int button, String action) {
    LOG_DEBUG("buttons.cpp", "[logic] " + getButton(button) + ": " + action);
  // Serial.println(getButton(button) + ": " + action);
} // log()


String Buttons_::getButton(int button) {
  String strButton = "BUTTON_UNKNOWN";

  if        (button == BUTTON_PLAY) { strButton = "BUTTON_PLAY";
  } else if (button == BUTTON_NEXT) { strButton = "BUTTON_NEXT";
  } else if (button == BUTTON_PREV) { strButton = "BUTTON_PREV";
  }
  return strButton;
} // getButton()


void Buttons_::info() {
  Serial.print(padRight("BUTTONS", PADR) + ": ");
  for (int button = 0; button < BUTTON_COUNT; button++) {
    Serial.print(String(_buttonIn[button]) + " ");
  }
  Serial.println();
  Serial.println();
} // info()


Buttons_ &Buttons_::getInstance() {
  static Buttons_ instance;
  return instance;
} // getInstance()


Buttons_ &Buttons = Buttons.getInstance();