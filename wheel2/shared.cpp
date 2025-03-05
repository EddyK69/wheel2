#include "log.h"
#include "shared.h"
#include "helper.h"


void Shared_::init(int app_version, String app_date) {
  LOG_DEBUG("shared.cpp", "[init]");
  appversion = app_version;
  appdate = app_date;
}


void Shared_::setState(eStates newState) {
  LOG_DEBUG("shared.cpp", "[setState]");
  stateChangedInterval.reset();
  firstTimeStateChange = true;

  state = newState;
  // LOG_DEBUG("shared.cpp", "[setState] State changed to " + getState(state));
  Serial.println("STATE: " + getState(state));
} // setState()


void Shared_::setError(eErrors newError) {
  LOG_DEBUG("shared.cpp", "[setError]");
  error = newError;
  errorChangedInterval.reset();
  Serial.println("ERROR: " + getError(error));
  Serial.println("STATE: " + getState(state));
  errorCount[error] += 1;
  Serial.println("TOTAL_ERRORS: " + String(getTotalErrors()));
} // setError()


bool Shared_::firstTimeStateChanged() {
  if (firstTimeStateChange) {
    firstTimeStateChange = false;
    return true;
  }
  return false;
} // firstTimeStateChanged()


int Shared_::getTotalErrors() {
  int totalErrors = 0;
  for (int error = 0; error < E_MAX; error++) {
    totalErrors += errorCount[error];
  }
  return totalErrors;
} // getTotalErrors()


void Shared_::info() {
  for (int error = 0; error < E_MAX; error++) {
    String err = getError(static_cast<eErrors>(error));
    if ((err != "E_UNKNOWN") && (err != "E_NONE")) {
      Serial.println(padRight(err, PADR) + ": " + String(errorCount[error]));
    }
  }
  Serial.println(padRight("TOTAL_ERRORS", PADR) + ": " + String(getTotalErrors()));

  Serial.println();
} // info()


Shared_ &Shared_::getInstance() {
  static Shared_ instance;
  return instance;
} // getInstance()


Shared_ &Shared = Shared.getInstance();