#include "log.h"
#include "carriage.h"
#include "helper.h"
#include "pins.h"
#include "pwm.h"


void Carriage_::init() {
  LOG_DEBUG("carriage.cpp", "[init]");
  setPwm(CARRIAGE_STEPPER_AP_PIN);
  setPwm(CARRIAGE_STEPPER_AN_PIN);
  setPwm(CARRIAGE_STEPPER_BP_PIN);
  setPwm(CARRIAGE_STEPPER_BN_PIN);
} // init()


void Carriage_::func() {
  if (_interval.tick()) {
    //--------------------------------------------- ARM ANGLE
    Arm.armAngleRaw += (analogRead(ARM_ANGLE_SENSOR_PIN) - Arm.armAngleRaw ) / 6;

    if (millisSinceBoot() > 1000) {
        if (Arm.armAngleRaw < Arm.armAngleMinCall) {
          Arm.armAngleMinCall = Arm.armAngleRaw;
        }
        if (Arm.armAngleRaw > Arm.armAngleMaxCall) {
          Arm.armAngleMaxCall = Arm.armAngleRaw;
        }
    }
    Arm.armAngleCall = mapFloat(Arm.armAngleRaw, Arm.armAngleMin, Arm.armAngleMax, 1, -1);
    Arm.armAngleDiff = Arm.armAngleCall - Arm.armAnglePrev;
    Arm.armAnglePrev = Arm.armAngleCall;
    Arm.armAngleSlow += (Arm.armAngleCall - Arm.armAngleSlow) / 100;
    Arm.armAngle = Arm.armAngleCall - Arm.armAngleOffset;

    //--------------------------------------------- LIMIT ERROR
    if (Shared.state == S_PLAYING) {
      if (Arm.armAngleCall > 0.95) {
        Shared.setError(E_ARMANGLE_LIMIT_POS);
        Arm.dockNeedle();
        Shared.setState(S_HOME);
      }
      if (Arm.armAngleCall < -0.95) {
        Shared.setError(E_ARMANGLE_LIMIT_NEG);
        Arm.dockNeedle();
        Shared.setState(S_HOME);
      }
    }

    sensorPosition = (position - CARRIAGE_SENSOR_OFFSET) - trackOffset;

    stateUpdate();

    _Dcomp *= 0.999;
    _Dcomp += limitFloat(Arm.armAngleDiff * D, -CARRIAGE_MAX_SPEED, CARRIAGE_MAX_SPEED); // to prevent oscillation
    realPosition = position + _Dcomp;

    if (offCenterCompensation) {
      _offCenterCompFilter +=  (SpeedComp.carriageFourierFilter - _offCenterCompFilter) / 4;
      realPosition += _offCenterCompFilter;
    }

    _motorPos = (realPosition + _offset) * _mm2step;

    if (_motorEnable) {
      pwmStepper(-_motorPos, CARRIAGE_STEPPER_AP_PIN, CARRIAGE_STEPPER_AN_PIN, CARRIAGE_STEPPER_BP_PIN, CARRIAGE_STEPPER_BN_PIN, true);
    } else {
      pwmDisableStepper(CARRIAGE_STEPPER_AP_PIN, CARRIAGE_STEPPER_AN_PIN, CARRIAGE_STEPPER_BP_PIN, CARRIAGE_STEPPER_BN_PIN);
    }

    if (Shared.state == S_PLAYING && Arm.isNeedleInGrove()) { // carriage pos filter for display
      float div = position - positionFilter;
      if (div < 0) {
        positionFilter += (div) / 1000;
      }
    } else {
      positionFilter = position;
    }

    _motorEnable = true;

    if (graphicData) {
      printGraphicData();
    } else {
      _headerShown = false;
    }
  } // _interval.tick()
} // func()


