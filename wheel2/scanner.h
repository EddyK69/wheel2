#ifndef SCANNER_H
#define SCANNER_H

#include <Arduino.h>
#include "interval.h"
#include "shared.h"
#include "plateau.h"

#define SCANNER_DETECTION_THRESHOLD 150 //200


class Scanner_ {
  private:
    Scanner_() = default; // Make constructor private

  public:
    static Scanner_& getInstance(); // Accessor for singleton instance

    Scanner_(const Scanner_&) = delete; // no copying
    Scanner_& operator=(const Scanner_&) = delete;

  private:
    Interval _interval = Interval(10000, TM_MICROS);
    bool _cut;
    bool _trackBelowThreshold = true;
    bool _headerShown = false;
    float _recordPresentFiltered = 0;
    float _raw;
    float _rawPrev;
    float _rawDiff;
    float _absDiff;
    float _value;
    float _valuePrev;
    float _diff;
    float _currentP = 0.0006;
    float _sensorTarget = 1500;
    float _buffer[2000][2];
    int _bufferCounter = 0;
    int _bufferLength = 0;
    int _trackThreshold = 500;
    int _currentTrackPrev;
    void newTrack(float pos);
    void recordDetection();
    bool isRecordPresent();
    void scanForTracks();
    void clearTracks();
    int getCurrentTrack();
    void scanLedOff();
    void setLedMilliAmp(float amp);
    int volt2pwm(float volt);
    void printGraphicData();
  public:
    bool recordPresent = false;
    bool graphicData = false;
    float tracks[100];
    float recordStart = 0;
    float current = 10;
    int trackCount = 0;
    int currentTrack = 0;
    void init();
    void func();
    void check();
    void setTracksAs7inch();
    void info();
}; // Scanner_

extern Scanner_& Scanner;

#endif // SCANNER_H
