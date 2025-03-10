#include "log.h"
#include "orientation.h"
#include "helper.h"
#include "pins.h"
#include "i2c.h"


void Orientation_::init() {
  LOG_DEBUG("orientation.cpp", "[init]");
  setI2CPins();
} // init()


void Orientation_::update() {
  if (_interval.tick() && millisSinceBoot() > 200) { // turned on for 200ms?
    if (_firstTime) {
      _firstTime = false;
      reset();
      return;
    }

    float sensorRawX = readAccelerationAxis(_i2cAdress, 1);
    // float sensorRawY = readAccelerationAxis(_i2cAdress, 3);
    float sensorRawZ = readAccelerationAxis(_i2cAdress, 5);

    _rawX += (sensorRawX - _rawX) / 10;
    // _rawY += (sensorRawY - _rawY) / 10;
    _rawZ += (sensorRawZ - _rawZ) / 10;

    x += ((_rawX - offsetX) - x) / 10;
    // y += ((_rawY - offsetY) - y) / 10;
    y += (Arm.armAngleCall - y) / 20;
    z += ((_rawZ - offsetZ) - z) / 10;

    if (_error) {
      _error = !isApprox(y, 0, 0.6);
    } else {
      if ((Shared.state == S_HOME) && (Shared.stateChangedInterval.duration() > 1000)) {
        _error = !isApprox(y, 0, 0.8);
      }
    }

    if (_error && !_errorPrev) {
      _errorPrev = _error;
      Shared.setState(S_BAD_ORIENTATION);
    }

    if (!_error) {
      if (_error != _errorPrev) {
        _errorPrev = _error;
        _isOkInterval.reset();
      }
      if (_isOkInterval.duration() > 3000 && Shared.state == S_BAD_ORIENTATION) {
        Shared.setState(S_HOME);
      }
    }

    isStanding = !(isApprox(x, 0, 0.4) && isApprox(z, -1, 0.4));
    if (isStanding != _isStandingPrev) {
      _isStandingPrev = isStanding;
      info();
    }

    if (graphicData) {
      printGraphicData();
    } else {
      _headerShown = false;
    }
  } // _interval.tick()
} // update()


void Orientation_::calibrate() {
  LOG_DEBUG("orientation.cpp", "[calibrate]");
  offsetX += x;
  offsetY += y;
  offsetZ += (z + 1);
  LOG_DEBUG("orientation.cpp", "[update] OffsetX: " + String(offsetX, 5) + " OffsetY: " + String(offsetY, 5) + " OffsetZ: " + String(offsetZ, 5));
} // calibrate()


void Orientation_::reset() {
  LOG_DEBUG("orientation.cpp", "[reset]");
  // reset
  i2cWrite(_i2cAdress, 0x36, 0xB6); // soft reset
  i2cWrite(_i2cAdress, 0x36, 0x00);

  delay(10);

  // set_mode
  uint8_t data = i2cRead(_i2cAdress, 0x11);
  setBit(&data, 7, 1); // 1 = active, 0 = inactive;
  i2cWrite(_i2cAdress, 0x11, data);
} // reset()


void Orientation_::printGraphicData() {
  if (!_headerShown) {
    Serial.println("GRAPH_HEADER: X-axis, Y-axis, Z-axis");
    _headerShown = true;
  }      
  Serial.print(x, 5);
  Serial.print(", ");
  Serial.print(y, 5);
  Serial.print(", ");
  Serial.print(z, 5);
  Serial.println();
}


void Orientation_::info() {
  Serial.println(padRight("ORIENTATION_RAW_XYZ", PADR) +  ": " + "X:" + String(_rawX, 3) + " Y:" + String(_rawY, 3) + " Z:" + String(_rawZ, 3));
  Serial.println(padRight("ORIENTATION_XYZ", PADR) +      ": " + "X:" + String(x, 3) + " Y:" + String(y, 3) + " Z:" + String(z, 3));
  Serial.println(padRight("ORIENTATION_POSITION", PADR) + ": " + String(isStanding ? "STANDING" : "NORMAL"));
  Serial.println();
} // info()


Orientation_ &Orientation_::getInstance() {
  static Orientation_ instance;
  return instance;
} // getInstance()


Orientation_ &Orientation = Orientation.getInstance();