void Carriage_::stateUpdate() {
  if (Shared.state == S_HOME) {
    Arm.centerArmAngle();
    _motorEnable = false;
    if (repeat) {
      repeat = false;
      Serial.println("REPEAT: OFF");
    }
    return;
  }

  if (Shared.state == S_STOPPING) {
    if (Arm.dockNeedle() && decelerate()) {
      Shared.setState(S_HOMING);
    }
    return;
  }


  if (Shared.state == S_HOMING || Shared.state == S_HOMING_BEFORE_PLAYING || Shared.state == S_HOMING_BEFORE_CLEANING) {
    // wait for stepper to stop shaking after turning on
    if (Shared.stateChangedInterval.duration() < 300) {
      return;
    }

    float speed = mapFloat(Arm.armAngleCall, 0.75, 0.5, 0, CARRIAGE_MAX_SPEED);
    speed = limitFloat(speed, (CARRIAGE_MAX_SPEED / 10), CARRIAGE_MAX_SPEED);
    bool arrived = movetoPosition(-150, speed);
    // bool arrived = movetoPosition(-150, CARRIAGE_MAX_SPEED);

    if (arrived) {
      _offset -= CARRIAGE_PARK - position;
      position = CARRIAGE_PARK;
      
      if (Shared.error == E_HOMING_FAILED){
        Shared.setError(E_HOMING_FAILED);
        Shared.setState(S_PARKING);
      } else {
        Shared.setError(E_HOMING_FAILED);
        Shared.setState(S_HOMING_FAILED);
      }
      return;
    }

    if (Arm.armAngleCall > 0.75) { //75 //-800 //-1000
      // LOG_DEBUG("carriage.cpp", "[stateUpdate] Home diff: " + String(CARRIAGE_PARK - realPosition) + " realPosition: " + String(realPosition) + " CARRIAGE_PARK: " + String(CARRIAGE_PARK) + " Dcomp: " + String(_Dcomp));
      LOG_DEBUG("carriage.cpp", "[stateUpdate] Home diff: " + String(CARRIAGE_PARK - realPosition) + " realPosition: " + String(realPosition));
      // Serial.println("Home diff: " + String(CARRIAGE_PARK - realPosition) + " realPosition: " + String(realPosition));
      
      _offset -= CARRIAGE_PARK - realPosition;
      position = CARRIAGE_PARK;
      _Dcomp = 0;

      emergencyStop();
      SpeedComp.clearCompSamples(); // nice moment to stop

      if (Shared.state == S_HOMING_BEFORE_PLAYING) {
        Shared.setState(S_GOTO_RECORD_START);
      } else if (Shared.state == S_HOMING_BEFORE_CLEANING) {
        Shared.setState(S_NEEDLE_CLEAN);
      } else {
        Shared.setState(S_PARKING);
      }
      return;
    }
    return;
  }


  if (Shared.state == S_PARKING) {
    if (movetoPosition(CARRIAGE_HOME, CARRIAGE_MAX_SPEED)) {
      // if (Shared.stateChangedInterval.duration() < 2000) {
        Shared.setState(S_HOME);
      // }
    }
    _Dcomp = 0;
    // _Dcomp *= 0.98;
    return;
  }


  if (Shared.state == S_HOMING_FAILED) {
    if (movetoPosition(CARRIAGE_HOME + 10, CARRIAGE_MAX_SPEED)) {
      if (Shared.stateChangedInterval.duration() < 2000) {
        Arm.centerArmAngle();
        return;
      }
      Shared.setState(S_HOMING);
    } 
    return;
  }


  if (Shared.state == S_BAD_ORIENTATION) {
    Arm.dockNeedle();
    _motorEnable = false;
    return;
  }


  if (Shared.state == S_GOTO_RECORD_START) {
    if (Shared.firstTimeStateChanged()) {
      Scanner.recordStart = 1000;
    }

    if (!Scanner.recordPresent && sensorPosition > CARRIAGE_RECORD_END + 1) { // record present?
      float recordDiaInch = (sensorPosition / 25.4) * 2;

      if (recordDiaInch < 6) { // stop when smaller than 6"
        // LOG_DEBUG("carriage.cpp", "[stateUpdate] No record? RecordDiameter: " + String(recordDiaInch));
        Serial.println("No record? RecordDiameter: " + String(recordDiaInch));
        Plateau.stop();
        return;
      } else if (recordDiaInch < 9) {
        // LOG_DEBUG("carriage.cpp", "[stateUpdate] RecordDiameter: " + String(recordDiaInch) + " : ±7\" ");
        Serial.println("RecordDiameter: " + String(recordDiaInch) + " : ±7\" ");
        Plateau.setRpm(RPM_45);
        Plateau.setPlayCount(R_7INCH);
        Scanner.recordStart = CARRIAGE_7INCH_START;
        Scanner.setTracksAs7inch();
      } else if (recordDiaInch < 11) { 
        // LOG_DEBUG("carriage.cpp", "[stateUpdate] RecordDiameter: " + String(recordDiaInch) + " : ±10\" ");
        Serial.println("RecordDiameter: " + String(recordDiaInch) + " : ±10\" ");
        Plateau.setRpm(RPM_33); // https://standardvinyl.com/vinyl-pressing/10-inch-records
        Plateau.setPlayCount(R_10INCH);
        Scanner.recordStart = CARRIAGE_10INCH_START;
        Scanner.check();
      } else {
        // LOG_DEBUG("carriage.cpp", "[stateUpdate] RecordDiameter: " + String(recordDiaInch) + " : ???\" ");
        Serial.println("RecordDiameter: " + String(recordDiaInch) + " : ???\" ");
        Scanner.recordStart = sensorPosition;
        // Plateau.setRpm(RPM_33);
        Plateau.setPlayCount(R_OTHER);
      }
      targetTrack = Scanner.recordStart;
      Shared.setState(S_PLAY_TILL_END);
      return;
    }

    // when arrived at carriage endrange
    if (movetoPosition(CARRIAGE_12INCH_START, CARRIAGE_MAX_SPEED)) { 
      Scanner.recordStart = CARRIAGE_12INCH_START;
      targetTrack = Scanner.recordStart;
      // LOG_DEBUG("carriage.cpp", "[stateUpdate] RecordDiameter: 12\" ");
      Serial.println("RecordDiameter: 12\" ");
      Plateau.setRpm(RPM_33);
      Plateau.setPlayCount(R_12INCH);
      Scanner.check();

      Shared.setState(S_PLAYING);
      return;
    }
    return;
  }

  if (Shared.state == S_PLAY_TILL_END) {
    if (movetoPosition(targetTrack, CARRIAGE_MAX_SPEED)) {
      Shared.setState(S_PLAYING);
      return;
    }
    return;
  }


  //  ================================================================
  //      PLAY
  //  ================================================================
  if (Shared.state == S_PLAYING) {
    if (Shared.stateChangedInterval.duration() < 1000) {
      Arm.centerArmAngle();
      movedForwardInterval.reset();
      return;
    }

    // if (Arm.isNeedleDownFor(3000)) { // error 3 fix i think
    //   movedForwardInterval.reset(); // reset timer to prevent error 3
    // }

    if (Arm.putNeedleInGrove()) {
      //---------------------------------------- transport calculations
      _newPosition = position + limitFloat(Arm.armAngle * P, -3, 3);
      _newPosition = limitFloat(_newPosition, 0, Scanner.recordStart);
      movetoPosition(_newPosition, CARRIAGE_MAX_SPEED);
      
      //---------------------------------------- events during playing
      if (realPosition <= CARRIAGE_RECORD_END) {
        // LOG_NOTICE("carriage.cpp", "[stateUpdate] Carriage reached limit!");
        Serial.println("Carriage reached limit!");
        stopOrRepeat();
        return;
      }
      if (realPosition < positionFilter - 3) {
        // LOG_NOTICE("carriage.cpp", "[stateUpdate] Run-out groove?");
        Serial.println("Run-out groove?");
        stopOrRepeat();
        return;
      }
      if (realPosition > positionFilter + 2.5) {
        Shared.setError(E_NEEDLE_MOVE_BACKWARDS);
        Plateau.stop();
        return;
      }

      if (SpeedComp.trackSpacing > 0.01) {
        movedForwardInterval.reset();
      } else if (movedForwardInterval.duration() > 4000) {
        if (position < 60 ){ // run-out groove? (54mm seems the farest from the middle
          stopOrRepeat();
          return;
        } else {
          Shared.setError(E_NEEDLE_DIDNT_MOVE); // carriage didn't move for a while
          movedForwardInterval.reset(); // reset time to prevent another trigger
          gotoTrack(position - 0.25); // move carriage 0.5mm inside to skip the skip
          return;
        }
      } 

      if (Shared.puristMode) {
        if ((SpeedComp.wow < 0.15) || Arm.isNeedleDownFor(10000) ){
          // LOG_NOTICE("carriage.cpp", "[stateUpdate] Seems to runs ok");
          Serial.println("Seems to runs ok");
          Shared.puristMode = false;
          Serial.println("PURIST MODE: OFF");
        // gotoRecordStart();
          gotoTrack(targetTrack);
        }
      }    
    }
    return;
  }


  //  ================================================================
  //      TRACKS & SKIPPING
  //  ================================================================
  if (Shared.state == S_GOTO_TRACK) {
    if (Arm.dockNeedle()) {
      if (movetoPosition(targetTrack, CARRIAGE_MAX_SPEED)) {
        Shared.setState(S_PLAYING);
        return;
      }
    }
    return;
  }

  if (Shared.state == S_SKIP_FORWARD) {
    if (Arm.dockNeedle()) {
      movetoPosition(CARRIAGE_RECORD_END, CARRIAGE_MAX_SPEED / 4);
    }
    targetTrack = position; // to clean display
  }

  if (Shared.state == S_SKIP_REVERSE) {
    if (Arm.dockNeedle()) {
      movetoPosition(Scanner.recordStart, CARRIAGE_MAX_SPEED / 4);
    }
    targetTrack = position; // to clean display
  }

  if (Shared.state == S_RESUME_AFTER_SKIP) {
    if (movetoPosition(targetTrack, CARRIAGE_MAX_SPEED / 4)) { 
      Shared.setState(S_PLAYING);
      return;
    }
    return;
  }

  if (Shared.state == S_PAUSE) {
    if (Arm.dockNeedle()) {
      movetoPosition(targetTrack, CARRIAGE_MAX_SPEED);
    }
    return;
  }

  if (Shared.state == S_NEEDLE_CLEAN) {
    if (movetoPosition(CARRIAGE_CLEAN_POS, CARRIAGE_MAX_SPEED)) {
      Arm.putNeedleInGrove();
    }

    if (sensorPosition > (CARRIAGE_RECORD_END + 2) && sensorPosition < (CARRIAGE_RECORD_END + 12) && Scanner.recordPresent) {
      // record present? stop!
      // LOG_ALERT("carriage.cpp", "[stateUpdate] Cannot clean needle; Record present?");
      Serial.println("Cannot clean needle; Record present? Clean record instead");
      Shared.setState(S_RECORD_CLEAN);
      Plateau.motorStart();
      Plateau.setRpm(RPM_33);
    }
    return;
  }

  if (Shared.state == S_RECORD_CLEAN) {
    decelerate();
    if (!Scanner.recordPresent) {
      // LOG_ALERT("carriage.cpp", "[stateUpdate] Record removed!");
      Serial.println("Record removed!");
      Plateau.stop();
    }
    return;
  }

  if (Shared.state == S_CALIBRATE) {
    if (Shared.stateChangedInterval.duration() < 100) {
      targetTrack = CARRIAGE_CLEAN_POS;
    }
    if (movetoPosition(targetTrack, CARRIAGE_MAX_SPEED)) {
      // Nothing      
    }
    return;
  }
} // stateUpdate()


