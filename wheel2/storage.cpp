#include "log.h"
#include "storage.h"
#include <EEPROM.h>
#include "helper.h"

/**
  https://arduino-pico.readthedocs.io/en/latest/eeprom.html
    "While the Raspberry Pi Pico RP2040 does not come with an EEPROM onboard,
    we simulate one by using a single 4K chunk of flash at the end of flash space."

  EEPROM.begin(4096);
  EEPROM.commit();
*/


void Storage_::init() {
  LOG_DEBUG("storage.cpp", "[init]");
  EEPROM.begin(4096);
} // init()


void Storage_::read() {
  readAddress(EEPROM_VERSION,           eepromVersion);
  readAddress(EEPROM_ARM_FORCE_500MG,   _armForceLow);
  readAddress(EEPROM_ARM_FORCE_4000MG,  _armForceHigh);
  readAddress(EEPROM_ARM_TARGETWEIGHT,  _armTargetWeight);
  readAddress(EEPROM_ARM_FORCE_DOCKED,  _armForceDocked);
  readAddress(EEPROM_LEVEL_OFFSET_X,    _levelOffsetX);
  readAddress(EEPROM_LEVEL_OFFSET_Y,    _levelOffsetY);
  readAddress(EEPROM_LEVEL_OFFSET_Z,    _levelOffsetZ);
  readAddress(EEPROM_TRACK_OFFSET,      _trackOffset);
  readAddress(EEPROM_ARM_ANGLE_MIN,     _armAngleMin);
  readAddress(EEPROM_ARM_ANGLE_MAX,     _armAngleMax);
  readAddress(EEPROM_PLATEAU_MOTOR_REV, _plateauMotorReverse);
  Arm.forceLow          = _armForceLow;
  Arm.forceHigh         = _armForceHigh;
  // Arm.targetWeight      = _armTargetWeight;
  Arm.justDockedWeight  = _armForceDocked;
  Orientation.offsetX   = _levelOffsetX;
  Orientation.offsetY   = _levelOffsetY;
  Orientation.offsetZ   = _levelOffsetZ;
  Carriage.trackOffset  = _trackOffset;
  Arm.armAngleMin       = _armAngleMin;
  Arm.armAngleMax       = _armAngleMax;
  Plateau.motorReverse  = _plateauMotorReverse;
} // read()


void Storage_::write() {
  eepromVersion        = Shared.appversion;
  _armForceLow         = Arm.forceLow;
  _armForceHigh        = Arm.forceHigh;
  // _armTargetWeight     = Arm.targetWeight;
  _armForceDocked      = Arm.justDockedWeight;
  _levelOffsetX        = Orientation.offsetX;
  _levelOffsetY        = Orientation.offsetY;
  _levelOffsetZ        = Orientation.offsetZ;
  _trackOffset         = Carriage.trackOffset;
  _armAngleMin         = Arm.armAngleMin;
  _armAngleMax         = Arm.armAngleMax;
  _plateauMotorReverse = static_cast<float>(Plateau.motorReverse);
  writeAddress(EEPROM_VERSION,           eepromVersion);
  writeAddress(EEPROM_ARM_FORCE_500MG,   _armForceLow);
  writeAddress(EEPROM_ARM_FORCE_4000MG,  _armForceHigh);
  writeAddress(EEPROM_ARM_TARGETWEIGHT,  _armTargetWeight);
  writeAddress(EEPROM_ARM_FORCE_DOCKED,  _armForceDocked);
  writeAddress(EEPROM_LEVEL_OFFSET_X,    _levelOffsetX);
  writeAddress(EEPROM_LEVEL_OFFSET_Y,    _levelOffsetY);
  writeAddress(EEPROM_LEVEL_OFFSET_Z,    _levelOffsetZ);
  writeAddress(EEPROM_TRACK_OFFSET,      _trackOffset);
  writeAddress(EEPROM_ARM_ANGLE_MIN,     _armAngleMin);
  writeAddress(EEPROM_ARM_ANGLE_MAX,     _armAngleMax);
  writeAddress(EEPROM_PLATEAU_MOTOR_REV, _plateauMotorReverse);

  // saveRequired = false;
  // Save changes to EEPROM
  commit();
} // write()


void Storage_::commit() {
  if (EEPROM.commit()) {
    LOG_INFO("storage.cpp", "[commit] EEPROM successfully committed");
    Serial.println("Changes saved to EEPROM");
    saveRequired = false;
  } else {
    LOG_CRITICAL("storage.cpp", "[commit] EEPROM commit failed!");
  }
} // commit()


void Storage_::info() {
  Serial.println(padRight("EEPROM_VERSION", PADR) +          ": " + String(eepromVersion, 0));
  Serial.println(padRight("EEPROM_ARM_FORCE_500MG", PADR) +  ": " + String(_armForceLow, 5));
  Serial.println(padRight("EEPROM_ARM_FORCE_4000MG", PADR) + ": " + String(_armForceHigh, 5));
  // Serial.println(padRight("EEPROM_ARM_TARGETWEIGHT", PADR) + ": " + String(_armTargetWeight, 5));
  Serial.println(padRight("EEPROM_ARM_FORCE_DOCKED", PADR) + ": " + String(_armForceDocked, 5));
  Serial.println(padRight("EEPROM_LEVEL_OFFSET_X", PADR) +   ": " + String(_levelOffsetX, 5));
  Serial.println(padRight("EEPROM_LEVEL_OFFSET_Y", PADR) +   ": " + String(_levelOffsetY, 5));
  Serial.println(padRight("EEPROM_LEVEL_OFFSET_Z", PADR) +   ": " + String(_levelOffsetZ, 5));
  Serial.println(padRight("EEPROM_TRACK_OFFSET", PADR) +     ": " + String(_trackOffset, 5));
  Serial.println(padRight("EEPROM_ARM_ANGLE_MIN", PADR) +    ": " + String(_armAngleMin, 5));
  Serial.println(padRight("EEPROM_ARM_ANGLE_MAX", PADR) +    ": " + String(_armAngleMax, 5));
  // Serial.println(padRight("EEPROM_SAVE_REQUIRED", PADR) +    ": " + String(saveRequired ? "YES" : "NO"));
  Serial.println();
} // info()


void Storage_::readAddress(int address, float& value) {
  float buffer = 0;
  buffer = EEPROM.get(address, buffer);

  if (isfinite(buffer)) {
    value = buffer;
    LOG_DEBUG("storage.cpp", "[readAddress] Read address: " + String(address) + ", value: " + String(value, 5));
  } else {
    LOG_NOTICE("storage.cpp", "[readAddress] Address: " + String(address) + " was still empty, so write it with value: " + String(value, 5));
    writeAddress(address, value);
  }
} // readAddress()


void Storage_::writeAddress(int address, float value) {
  EEPROM.put(address, value);
  LOG_DEBUG("storage.cpp", "[writeAddress] Wrote address: " + String(address) + ", value: " + String(value, 5));
} // writeAddress()


Storage_ &Storage_::getInstance() {
  static Storage_ instance;
  return instance;
} // getInstance()


Storage_ &Storage = Storage.getInstance();
