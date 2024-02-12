#include "log.h"
#include "helper.h"


String padLeft(String value, int length, char character) {
  while (value.length() < length) {
    value = character + value;
  }
  return value;
} // padLeft()


String padRight(String value, int length, char character) {
  while (value.length() < length) {
    value += character;
  }
  return value;
} // padRight()


int roundTrip(int n, int max) {
  if (n < 0) {
    return max + (n % max);
  }
  return n % max;
} // roundTrip()


float limitFloat(float n, float min, float max) {
  if (n < min) {
    return min;
  } else if (n > max) {
    return max;
  } else {
    return n;
  }
} // limitFloat()


int limitInt(int n, int min, int max) {
  if (n < min) {
    return min;
  } else if (n > max) {
    return max;
  } else {
    return n;
  }
} // limitInt()


float mapFloat(float n, float nMin, float nMax, float min, float max) {
  return (((n - nMin) / (nMax - nMin)) * (max - min)) + min;
} // mapFloat()


bool isApprox(float value, float compare, float margin) {
  return (value > compare - margin) && (value < compare + margin);
} // isApprox()


String getState(eStates state) {
  String strState = "S_UNKNOWN";

  if        (state == S_STOPPING)               { strState = "S_STOPPING";
  } else if (state == S_PARKING)                { strState = "S_PARKING";
  } else if (state == S_HOMING)                 { strState = "S_HOMING";
  } else if (state == S_HOME)                   { strState = "S_HOME";

  } else if (state == S_HOMING_BEFORE_PLAYING)  { strState = "S_HOMING_BEFORE_PLAYING";
  } else if (state == S_GOTO_RECORD_START)      { strState = "S_GOTO_RECORD_START";
  } else if (state == S_PLAY_TILL_END)          { strState = "S_PLAY_TILL_END";
  } else if (state == S_PLAYING)                { strState = "S_PLAYING";

  } else if (state == S_PAUSE)                  { strState = "S_PAUSE";
  } else if (state == S_GOTO_TRACK)             { strState = "S_GOTO_TRACK";
  } else if (state == S_SKIP_FORWARD)           { strState = "S_SKIP_FORWARD";
  } else if (state == S_SKIP_REVERSE)           { strState = "S_SKIP_REVERSE";
  } else if (state == S_RESUME_AFTER_SKIP)      { strState = "S_RESUME_AFTER_SKIP";

  } else if (state == S_HOMING_BEFORE_CLEANING) { strState = "S_HOMING_BEFORE_CLEANING";
  } else if (state == S_NEEDLE_CLEAN)           { strState = "S_NEEDLE_CLEAN";
  } else if (state == S_RECORD_CLEAN)           { strState = "S_RECORD_CLEAN";
  } else if (state == S_CALIBRATE)              { strState = "S_CALIBRATE";
  } else if (state == S_HOMING_FAILED)          { strState = "S_HOMING_FAILED";
  } else if (state == S_BAD_ORIENTATION)        { strState = "S_BAD_ORIENTATION";
  }
  return strState;
} // getState()


String getRpmState(eRpmMode rpm) {
  String strRpm = "RPM_UNKNOWN";

  if        (rpm == RPM_AUTO)  { strRpm = "RPM_AUTO";
  } else if (rpm == RPM_33)    { strRpm = "RPM_33";
  } else if (rpm == RPM_45)    { strRpm = "RPM_45";
  } else if (rpm == RPM_78)    { strRpm = "RPM_78";
  }
  return strRpm;
} // getRpmState()


String getError(eErrors error) {
  String strError = "ERROR_UNKNOWN";

  if        (error == E_NONE)                  { strError = "E_NONE";
  } else if (error == E_NEEDLE_MOVE_BACKWARDS) { strError = "E_NEEDLE_MOVE_BACKWARDS";
  } else if (error == E_NEEDLE_DIDNT_MOVE)     { strError = "E_NEEDLE_DIDNT_MOVE";
  } else if (error == E_SPEED_UP_FAILED)       { strError = "E_SPEED_UP_FAILED";
  } else if (error == E_HOMING_FAILED)         { strError = "E_HOMING_FAILED";
  } else if (error == E_ARMANGLE_LIMIT_POS)    { strError = "E_ARMANGLE_LIMIT_POS";
  } else if (error == E_ARMANGLE_LIMIT_POS)    { strError = "E_ARMANGLE_LIMIT_POS";
  } else if (error == E_TO_MUCH_TRAVEL)        { strError = "E_TO_MUCH_TRAVEL";
  }
  return strError;
} // getError()


void setBit(uint8_t *byte, uint8_t n, bool value) {
  *byte = (*byte & ~(1UL << n)) | (value << n);
} // setBit()


bool getBit(uint8_t byte, uint8_t n) {
  return (byte >> n) & 1U;
} // getBit()

// uint32_t millisSinceBoot() {
uint64_t millisSinceBoot() {
  // return time_us_64() / 1000;
  return ((time_us_64() + ((uint64_t) 1000000 * 60 * 60 * 24 * 49) + ((uint64_t) 1000000 * 60 * 60 * 17)) / 1000); // 49:17:00:00.000 // 2m:47s before rollover
} // millisSinceBoot()

// uint32_t microsSinceBoot() {
uint64_t microsSinceBoot() {
  // return time_us_64();
  return time_us_64() + ((uint64_t) 1000000 * 60 * 60 * 24 * 49 + ((uint64_t) 1000000 * 60 * 60 * 17)); // 49:17:00:00.000 // 2m:47s before rollover
} // microsSinceBoot()

String uptime() {
  uint64_t ms = millisSinceBoot();
  unsigned long milliseconds = ms % 1000;
  unsigned long seconds = (ms / 1000) % 60;
  unsigned long minutes = (ms / (1000 * 60)) % 60;
  unsigned long hours = (ms / (1000 * 60 * 60)) % 24;
  unsigned long days = (ms / (1000 * 60 * 60 * 24));

  String uptimeString = String(days) + ":" + padLeft(String(hours),2,'0') + ":" + padLeft(String(minutes),2,'0') + ":" + padLeft(String(seconds),2,'0') + "." + padLeft(String(milliseconds),3,'0');
  
  return uptimeString;
} // uptime()
