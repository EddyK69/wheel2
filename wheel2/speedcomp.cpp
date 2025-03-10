#include "log.h"
#include "speedcomp.h"
#include "carriage.h"
#include "pins.h"
#include "helper.h"
#include "plateau.h"


void SpeedComp_::init() {
  LOG_DEBUG("speedcomp.cpp", "[init]");
  float radialCounter;
  clearSamples();
  clearCompSamples();

  for (int i = 0; i < pulsesPerRev; i++) {
    radialCounter = (i * SPEEDCOMP_TAU) / pulsesPerRev;
    _sinus[i] = sin(radialCounter);
    _cosin[i] = cos(radialCounter);
  }

  createUnbalanceFilterCurve();
} // init()


void SpeedComp_::update() {
  if ((microsSinceBoot() - _speedInterval) > SPEEDCOMP_SAMPLES_MAX ) {
    if (_glitchCounter > 3) {
      shiftSamples(SPEEDCOMP_SAMPLES_MAX * _direction);
      speedRaw = 0;
      speed += (speedRaw - speed) / 10;
      speedLowPass += (speed - speedLowPass) / 100;
    } else {
      _glitchCounter++;
    }
  } else {
    _glitchCounter = 0;
  }
} // update()


void SpeedComp_::stroboInterrupt() {
  _time = microsSinceBoot();
  // _processTime = microsSinceBoot();

  //------------------------------------------------------------ DIRECTION
  _direction = 1;
  _sens = (gpio_get(PLATEAU_A_PIN) << 1 ) | gpio_get(PLATEAU_B_PIN);
  if (_sens == 0b00 && _sensPrev == 0b01 ||
      _sens == 0b01 && _sensPrev == 0b11 ||
      _sens == 0b11 && _sensPrev == 0b10 ||
      _sens == 0b10 && _sensPrev == 0b00) {
    _direction = -1;
  }
  _sensPrev = _sens;

  // Serial.println(String(_direction == 1 ? "Anti-Clockwise" : "Clockwise"));
  if (_directionPrev != _direction) {
    // LOG_DEBUG("speedcomp.cpp", "[stroboInterrupt] Direction changed: " + String(_direction == 1 ? "CW -> ACW" : "ACW -> CW"));
    clearUnbalanceCompSamples();
  }
  _directionPrev = _direction;


  //------------------------------------------------------------ SPEED
  _interval = _time - _speedInterval;
  _speedInterval = _time;

  if (_interval > SPEEDCOMP_SAMPLES_MAX) {
    _interval = SPEEDCOMP_SAMPLES_MAX;
  }

  _counterRaw += _direction;
  _counterSinceReset += _direction;
  rotationPosition = roundTrip(rotationPosition + _direction, pulsesPerRev);

  shiftSamples(_interval);
  getSpeed();
  speed += (speedRaw - speed) / 10;
  // _processInterval = microsSinceBoot() - _processTime;

  if (rotationPosition == 0) { // one rotation
    if (_clearCompSamplesQueue) { // T = 0, Comp reset
      _clearCompSamplesQueue = false;
      _counterSinceReset = 0;
      clearUnbalanceCompSamples();
    }
  }


  //------------------------------------------------------------ OFF CENTER COMPENSATION
  _carriagePosMiddlePre -= _carriageOffCenterWave[rotationPosition];
  _carriageOffCenterWave[rotationPosition] = Carriage.realPosition;
  _carriagePosMiddlePre += _carriageOffCenterWave[rotationPosition];
  carriagePosMiddle = _carriagePosMiddlePre / pulsesPerRev;

  trackSpacing = _carriagePosCenterHist[rotationPosition] - carriagePosMiddle;
  _carriagePosCenterHist[rotationPosition] = carriagePosMiddle;

  // if (trackSpacing > 0.01 || !Arm.isNeedleDownFor(2000)) {
  //   Carriage.movedForwardInterval.reset();
  // } else {
  //   // Nothing
  // }

  float carriagePosOffCenter = Carriage.realPosition - carriagePosMiddle;

  if (Arm.isNeedleDownFor(1000) && Shared.state == S_PLAYING) { // needle has to be down while playing before calculation
    _carriageSin -= _carriageSinValues[rotationPosition];
    _carriageSinValues[rotationPosition] = _sinus[rotationPosition] * carriagePosOffCenter;
    _carriageSin += _carriageSinValues[rotationPosition];
    
    _carriageCos -= _carriageCosValues[rotationPosition];
    _carriageCosValues[rotationPosition] = _cosin[rotationPosition] * carriagePosOffCenter;
    _carriageCos += _carriageCosValues[rotationPosition];

    _carriageSinFilt += (_carriageSin - _carriageSinFilt) / 2000;
    _carriageCosFilt += (_carriageCos - _carriageCosFilt) / 2000;
  }

  carriageFourier  = ( ( (_sinus[rotationPosition] * _carriageSin) + ( _cosin[rotationPosition] * _carriageCos) ) / pulsesPerRev ) * 2;
  carriageFourierFilter  = ( ( ( _sinus[rotationPosition] * _carriageSinFilt )  +  ( _cosin[rotationPosition] * _carriageCosFilt ) ) / pulsesPerRev) * 2;
  

  //------------------------------------------------------------ too big break-out ERROR
  float sinBuff = _carriageSinFilt / pulsesPerRev;
  float cosBuff = _carriageCosFilt / pulsesPerRev;
  // an off-center of 6mm (3mm radius) triggers error
  if ((sinBuff * sinBuff + cosBuff * cosBuff) > (3 * 3)) {
    Shared.setError(E_TO_MUCH_TRAVEL);
    clearCompSamples();
    Plateau.stop();
  }


  //------------------------------------------------------------ COMP SPEEDS
  // phase shift: 8 of 16 samples avg filter, and 9 from found filter
  int leadCounter = roundTrip(rotationPosition - (8+9), pulsesPerRev);
  float offCenterSpeedComp = ( ( (_sinus[leadCounter] * _carriageSinFilt) + (_cosin[leadCounter] * _carriageCosFilt) ) / pulsesPerRev) * 2;

  _centerComp = ((carriagePosMiddle - offCenterSpeedComp) / carriagePosMiddle);
  centerCompTargetRpm = Plateau.targetRpm * _centerComp;
  
  if (recordOffCenterComp) {
    speedCenterComp = speed / _centerComp;
  } else {
    speedCenterComp = speed;
  }


  //------------------------------------------------------------ WOW FLUTTER MEASUREMENTS
  speedLowPass += (speedCenterComp - speedLowPass) / 100;
  speedHighPass = speedCenterComp - speedLowPass;

  lowpassRect = abs(speedLowPass - Plateau.targetRpm);
  if (lowpassRect > wow) {
    wow = lowpassRect;
  } else {
    wow += (lowpassRect - wow) / 1000;
  }

  if (wow < 0.1 && _wowFirstLow) { // _wowFirstLow == true
    _wowFirstLow = false;
    // LOG_DEBUG("speedcomp.cpp", "[stroboInterrupt] Runs synchrone again after " + String(_counterSinceReset / float(pulsesPerRev)) + " turns");
    // LOG_DEBUG("speedcomp.cpp", "[stroboInterrupt] Unbalance Phase       : " + String(unbalancePhase));
    // LOG_DEBUG("speedcomp.cpp", "[stroboInterrupt] Unbalance Comp Weight : " + String(unbalanceCompWeight));
    // LOG_DEBUG("speedcomp.cpp", "[stroboInterrupt] Unbalance Filter Width: " + String(unbalanceFilterWidth));
    _counterSinceReset = 0;
  }

  if (wow > 0.3 && !_wowFirstLow) { // _wowFirstLow == false
    _wowFirstLow = true;
  }


  //------------------------------------------------------------ UNBALANCE COMPENSATION
  if (unbalanceCompOn                      // all prereqs when compensation should be off
      && Plateau.motorOn 
      && Plateau.turnInterval.duration() > 1000 // should be on for 1 sec.
      && Plateau.atSpeed                        // and speeded up
      && isApprox(speed, Plateau.targetRpm, 10) // not more than 10rpm from target rpm
      && ((Arm.isNeedleDownFor(2000) && Shared.state == S_PLAYING) ||
      Shared.state == S_HOMING_BEFORE_PLAYING ||    
      Shared.state == S_GOTO_RECORD_START)) { 
    
    int speedError = (speedCenterComp - Plateau.targetRpm) * 1000.0;
    int value;
    for (int i = 0; i < _unbalanceFilterCurveWidth; i++) {
      value = _unbalanceFilterCurve[i] * speedError;
      _unbalansComp[(rotationPosition + 1 + i) % pulsesPerRev] += value;
      _unbalansComp[(rotationPosition + pulsesPerRev - i) % pulsesPerRev] += value;
    }
    // digitalWrite(LED_PIN, 1); // turn led on
  } else {
    // digitalWrite(LED_PIN, 0); // turn led off
  }

  if (unbalanceCompOn) {
    unbalanceComp = - (_unbalansComp[roundTrip(rotationPosition + unbalancePhase, pulsesPerRev)] / (100000000.0)) * unbalanceCompWeight;
  } else {
    unbalanceComp = 0;
  }

  if (graphicData) {
    printGraphicData();
  } else {
    _headerShown = false;
  }
} // stroboInterrupt()