void Carriage_::gotoNextTrack() {
  LOG_DEBUG("carriage.cpp", "[gotoNextTrack]");

  float pos = positionFilter;

  if (Shared.state == S_GOTO_TRACK) {
    pos = targetTrack;
  }

  int track = Scanner.trackCount - 1;
  if (track <= 0) {
    stopOrRepeat();
    return;
  }

  while ((pos - 2) <= Scanner.tracks[track]) { // 2mm offset to prevent repeating the same track
    track--;
    if (track <= 0) {
      stopOrRepeat();
      return;
    }
  }
  gotoTrack(Scanner.tracks[track]);
} // gotoNextTrack()


void Carriage_::gotoPreviousTrack() {
  LOG_DEBUG("carriage.cpp", "[gotoPreviousTrack]");

  float pos = positionFilter;

  if (Shared.state == S_GOTO_TRACK) {
    pos = targetTrack;
  }

  int track = 0;
  while ((pos + CARRIAGE_BACKTRACK_OFFSET) >= Scanner.tracks[track]) {
    track++;
    if (track > (Scanner.trackCount - 1)) {
      gotoRecordStart();
      return;
    }
  }

  if (Scanner.tracks[track] > Scanner.recordStart) {
    gotoRecordStart();
  } else if (pos < Scanner.tracks[0]) { // When pos is after Record_End, skip to last track
    gotoTrack(Scanner.tracks[1]);
  } else {
    gotoTrack(Scanner.tracks[track]);
  }
} // gotoPreviousTrack()


