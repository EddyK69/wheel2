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


Storage::Storage(Shared& shared, Arm& arm, Carriage& carriage, Orientation& orientation, Scanner& scanner) :
  _shared(shared),
  _arm(arm),
  _carriage(carriage),
  _orientation(orientation),
  _scanner(scanner) {
} // Storage()


void Storage::init() {
  LOG_DEBUG("storage.cpp", "[init]");
  EEPROM.begin(4096);
} // init()


void Storage::read() {
  readAddress(EEPROM_VERSION,               eepromVersion);
  readAddress(EEPROM_ARM_FORCE_500MG,       _armForceLow);
  readAddress(EEPROM_ARM_FORCE_4000MG,      _armForceHigh);
  readAddress(EEPROM_ARM_TARGETWEIGHT,      _armTargetWeight);
  readAddress(EEPROM_ARM_FORCE_DOCKED,      _armForceDocked);
  readAddress(EEPROM_LEVEL_OFFSET_X,        _levelOffsetX);
  readAddress(EEPROM_LEVEL_OFFSET_Y,        _levelOffsetY);
  readAddress(EEPROM_LEVEL_OFFSET_Z,        _levelOffsetZ);
  readAddress(EEPROM_TRACK_OFFSET,          _trackOffset);
  readAddress(EEPROM_ARM_ANGLE_MIN,         _armAngleMin);
  readAddress(EEPROM_ARM_ANGLE_MAX,         _armAngleMax);
  readAddress(EEPROM_SCANNER_DETECTION_TH,  _scannerDetectionThreshold);
  readAddress(EEPROM_CARRIAGE_12INCH_START, _carriage12inchStart);
  readAddress(EEPROM_CARRIAGE_10INCH_START, _carriage10inchStart);
  readAddress(EEPROM_CARRIAGE_7INCH_START,  _carriage7inchStart);
  readAddress(EEPROM_CARRIAGE_RECORD_END,   _carriageRecordEnd);

  _arm.forceLow               = _armForceLow;
  _arm.forceHigh              = _armForceHigh;
  // _arm.targetWeight           = _armTargetWeight;
  _arm.justDockedWeight       = _armForceDocked;
  _orientation.offsetX        = _levelOffsetX;
  _orientation.offsetY        = _levelOffsetY;
  _orientation.offsetZ        = _levelOffsetZ;
  _carriage.trackOffset       = _trackOffset;
  _arm.armAngleMin            = _armAngleMin;
  _arm.armAngleMax            = _armAngleMax;
  _scanner.detectionThreshold = _scannerDetectionThreshold;
  _carriage.r12inchStart      = _carriage12inchStart;
  _carriage.r10inchStart      = _carriage10inchStart;
  _carriage.r7inchStart       = _carriage7inchStart;
  _carriage.recordEnd         = _carriageRecordEnd;
} // read()


void Storage::write() {
  eepromVersion              = _shared.version;
  _armForceLow               = _arm.forceLow;
  _armForceHigh              = _arm.forceHigh;
  // _armTargetWeight           = _arm.targetWeight;
  _armForceDocked            = _arm.justDockedWeight;
  _levelOffsetX              = _orientation.offsetX;
  _levelOffsetY              = _orientation.offsetY;
  _levelOffsetZ              = _orientation.offsetZ;
  _trackOffset               = _carriage.trackOffset;
  _armAngleMin               = _arm.armAngleMin;
  _armAngleMax               = _arm.armAngleMax;
  _scannerDetectionThreshold = _scanner.detectionThreshold;
  _carriage12inchStart       = _carriage.r12inchStart;
  _carriage10inchStart       = _carriage.r10inchStart;
  _carriage7inchStart        = _carriage.r7inchStart;
  _carriageRecordEnd         = _carriage.recordEnd;
  writeAddress(EEPROM_VERSION,               eepromVersion);
  writeAddress(EEPROM_ARM_FORCE_500MG,       _armForceLow);
  writeAddress(EEPROM_ARM_FORCE_4000MG,      _armForceHigh);
  writeAddress(EEPROM_ARM_TARGETWEIGHT,      _armTargetWeight);
  writeAddress(EEPROM_ARM_FORCE_DOCKED,      _armForceDocked);
  writeAddress(EEPROM_LEVEL_OFFSET_X,        _levelOffsetX);
  writeAddress(EEPROM_LEVEL_OFFSET_Y,        _levelOffsetY);
  writeAddress(EEPROM_LEVEL_OFFSET_Z,        _levelOffsetZ);
  writeAddress(EEPROM_TRACK_OFFSET,          _trackOffset);
  writeAddress(EEPROM_ARM_ANGLE_MIN,         _armAngleMin);
  writeAddress(EEPROM_ARM_ANGLE_MAX,         _armAngleMax);
  writeAddress(EEPROM_SCANNER_DETECTION_TH,  _scannerDetectionThreshold);
  writeAddress(EEPROM_CARRIAGE_12INCH_START, _carriage12inchStart);
  writeAddress(EEPROM_CARRIAGE_10INCH_START, _carriage10inchStart);
  writeAddress(EEPROM_CARRIAGE_7INCH_START,  _carriage7inchStart);
  writeAddress(EEPROM_CARRIAGE_RECORD_END,   _carriageRecordEnd);

  // saveRequired = false;
  // Save changes to EEPROM
  commit();
} // write()


