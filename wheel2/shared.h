#ifndef SHARED_H
#define SHARED_H

#include <Arduino.h>
#include "enums.h"
#include "interval.h"


class Shared_ {
  private:
    Shared_() = default; // Make constructor private

  public:
    static Shared_& getInstance(); // Accessor for singleton instance

    Shared_(const Shared_&) = delete; // no copying
    Shared_& operator=(const Shared_&) = delete;

  private:
    bool firstTimeStateChange = false;
    int getTotalErrors();
  public:
    Interval stateChangedInterval = Interval(1000, TM_MILLIS);
    Interval errorChangedInterval = Interval(0, TM_MILLIS);
    int appversion;
    String appdate;
    eStates state = S_HOME;
    eErrors error = E_NONE;
    int errorCount[E_MAX];
    bool puristMode = false;
    void init(int app_version, String app_date);
    void setState(eStates state);
    void setError(eErrors newError);
    bool firstTimeStateChanged();
    void info();
}; // Shared_

extern Shared_& Shared;

#endif //SHARED_H