void SpeedComp_::clearCompSamplesOnT0() {
  LOG_DEBUG("speedcomp.cpp", "[clearCompSamplesOnT0]");
  _clearCompSamplesQueue = true;
} // clearCompSamplesOnT0()


void SpeedComp_::clearSamples() {
  LOG_DEBUG("speedcomp.cpp", "[clearSamples]");
  for (int i = 0; i < samples; i++) {
    _samplesArr[i] = SPEEDCOMP_SAMPLES_MAX;
  }
} // clearSamples()


void SpeedComp_::clearCompSamples() {
  LOG_DEBUG("speedcomp.cpp", "[clearCompSamples]");
  clearUnbalanceCompSamples();
  clearCenterCompSamples();
} // clearCompSamples()


void SpeedComp_::clearUnbalanceCompSamples() {
  // LOG_DEBUG("speedcomp.cpp", "[clearUnbalanceCompSamples]");
  for (int i = 0; i < pulsesPerRev; i++) {
    _unbalansComp[i] = 0;
  }
} // clearUnbalanceCompSamples()


void SpeedComp_::clearCenterCompSamples() {
  LOG_DEBUG("speedcomp.cpp", "[clearCenterCompSamples]");
  float pos = Carriage.realPosition;

  for (int i = 0; i < pulsesPerRev; i++) {
    _carriageSinValues[i] = 0;
    _carriageCosValues[i] = 0;
    _carriageOffCenterWave[i] = pos;
    _carriagePosCenterHist[i] = pos;
  }

  _carriageSinFilt = 0;
  _carriageCosFilt = 0;
  _carriageSin = 0;
  _carriageCos = 0;

  _carriagePosMiddlePre = pos * pulsesPerRev;
  carriagePosMiddle = pos;
} // clearCenterCompSamples()


