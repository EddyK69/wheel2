#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include "shared.h"
#include "arm.h"
#include "carriage.h"
#include "orientation.h"

#define EEPROM_VERSION            0
#define EEPROM_ARM_FORCE_500MG    100
#define EEPROM_ARM_FORCE_4000MG   110
#define EEPROM_ARM_TARGETWEIGHT   120
#define EEPROM_ARM_FORCE_DOCKED   130

#define EEPROM_LEVEL_OFFSET_X     200
#define EEPROM_LEVEL_OFFSET_Y     210
#define EEPROM_LEVEL_OFFSET_Z     220

#define EEPROM_TRACK_OFFSET       300

#define EEPROM_ARM_ANGLE_MIN      400
#define EEPROM_ARM_ANGLE_MAX      410

#define EEPROM_PLATEAU_MOTOR_REV  420


class Storage_ {
  private:
    Storage_() = default; // Make constructor private

  public:
    static Storage_& getInstance(); // Accessor for singleton instance

    Storage_(const Storage_&) = delete; // no copying
    Storage_& operator=(const Storage_&) = delete;

  private:
    float _armForceLow = 0;
    float _armForceHigh = 0;
    float _armTargetWeight = 0;
    float _armForceDocked = 0;
    float _levelOffsetX = 0;
    float _levelOffsetY = 0;
    float _levelOffsetZ = 0;
    float _trackOffset = 0;
    float _armAngleMin = 0;
    float _armAngleMax = 0;
    float _plateauMotorReverse = 0;
    void readAddress(int address, float& value);
    void writeAddress(int address, float value);
    void commit();
  public:
    float eepromVersion = 0;
    bool saveRequired = false;
    void init();
    void read();
    void write();
    void info();
}; // Storage_

extern Storage_& Storage;

#endif // STORAGE_H