void Carriage_::gotoTrack(float pos) {
  LOG_DEBUG("carriage.cpp", "[gotoTrack]");
  targetTrack = pos;
  // LOG_DEBUG("carriage.cpp", "[gotoTrack] To position " + String(targetTrack));
  Serial.println("To position " + String(targetTrack));
  Shared.setState(S_GOTO_TRACK);
} // gotoTrack()


void Carriage_::gotoRecordStart() {
  LOG_DEBUG("carriage.cpp", "[gotoRecordStart]");
  gotoTrack(Scanner.recordStart);
} // gotoRecordStart()

bool Carriage_::movetoPosition(float target, float spd) {
  _acceleration = 0;

  float togo = abs(target - position);
  int togoDirection = (target - position) > 0 ? 1 : -1;

  _distanceToStop = (_speed * _speed) / (2 * CARRIAGE_ACCELERATION);
  int _distanceToStopDirection = _speed > 0 ? 1 : -1;

  if (isApprox(togo, 0, 0.01) && _distanceToStop < 0.1) {
    _speed = 0;
    return true;
  }

  if (_distanceToStop >= togo) {
    _acceleration = -CARRIAGE_ACCELERATION * togoDirection;
  } else if (abs(_speed) < CARRIAGE_MAX_SPEED) {
    _acceleration = CARRIAGE_ACCELERATION * togoDirection;
  }

  position += _speed + (_acceleration / 2);
  _speed += _acceleration;
  _speed = limitFloat(_speed, -spd, spd);

  return false;
} // movetoPosition()