void Storage::commit() {
  if (EEPROM.commit()) {
    LOG_INFO("storage.cpp", "[commit] EEPROM successfully committed");
    Serial.println("Changes saved to EEPROM");
    saveRequired = false;
  } else {
    LOG_CRITICAL("storage.cpp", "[commit] EEPROM commit failed!");
  }
} // commit()


void Storage::info() {
  int padR = 25;
  Serial.println(padRight("EEPROM_VERSION", padR) +            ": " + String(eepromVersion, 0));
  Serial.println(padRight("EEPROM_ARM_FORCE_500MG", padR) +    ": " + String(_armForceLow, 5));
  Serial.println(padRight("EEPROM_ARM_FORCE_4000MG", padR) +   ": " + String(_armForceHigh, 5));
  // Serial.println(padRight("EEPROM_ARM_TARGETWEIGHT", padR) +   ": " + String(_armTargetWeight, 5));
  Serial.println(padRight("EEPROM_ARM_FORCE_DOCKED", padR) +   ": " + String(_armForceDocked, 5));
  Serial.println(padRight("EEPROM_LEVEL_OFFSET_X", padR) +     ": " + String(_levelOffsetX, 5));
  Serial.println(padRight("EEPROM_LEVEL_OFFSET_Y", padR) +     ": " + String(_levelOffsetY, 5));
  Serial.println(padRight("EEPROM_LEVEL_OFFSET_Z", padR) +     ": " + String(_levelOffsetZ, 5));
  Serial.println(padRight("EEPROM_TRACK_OFFSET", padR) +       ": " + String(_trackOffset, 5));
  Serial.println(padRight("EEPROM_ARM_ANGLE_MIN", padR) +      ": " + String(_armAngleMin, 5));
  Serial.println(padRight("EEPROM_ARM_ANGLE_MAX", padR) +      ": " + String(_armAngleMax, 5));
  Serial.println(padRight("EEPROM_SCAN_DETECTION_TH", padR) +  ": " + String(_scannerDetectionThreshold, 5));
  Serial.println(padRight("EEPROM_CARR_12INCH_START", padR) +  ": " + String(_carriage12inchStart, 5));
  Serial.println(padRight("EEPROM_CARR_10INCH_START", padR) +  ": " + String(_carriage10inchStart, 5));
  Serial.println(padRight("EEPROM_CARR_7INCH_START", padR) +   ": " + String(_carriage7inchStart, 5));
  Serial.println(padRight("EEPROM_CARR_RECORD_END", padR) +    ": " + String(_carriageRecordEnd, 5));
  // Serial.println(padRight("EEPROM_SAVE_REQUIRED", padR) +      ": " + String(saveRequired ? "YES" : "NO"));
  Serial.println();
} // info()


void Storage::readAddress(int address, float& value) {
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


void Storage::writeAddress(int address, float value) {
  EEPROM.put(address, value);
  LOG_DEBUG("storage.cpp", "[writeAddress] Wrote address: " + String(address) + ", value: " + String(value, 5));
} // writeAddress()