void SpeedComp_::createUnbalanceFilterCurve(){
  LOG_DEBUG("speedcomp.cpp", "[createUnbalanceFilterCurve]");

  float total = 0;
  _unbalanceFilterCurveWidth = pulsesPerRev / 2;

  for (int i = 0; i < pulsesPerRev / 2; i++) {
    float j = float(i) / pulsesPerRev;
    float value = exp(-unbalanceFilterWidth * (j*j));

    if (value > 0.01) {
      _unbalanceFilterCurve[i] = value * 1000;
    } else {
      if ( _unbalanceFilterCurveWidth > i ) {
        _unbalanceFilterCurveWidth = i;
      }
    }
  }
} // createUnbalanceFilterCurve()


float SpeedComp_::getSpeed() {
  _average = averageInterval();
  speedRaw = currentSpeed(_average) * _direction;
  return speedRaw; // don't compensate
} // getSpeed()


float SpeedComp_::averageInterval() {
  int total = 0;

  for (byte i = 0; i < SPEEDCOMP_SAMPLES; i++) {
    total += _samplesArr[i];
  }
  return total / float(SPEEDCOMP_SAMPLES);
} // averageInterval()


float SpeedComp_::currentSpeed(float inter) { // Calculate rpm
  float value = ((1000000.0 * 60) / inter) / pulsesPerRev; // return total
  return limitFloat(value, -300, 300);
} // currentSpeed()


void SpeedComp_::shiftSamples(int sample) {
  _samplesArr[_sampleCounter++ % samples] = sample;
} // shiftSamples()


void SpeedComp_::printGraphicData() {
  if (!_headerShown) {
    Serial.println("GRAPH_HEADER: SpeedRaw, Speed, CarriageFourier");
    _headerShown = true;
  }
  Serial.print(speedRaw, 3);
  Serial.print(",");
  Serial.print(speed, 3);
  Serial.print(",");
  Serial.print(carriageFourier, 3);
  Serial.println();
}


void SpeedComp_::info() {
  // Serial.println(padRight("STROBO_SAMPLES", PADR) +            ": " + String(samples));
  // Serial.println(padRight("STROBO_PULSES_PER_REV", PADR) +     ": " + String(pulsesPerRev));
  Serial.println(padRight("STROBO_UNBAL_PHASE", PADR) +        ": " + String(unbalancePhase));
  Serial.println(padRight("STROBO_UNBAL_COMP_WEIGHT", PADR) +  ": " + String(unbalanceCompWeight));
  Serial.println(padRight("STROBO_UNBAL_FILT_WIDTH", PADR) +   ": " + String(unbalanceFilterWidth));
  Serial.println(padRight("STROBO_UNBAL_FILT_CURVE_W", PADR) + ": " + String(_unbalanceFilterCurveWidth));

  Serial.println();
} // info()


SpeedComp_ &SpeedComp_::getInstance() {
  static SpeedComp_ instance;
  return instance;
} // getInstance()


SpeedComp_ &SpeedComp = SpeedComp.getInstance();
