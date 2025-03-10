#include "log.h"
#include "display.h"
#include "pins.h"
#include "helper.h"


void Display_::init() {
  LOG_DEBUG("display.cpp", "[init]");

  pinMode(DISPLAY_IN_PIN,       OUTPUT);
  gpio_set_drive_strength(DISPLAY_IN_PIN, GPIO_DRIVE_STRENGTH_2MA);
  pinMode(DISPLAY_OUT_PIN,      INPUT);
  pinMode(DISPLAY_CLOCK_PIN,    OUTPUT);
  gpio_set_drive_strength(DISPLAY_CLOCK_PIN, GPIO_DRIVE_STRENGTH_2MA);
  pinMode(DISPLAY_LATCH_PIN,    OUTPUT);
  pinMode(DISPLAY_POTMETER_PIN, INPUT);
  pinMode(DISPLAY_EN_PIN,       OUTPUT);
  gpio_set_drive_strength(DISPLAY_EN_PIN, GPIO_DRIVE_STRENGTH_2MA);
  digitalWrite(DISPLAY_EN_PIN, 1);
} // init()


void Display_::update() {
  if (_interval.tick()) {

    _trackCounter = 0;
    int needle = mapRealPos2Display(Carriage.positionFilter);
    int target = mapRealPos2Display(Carriage.targetTrack);
    int sensor = mapRealPos2Display(Carriage.sensorPosition);
    int sensorMaxRange = mapRealPos2Display(CARRIAGE_12INCH_START - CARRIAGE_SENSOR_OFFSET) + 3;
    int recordSize = mapRealPos2Display(Scanner.recordStart);
    int _dispHalf = DISPLAY_LENGTH / 2;

    clear();

    //--------------------------------------------- INTRO
    if (millisSinceBoot() < 4000) {
      int pos = DISPLAY_LENGTH - (millisSinceBoot() / 10);
      int blockLength = 10;
      int spaceLength = 3;

      int decimals = 3;
      int version = Shared.appversion;
      int versionDecimals[decimals] = { (version % 10), (version / 10) % 10, (version / 100) % 10 };

      for (int i = 0; i < decimals; i++) {
        int dec = versionDecimals[i];

        if (dec == 0) { // 0 dot
          int end = pos + spaceLength;
          drawBlock(pos, end, 0.1);
          pos = end + spaceLength;
        }

        while (dec > 0) {
          if (dec >= 5) {
            dec -= 5;
            int end = pos + blockLength * 2; // long line for 5
            drawBlock(pos, end, 0.1);
            pos = end + spaceLength;
          }

          if (dec > 0) {
            dec -= 1;
            int end = pos + blockLength;
            drawBlock(pos, end, 0.1);
            pos = end + spaceLength;
          }
        }
        pos += blockLength; // a pause between every block
      }

    //--------------------------------------------- SHOW ERROR
    } else if (Shared.errorChangedInterval.duration() < 10000 && Shared.error != E_NONE) { // Blink for 10 sec.
      if ((millisSinceBoot() % 1000) < 800) { // Blink
        int blockLength = 0.1 * DISPLAY_LENGTH;
        int begin = _dispHalf - (blockLength / 2) * Shared.error;

        for (int i = 0; i < Shared.error; i++) {
          drawBlock(begin + 1, begin + blockLength - 1, 0.1);
          begin += blockLength;
        }
      }

    //--------------------------------------------- CLEAN MODE
    } else if (Shared.state == S_NEEDLE_CLEAN && Arm.motorOn) {
        int volPoint = mapFloat(Arm.targetWeight, 0, 4, 0, DISPLAY_LENGTH) / 2.0;
        // float pointCounter = (Arm.targetWeight / 2);
        float pointCounter = 0.5;

        for (int i = 0; i < DISPLAY_LENGTH; i++) {
          _data[i] = 0;

          if (i > (_dispHalf - 1) - volPoint && i < _dispHalf + volPoint) {
            _data[i] = 0.1;
          }

          float point = mapFloat(pointCounter, 0, 4, (_dispHalf - 1) - volPoint, ((_dispHalf - 1) - volPoint) + (DISPLAY_LENGTH - 1));
          if (i > point) {
            pointCounter += 0.5;
            if (pointCounter >= 4) {
              pointCounter += 0.5;
            }

            _data[i] = 0;
          }
        }

        if (Orientation.isStanding) {
          flipData();
        }

    //--------------------------------------------- RECORD CLEAN MODE
    } else if (Shared.state == S_RECORD_CLEAN) {
      int rpmPoint = mapFloat(SpeedComp.speed - Plateau.targetRpm, 10, -10, 0, DISPLAY_LENGTH - 1);
      drawBlock(rpmPoint - 2, rpmPoint + 2, 0.9);

      if (!Orientation.isStanding) {
        flipData();
      }

    //--------------------------------------------- CALIBRATE
    } else if (Shared.state == S_CALIBRATE) {
        int volPoint = mapFloat(Arm.force, 0, 1, DISPLAY_LENGTH - 1, 0);
        int forceLowPoint = mapFloat(Arm.forceLow, 0, 1, DISPLAY_LENGTH - 1, 0);
        int forceHighPoint = mapFloat(Arm.forceHigh, 0, 1, DISPLAY_LENGTH - 1, 0);
        int armAnglePoint = mapFloat(Arm.armAngleCall, 1, -1, 0, DISPLAY_LENGTH - 1);

        drawBlock(DISPLAY_LENGTH, volPoint, 0.1);
        drawPoint(armAnglePoint, 0.9);
        drawPoint(forceLowPoint, 0.9);
        drawPoint(forceHighPoint, 0.9);

    //--------------------------------------------- LEVEL MODE
    } else if (Shared.state == S_BAD_ORIENTATION) {
      if ((millisSinceBoot() % 1000) < 800) {
        drawBlock(0, DISPLAY_LENGTH / 4, 0.1);
        drawBlock(DISPLAY_LENGTH - (DISPLAY_LENGTH / 4), DISPLAY_LENGTH, 0.1);
      }

      int armAnglePoint = mapFloat(-Orientation.y, 1, -1, 0, DISPLAY_LENGTH-1);

      drawBlock(armAnglePoint - 2, armAnglePoint + 2, 0.9);

    //--------------------------------------------- RPM
    } else if (Buttons.rpmDisplayActionInterval.duration() < 2000) {

        int blocks = 1; // rpmMode == AUTO

        if (Plateau.rpmMode == RPM_33) {
          blocks = 3;
        } else if(Plateau.rpmMode == RPM_45) {
          blocks = 4;
        } else if(Plateau.rpmMode == RPM_78) {
          blocks = 7;
        }

        int blockLength = 0.1 * DISPLAY_LENGTH;
        int begin = _dispHalf - (blockLength / 2) * blocks;

        for (int i = 0; i < blocks; i++) {
          drawBlock(begin + 1, begin + blockLength - 1, 0.1);
          begin += blockLength;
        }  

    //--------------------------------------------- VOLUME
    } else if (Buttons.volumeDisplayActionInterval.duration() < 2000
        && Shared.state != S_SKIP_FORWARD && Shared.state != S_SKIP_REVERSE
        &&  Shared.state != S_GOTO_TRACK && Shared.state != S_PAUSE) {
      int volPoint = mapFloat(Amplifier.volume, 0, 63, 1, _dispHalf);
      drawBlock(_dispHalf + volPoint, _dispHalf - volPoint, 0.1);

    //--------------------------------------------- TRACK & CARRIAGE DISPLAY
    } else {
      for (int i = 0; i < DISPLAY_LENGTH; i++) {
        if (!(Shared.state == S_STOPPING || Shared.state == S_PARKING || Shared.state == S_HOMING || Shared.state == S_HOME )) {
          int nextTrack = mapRealPos2Display(Scanner.tracks[_trackCounter]);

          if (i > recordSize) {
            _data[i] = 0;
          } else if (Shared.state == S_GOTO_RECORD_START && i > sensor && Scanner.recordStart == 1000) {
            _data[i] = 0;
          } else if (i > sensorMaxRange) {
            _data[i] = 0;
          } else if (nextTrack <= i && _trackCounter < Scanner.trackCount) {
            _trackCounter++;
            _data[i] = 0;
          } else {
            if (_trackCounter == 0) {
              _data[i] = 0;
            } else {
              _data[i] = 0.1;
            }
          }          
        }
      }  


      //--------------------------------------------- CURSOR
      if (Shared.state == S_GOTO_TRACK || Shared.state ==  S_SKIP_FORWARD || Shared.state == S_SKIP_REVERSE || Shared.state == S_PAUSE) { 
        drawPoint((needle - 1), 0.9);
        drawPoint((needle + 1), 0.9);
        
        if (needle != target) { // target dot
          drawPoint(target, 0.9);
        }
      } else if (Shared.state == S_PLAYING && !Arm.isNeedleInGrove() && (Shared.stateChangedInterval.duration() % 1000 < 250) && !Shared.puristMode) {
        // Nothing
      } else if (Carriage.repeat) {
        drawPoint(needle, 0.9);
        drawPoint((needle - 2), 0.9);
        drawPoint((needle + 2), 0.9);
      } else if (Shared.puristMode && (millisSinceBoot() % 1000 < 500)) {
        drawPoint((needle - 1), 0.9);
        drawPoint((needle + 1), 0.9);
      } else {
        drawPoint(needle, 0.9);
      }

      if (!Orientation.isStanding) {
        flipData();
      }
    }

    if (Storage.saveRequired) { // keep blinking when EEPROM is not saved yet
      if (millisSinceBoot() % 1000 > 500) {
        drawBlock(0, (DISPLAY_LENGTH / 20), 0.9);
        drawBlock(DISPLAY_LENGTH, DISPLAY_LENGTH - DISPLAY_LENGTH / 20, 0.9);
      }
    }

    //--------------------------------------------- BUTTON BLINK
    if (Buttons.ledBlinkInterval.duration() < 100) {
      int buttonSize = 6;
      int buttonSizeHalf = buttonSize / 2;
      int buttonOffMiddle = 0.23 * DISPLAY_LENGTH;

      if (Buttons.state[BUTTON_PLAY] != BUTTON_RELEASE) {
        drawBlock(_dispHalf - buttonSizeHalf, _dispHalf + buttonSizeHalf, 0.9);
      }
      if (Buttons.state[BUTTON_PREV] != BUTTON_RELEASE) {
        drawBlock(_dispHalf - buttonOffMiddle, _dispHalf - buttonOffMiddle - buttonSize, 0.9);
      }
      if (Buttons.state[BUTTON_NEXT] != BUTTON_RELEASE) {
        drawBlock(_dispHalf + buttonOffMiddle, _dispHalf + buttonOffMiddle + buttonSize, 0.9);
      }
    }

    print(0);
    commit();

    _delay = microsSinceBoot();
    while(microsSinceBoot() - _delay < 2) {
    }
    digitalWrite(DISPLAY_EN_PIN, 0);
    while(microsSinceBoot() - _delay < 200) {
    }
    digitalWrite(DISPLAY_EN_PIN, 1);

    print(0.5);
    commit();

    digitalWrite(DISPLAY_EN_PIN, 0);
    delayMicroseconds(1500);
    digitalWrite(DISPLAY_EN_PIN, 1);
  } // _interval.tick()
} // update()


