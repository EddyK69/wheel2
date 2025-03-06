#ifndef PLATEAU_H
#define PLATEAU_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "speedcomp.h"
#include "enums.h"

#define PLATEAU_RPM33 33.333
#define PLATEAU_RPM45 45
#define PLATEAU_RPM78 78

#define PLATEAU_ENABLE_RPM78 false


class Plateau_ {
  private:
    Plateau_() = default; // Make constructor private

  public:
    static Plateau_& getInstance(); // Accessor for singleton instance

    Plateau_(const Plateau_&) = delete; // no copying
    Plateau_& operator=(const Plateau_&) = delete;

  private:
    Interval _interval = Interval(5000, TM_MICROS);
    float _autoRpm;
    float _basicVoltage = 0;
    float _outBuff;
    float _outBuffPrev;
    float _rpmPrev = 0;
    bool _spinningDown;
    uint64_t _tsMotorOn;
    uint64_t _motorUsed = 0;
    int _playCount7 = 0;
    int _playCount10 = 0;
    int _playCount12 = 0;
    int _playCountOther = 0;
    float pid(float rpmIn);
    void update();
    void startUseCounter();
    void stopUseCounter();
    String getUseCounter();
  public:
    Interval turnInterval = Interval(10, TM_MILLIS);
    float P = 1;    // 0.5
    float I = 0.02; // plateau33I;
    float D = 0;
    float targetRpm = 0;
    eRpmMode rpmMode = RPM_AUTO;
    // int rmpSetRpmMode = 0; // TODO: EK
    bool logic = true;
    bool unbalanceCompensation = true;
    bool atSpeed;
    bool motorOn = false;
    bool motorReverse = false;
    void init();
    void func();
    void motorStart();
    void updateRpm();
    void setRpm(eRpmMode rpm);
    void setPlayCount(eRecordDiameter rd);
    void motorStop();
    void play();
    void stop();
    void cleanMode();
    void info();
}; // Plateau_

extern Plateau_& Plateau;

#endif // PLATEAU_H