void Carriage_::stopOrRepeat() {
  LOG_DEBUG("carriage.cpp", "[stopOrRepeat]");
  if (repeat) {
    gotoRecordStart();
  } else {
    Plateau.stop();
  }
} // stopOrRepeat()


void Carriage_::pause() {
  LOG_DEBUG("carriage.cpp", "[pause]");
  if (Shared.state == S_PLAYING) {
    Shared.setState(S_PAUSE);
    targetTrack = position;
  } else if (Shared.state == S_PAUSE) {
    Shared.setState(S_PLAY_TILL_END);
  }
} // pause()


bool Carriage_::decelerate() {
  int direction = _speed > 0 ? 1 : -1;

  if(abs(_speed) < CARRIAGE_ACCELERATION){
    _speed = 0;
    return true;
  }

  _acceleration = -CARRIAGE_ACCELERATION * direction;
  position += _speed + (_acceleration / 2);
  _speed += _acceleration;
  return false;
} // decelerate()


void Carriage_::emergencyStop() {
  LOG_DEBUG("carriage.cpp", "[emergencyStop]");
  _speed = 0;
} // emergencyStop()


void Carriage_::printGraphicData() {
  if (!_headerShown) {
    Serial.println("GRAPH_HEADER: DComp, ArmAngleRaw, ArmAngleCall");
    _headerShown = true;
  }
  Serial.print(_Dcomp, 5);
  Serial.print(", ");
  Serial.print(Arm.armAngleRaw);
  Serial.print(", ");
  Serial.print(Arm.armAngleCall, 2);
  Serial.println();
} // printGraphicData()


void Carriage_::info() {
  Serial.println(padRight("CARRIAGE_P", PADR) +        ": " + String(P, 5));
  Serial.println(padRight("CARRIAGE_I", PADR) +        ": " + String(I, 5));
  Serial.println(padRight("CARRIAGE_D", PADR) +        ": " + String(D, 5));
  Serial.println(padRight("CARRIAGE_POSITION", PADR) + ": " + String(position));
  Serial.println(padRight("CARRIAGE_REAL_POS", PADR) + ": " + String(realPosition));
  Serial.println(padRight("CARRIAGE_REPEAT", PADR) +   ": " + String(repeat ? "ON" : "OFF"));
  Serial.println();
} // info()


Carriage_ &Carriage_::getInstance() {
  static Carriage_ instance;
  return instance;
} // getInstance()


Carriage_ &Carriage = Carriage.getInstance();
