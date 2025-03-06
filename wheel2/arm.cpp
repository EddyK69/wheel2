#include "log.h"
#include "arm.h"
#include "helper.h"
#include "pins.h"
#include "pwm.h"


void Arm_::init() {
  LOG_DEBUG("arm.cpp", "[init]");
  setPwm(ARM_MOTOR_PIN);

  armAngleRaw = analogRead(ARM_ANGLE_SENSOR_PIN);
  armAngleSlow = armAngleRaw;
} // init()


void Arm_::func() {
  if (_interval.tick()) {
    if (Shared.state == S_CALIBRATE) {
      weight = pwm2ArmWeight(force);
      pwmWriteFloat(ARM_MOTOR_PIN, force);
      return;
    }

    if (Shared.state == S_HOMING_BEFORE_PLAYING
      || Shared.state == S_HOMING_BEFORE_CLEANING
      || Shared.state == S_HOMING) {
      if (motorOn) { // motorOn == true
        // LOG_CRITICAL("arm.cpp", "[func] Needle should not have been turned on!");
        Serial.println("Needle should not have been turned on!");
        needleEmergencyStop();
      }
    }

    if (motorOn) { // should the motor be on?
      _motorOffInterval.reset();
      
      if (weight > _justInGroveWeight) { // is the needle already in the groove?
        weight = targetWeight; // put arm on target weight immediately
      } else if (weight < justDockedWeight) {
        weight = justDockedWeight;
      } else {
        weight += (_interval.interval / _speedUp) * (_justInGroveWeight - justDockedWeight);
      }
    } else { // should the motor be off?
      _motorOnInterval.reset();

      if (weight < justDockedWeight) { // is needle up?
        weight = ARM_DOCKED_WEIGHT; // turn off arm immediately
      } else if ( weight > _justInGroveWeight) {
        weight = _justInGroveWeight;
      } else {
        weight -= (_interval.interval / _speedDown) * (_justInGroveWeight - justDockedWeight);
      }
    }

    force = armWeight2Pwm(weight);
    pwmWriteFloat(ARM_MOTOR_PIN, force);

    if (weight != targetWeight) {
      _needleDownInterval.reset();
    }
  } // _interval.tick()
} // func()


bool Arm_::putNeedleInGrove() {
  motorOn = true;
  return isNeedleInGrove();
} // putNeedleInGrove()


bool Arm_::dockNeedle() {
  motorOn = false;
  return isNeedleDocked();
} // dockNeedle()


bool Arm_::needleEmergencyStop() {
  LOG_DEBUG("arm.cpp", "[needleEmergencyStop]");
  weight = ARM_DOCKED_WEIGHT;
  motorOn = false;
  return true;
} // needleEmergencyStop()


bool Arm_::isNeedleInGrove() {
  return weight == targetWeight;
} // isNeedleInGrove()


bool Arm_::isNeedleDocked() {
  return weight == ARM_DOCKED_WEIGHT;
} // isNeedleDocked()


bool Arm_::isNeedleDownFor(int ms) {
  return isNeedleInGrove() && _needleDownInterval.duration() > ms;
} // isNeedleDownFor()


void Arm_::centerArmAngle() {
  // LOG_DEBUG("arm.cpp", "[centerArmAngle]");
  armAngleOffset = armAngleSlow;
} // centerArmAngle


void Arm_::calibrateAngle() {
  LOG_DEBUG("arm.cpp", "[calibrateAngle]");
  armAngleMin = armAngleMinCall;
  armAngleMax = armAngleMaxCall;

  armAngleMinCall = ARM_AMAX;
  armAngleMaxCall = 0;
  // LOG_DEBUG("arm.cpp", "[calibrateAngle] ArmAngle calibrated and buffer values reset. MIN:" + String(armAngleMin) + " MAX:" + String(armAngleMax));
  Serial.println("ArmAngle calibrated and buffer values reset. MIN:" + String(armAngleMin) + " MAX:" + String(armAngleMax));
} // calibrateAngle()


float Arm_::armWeight2Pwm(float weight) {
  float pwm = mapFloat(weight, ARM_MIN_WEIGHT, ARM_MAX_WEIGHT, forceLow, forceHigh);
  return limitFloat(pwm, 0, 1);
} // armWeight2Pwm()


float Arm_::pwm2ArmWeight(float pwm) {
  return mapFloat(pwm, forceLow, forceHigh, ARM_MIN_WEIGHT, ARM_MAX_WEIGHT);
} // pwm2ArmWeight()


void Arm_::info() {
  Serial.println(padRight("ARM_FORCE_LOW", PADR) +     ": " + String(forceLow,  5));
  Serial.println(padRight("ARM_FORCE_HIGH", PADR) +    ": " + String(forceHigh, 5));
  Serial.println(padRight("ARM_FORCE", PADR) +         ": " + String(force,  5));
  Serial.println(padRight("ARM_WEIGHT", PADR) +        ": " + String(weight, 5));
  Serial.println(padRight("ARM_TARGET_WEIGHT", PADR) + ": " + String(targetWeight, 5));
  Serial.println(padRight("ARM_MOTOR", PADR) +         ": " + String(motorOn ? "ON" : "OFF"));
  Serial.println(padRight("NEEDLE", PADR) +            ": " + String(isNeedleInGrove() ? "DOWN" : "UP"));
  Serial.println();
} // info()


Arm_ &Arm_::getInstance() {
  static Arm_ instance;
  return instance;
} // getInstance()


Arm_ &Arm = Arm.getInstance();