void Display_::clear() {
  for (int i = 0; i < DISPLAY_LENGTH; i++) {
    _data[i] = 0;
  }
} // clear()


int Display_::mapRealPos2Display(float pos) {
  return mapFloat(pos, CARRIAGE_RECORD_END, CARRIAGE_12INCH_START, 0, DISPLAY_LENGTH - 1);
} // mapRealPos2Display()


void Display_::drawBlock(int start, int end, float color) {
  int startLim = min(start, end);
  int endLim = max(start, end);

  // Use min and max to ensure the range of 'i' is always between '0' and 'DISPLAY_LENGTH'.
  startLim = max(startLim, 0);
  endLim = min(endLim, DISPLAY_LENGTH);

  if (startLim < 0 || startLim > DISPLAY_LENGTH || endLim < 0 || endLim > DISPLAY_LENGTH) {
    return;
  }

  for (int i = startLim; i < endLim; i++) {
    _data[i] = color;
  }  
} // drawBlock()


void Display_::drawPoint(int pos, float color) {
  if (pos < 0 || pos >= DISPLAY_LENGTH) {
    return;
  }
  _data[pos] = color;
} // drawPoint()


void Display_::flipData() {
  float buffer;
  for (int i = 0; i < DISPLAY_LENGTH / 2; i++) {
    buffer = _data[i];
    _data[i] = _data[(DISPLAY_LENGTH - 1) - i];
    _data[(DISPLAY_LENGTH - 1) - i] = buffer;
  }
} // flipData()


void Display_::print(float time) {
  for (int i = 0; i < DISPLAY_LENGTH; i++) {
    int pix = i;
    pix = (pix + 7) - ((pix % 8) * 2); // flip byte

    gpio_put(DISPLAY_IN_PIN, _data[pix] > time ? 1 : 0);
    delayMicroseconds(2);

    gpio_put(DISPLAY_CLOCK_PIN, 1);
    delayMicroseconds(2);
    gpio_put(DISPLAY_CLOCK_PIN, 0);
  }
} // print()


void Display_::commit() {
  gpio_put(DISPLAY_LATCH_PIN, 0);
  delayMicroseconds(2);
  gpio_put(DISPLAY_LATCH_PIN, 1);
} // commit()


void Display_::bootLED() {
  digitalWrite(LED_PIN, secsSinceBoot() < 3); // turn LED on
}


Display_ &Display_::getInstance() {
  static Display_ instance;
  return instance;
} // getInstance()


Display_ &Display = Display.getInstance();
