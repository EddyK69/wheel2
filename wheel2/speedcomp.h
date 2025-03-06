#ifndef SPEEDCOMP_H
#define SPEEDCOMP_H

#include <Arduino.h>
#include "shared.h"
#include "arm.h"

#define SPEEDCOMP_TAU         2*PI

#define SPEEDCOMP_SAMPLES     16
#define SPEEDCOMP_PPR         720
#define SPEEDCOMP_SAMPLES_MAX 65500
#define SPEEDCOMP_PPR_MAX     1000


class SpeedComp_ {
  private:
    SpeedComp_() = default; // Make constructor private

  public:
    static SpeedComp_& getInstance(); // Accessor for singleton instance

    SpeedComp_(const SpeedComp_&) = delete; // no copying
    SpeedComp_& operator=(const SpeedComp_&) = delete;

  private:
    //-------------------- speed
    volatile uint64_t _speedInterval;
    volatile uint64_t _time;
    volatile uint64_t _interval;

    //-------------------- direction
    char _sens, _sensPrev;
    int _direction;
    int _directionPrev;
    int _glitchCounter;
    int _counterRaw = 0;

    //-------------------- samplede sinus & cos
    float _sinus[SPEEDCOMP_PPR_MAX];
    float _cosin[SPEEDCOMP_PPR_MAX];

    //-------------------- filter
    volatile int _samplesArr[100];
    volatile uint64_t _sampleCounter = 0;

    float _average = SPEEDCOMP_SAMPLES_MAX;

    //-------------------- off center compensation
    float _carriagePosMiddlePre;
    float _carriageOffCenterWave[SPEEDCOMP_PPR_MAX];
    float _carriagePosCenterHist[SPEEDCOMP_PPR_MAX];
    float _carriageSinValues[SPEEDCOMP_PPR_MAX];
    float _carriageCosValues[SPEEDCOMP_PPR_MAX];
    float _carriageSin;
    float _carriageCos;

    float _carriageSinFilt;
    float _carriageCosFilt;
    float _centerComp;

    //-------------------- unbalance compensation
    int _unbalanceFilterCurve[SPEEDCOMP_PPR_MAX];
    int _unbalansComp[SPEEDCOMP_PPR_MAX];
    int _unbalanceFilterCurveWidth = 0;

    //-------------------- debug
    // uint64_t _processTime;
    // uint64_t _processInterval;
    bool _clearCompSamplesQueue = false;
    bool _headerShown = false;
    bool _wowFirstLow;
    int _counterSinceReset;

    void clearSamples();
    void clearUnbalanceCompSamples();
    void clearCenterCompSamples();
    float getSpeed();
    float averageInterval();
    float currentSpeed(float inter);
    void shiftSamples(int sample);
    void printGraphicData();
  public:
    //-------------------- direction
    int rotationPosition = 0;
    int pulsesPerRev = SPEEDCOMP_PPR;

    //-------------------- filter
    int samples = SPEEDCOMP_SAMPLES;
    float speedRaw;
    float speed;
    float speedCenterComp;
    float speedLowPass;
    float speedHighPass;
    float lowpassRect;
    float wow;

    //-------------------- off center compensation
    float carriageFourier;
    float carriageFourierFilter;

    //-------------------- unbalance compensation
    int unbalancePhase = 23; //25; //50; //50;
    float unbalanceCompWeight = 1.2; // 1.1; //1.3; //2;
    float unbalanceFilterWidth = 240; //80; //65; //50; //100;
    volatile float unbalanceComp = 0;

    //-------------------- debug
    bool graphicData = false;
    bool unbalanceCompOn = true;
    bool recordOffCenterComp = true;

    float carriagePosMiddle;
    float trackSpacing;
    float centerCompTargetRpm;

    void init();
    void update();
    void clearCompSamples();
    void clearCompSamplesOnT0();
    void stroboInterrupt();
    void createUnbalanceFilterCurve();
    void info();
}; // SpeedComp_

extern SpeedComp_& SpeedComp;

#endif // SPEEDCOMP_H
