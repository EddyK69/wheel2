#include "log.h"
#include "serialcomm.h"
#include "pins.h"
#include "helper.h"


SerialComm::SerialComm(Bluetooth& bluetooth, Buttons& buttons, Carriage& carriage,
      Scanner& scanner, Storage& storage) :
      _bluetooth(bluetooth),
      _buttons(buttons),
      _carriage(carriage),
      _scanner(scanner),
      _storage(storage),
      _interval(10000, TM_MICROS),
      _uptimeInterval(60, TM_MINS) {
} // SerialComm()


void SerialComm::init() {
    // Start serial output
  Serial.begin(SERIAL_BAUDRATE);

  // Wait until the serial stream is open
  delay(1000); // Needed, otherwise you miss the debug log's in all the init-void's
  // while (!Serial); // <- don't use this, as it waits for ages for a serial connection before is will startup

  if (Serial) {
    LOG_INFO("serialcomm.cpp", "[init] Serial port has been openend at " + String(SERIAL_BAUDRATE) + " bps (baud)");
  } else {
    LOG_CRITICAL("serialcomm.cpp", "[init] Serial port has not been opened!");
  }
} // init()


void SerialComm::func() {
  if (_interval.tick()) {

    if (_graphicData) {
      printGraphicData();
    } else {
      _headerShown = false;
    }

    while(Serial.available() > 0) {
      char letter = Serial.read();
      
      if ((letter == '\n' || letter == '\r') && _line != "") {
        _lineRaw = _line;
        _line.trim();
        _line.toLowerCase();

        if (_line.startsWith("l")) { // 'L' is last command; add previous command to _line
          _line.replace("l", _lastCommand);
        }

        checkReceivedLine(_line);
        _line = "";
      } else {
        _line += letter;
      }
    }
  } // _interval.tick()

  if (_uptimeInterval.tick()) {
    Serial.println("UPTIME: " + msToString(millisSinceBoot()));
    Serial.println("TEMPERATURE: " + String(analogReadTemp(), 2) + " °C");
  }
} // func()


void SerialComm::checkReceivedLine(String line, eCheckMode mode) {
  LOG_DEBUG("serialcomm.cpp", "[checkReceivedLine]");
  println(mode);
  if (checkLineCommand( "RST",    "Reboot",                     mode)) { rp2040.reboot();                     return; }
  // if (checkLineCommand( "BOOT",   "Reboot to USB bootloader",   mode)) { rp2040.rebootToBootloader();         return; }

  if (checkLineCommand( "AT+",    "Bluetooth command",          mode)) { _bluetooth.write(_lineRaw);          return; }
  if (checkLineBool(    "BT",     "Bluetoot uart",              mode,  _bluetooth.debug)) {                   return; }

  if (checkLineBool(    "G",      "Graphics",                   mode, _graphicData)) {                        return; }
  if (checkLineBool(    "PLG",    "RecordScanner graphics",     mode, _scanner.graphicData)) {                return; }
  if (checkLineBool(    "KG",     "Carriage graphics",          mode, _carriage.graphicData)) {               return; }
  if (checkLineBool(    "SG",     "Strobo graphics",            mode, SpeedComp.graphicData)) {               return; }
  if (checkLineBool(    "OG",     "Orientation graphics",       mode, Orientation.graphicData)) {             return; }

  //-------------------------------------------------- STATE --------------------------------------------------
  println(mode);
  if (checkLineCommand( ">>",     "Next track",                 mode)) { _carriage.gotoNextTrack();           return; }
  if (checkLineCommand( "<<",     "Previous track",             mode)) { _carriage.gotoPreviousTrack();       return; }
  if (checkLineCommand( "HOK",    "Home",                       mode)) { Shared.setState(S_HOME);             return; }
  if (checkLineCommand( "STOP",   "Stop",                       mode)) { Plateau.stop();                      return; }
  if (checkLineCommand( "SPEEL",  "Play",                       mode)) { Plateau.play();                      return; }
  if (checkLineCommand( "PAUZE",  "Pause",                      mode)) { _carriage.pause();                   return; }
  if (checkLineCommand( "NAALD",  "Clean needle",               mode)) { Shared.setState(S_NEEDLE_CLEAN);     return; }
  if (checkLineCommand( "CAL",    "Calibrate",                  mode)) { Shared.setState(S_CALIBRATE);        return; }
  if (checkLineBool(    "REP",    "Repeat",                     mode, _carriage.repeat)) {                    return; }

  //-------------------------------------------------- ARM --------------------------------------------------
  println(mode);
  if (checkLineCommand( "NE",     "Needle down",                mode)) { Arm.putNeedleInGrove();              return; }
  if (checkLineCommand( "NA",     "Needle up",                  mode)) { Arm.dockNeedle();                    return; }
  if (checkLineFloat(   "ATG",    "Arm targetweight",           mode, Arm.targetWeight)) { _storage.saveRequired  = true; return; }
  if (checkLineFloat(   "AG",     "Arm weight",                 mode, Arm.weight)) {                          return; }
  if (checkLineCommand( "AKHOK",  "Arm force Docked calibrate", mode)) { Arm.justDockedWeight = Arm.weight;   Serial.println(padRight("AKHOK", 8) + " " + padRight("Arm force Docked calibrate", 26) + " SET: " + String(Arm.justDockedWeight, 5)); _storage.saveRequired = true; return; }
  if (checkLineCommand( "AKL",    "Arm force 500mg calibrate",  mode)) { Arm.forceLow = Arm.force;            Serial.println(padRight("AKL", 8)   + " " + padRight("Arm force 500mg calibrate", 26)  + " SET: " + String(Arm.forceLow, 5));         _storage.saveRequired = true; return; }
  if (checkLineCommand( "AKH",    "Arm force 4000mg calibrate", mode)) { Arm.forceHigh = Arm.force;           Serial.println(padRight("AKH", 8)   + " " + padRight("Arm force 4000mg calibrate", 26) + " SET: " + String(Arm.forceHigh, 5));        _storage.saveRequired = true; return; }
  if (checkLineFloat(   "AK",     "Arm force",                  mode, Arm.force)) { Arm.force = limitFloat(Arm.force, 0, 1); return;}
 
  //-------------------------------------------------- CARRIAGE --------------------------------------------------
  println(mode);
  if (checkLineFloat(   "KP",     "Carriage P",                 mode, _carriage.P)) {                         return; }
  if (checkLineFloat(   "KI",     "Carriage I",                 mode, _carriage.I)) {                         return; }
  if (checkLineFloat(   "KD",     "Carriage D",                 mode, _carriage.D)) {                         return; }
  if (checkLineFloat(   "TNP",    "Target track",               mode, _carriage.targetTrack)) { _carriage.targetTrack = limitFloat(_carriage.targetTrack, CARRIAGE_HOME, CARRIAGE_12INCH_START); return; }

  //-------------------------------------------------- PLATEAU --------------------------------------------------
  println(mode);
  if (checkLineFloat(   "PP",     "Plateau P",                  mode, Plateau.P)) {                           return; }
  if (checkLineFloat(   "PI",     "Plateau I",                  mode, Plateau.I)) {                           return; }
  if (checkLineFloat(   "PD",     "Plateau D",                  mode, Plateau.D)) {                           return; }

  if (checkLineBool(    "PR",     "Plateau motor reverse",      mode, Plateau.motorReverse)) {                return; }

  if (checkLineFloat(   "TR",     "Target RPM",                 mode, Plateau.targetRpm)) { Plateau.turnInterval.reset(); return; }
  // if (checkLineInt(     "RPM",    "RPM mode (1/3/4)",           mode, Plateau.rpmMode)) {                      return; } // TODO: EK

  if (checkLineCommand( "PA",     "Plateau start",              mode)) { Plateau.motorStart();                return; }
  if (checkLineCommand( "PS",     "Plateau stop",               mode)) { Plateau.motorStop();                 return; }
  if (checkLineBool(    "PL",     "Plateau logica",             mode, Plateau.logic)) {                       return; }
  if (checkLineBool(    "PC",     "Unbalance compensation",     mode, Plateau.unbalanceCompensation)) {       return; }

  //-------------------------------------------------- STROBO --------------------------------------------------
  println(mode);
  // if (checkLineInt(     "SSN",    "Strobo samples",             mode, SpeedComp.samples)) {                   return; }
  if (checkLineBool(    "SOC",    "Strobo unbalance Comp. On",  mode, SpeedComp.unbalanceCompOn)) {           return; }
  if (checkLineBool(    "SKC",    "Strobo OffCenter Comp.",     mode, SpeedComp.recordOffCenterComp)) {       return; }
  if (checkLineBool(    "KC",     "Carriage OffCenter Comp.",   mode, _carriage.offCenterCompensation)) {     return; }

  if (checkLineFloat(   "SOG",    "Strobo unbal. Comp. Weight", mode, SpeedComp.unbalanceCompWeight)) {       return; }
  if (checkLineFloat(   "SOFB",   "Strobo unbal. Filter Width", mode, SpeedComp.unbalanceFilterWidth)) {SpeedComp.createUnbalanceFilterCurve(); return; }
  if (checkLineInt(     "SOF",    "Strobo unbal. Phase",        mode, SpeedComp.unbalancePhase)) {            return; }

  if (checkLineCommand( "SCZ",    "Strobo clearCompSamples On T0", mode)) { SpeedComp.clearCompSamplesOnT0(); return; }
  if (checkLineCommand( "SCC",    "Strobo clearCompSamples",    mode)) { SpeedComp.clearCompSamples();        return; }

  //-------------------------------------------------- STORAGE --------------------------------------------------
  println(mode);
  if (checkLineFloat(   "EV",     "eepromVersie",               mode, _storage.eepromVersion)) {              return; }
  if (checkLineCommand( "EO",     "Save EEPROM",                mode)) { _storage.write();                    return; }
  if (checkLineCommand( "EL",     "Read EEPROM",                mode)) { _storage.read();                     return; }
  if (checkLineCommand( "OC",     "Orientation calibrate",      mode)) { Orientation.calibrate(); _storage.saveRequired = true; return; }
  if (checkLineFloat(   "TO",     "Track offset",               mode, _carriage.trackOffset)) { _storage.saveRequired = true; return; }
  if (checkLineCommand( "AHCal",  "Calibrate arm angle",        mode)) { Arm.calibrateAngle(); _storage.saveRequired = true; return; }

  //-------------------------------------------------- CARRIAGE SENSORS --------------------------------------------------
  println(mode);
  // if(checkLineFloat(    "PLS",    "Scanner current",            mode, _scanner.current)) {                    return; }
  if (checkLineInt(     "VOLUME", "Volume w/o override",        mode, Amplifier.volume)) { Amplifier.volumeOverRide = false; return; }
  if (checkLineInt(     "VOL",    "Volume",                     mode, Amplifier.volume)) { Amplifier.volumeOverRide = true; return; }
  if (checkLineCommand( "AHCent", "Center Arm Angle",           mode)) { Arm.centerArmAngle();                return; }

  //-------------------------------------------------- HELP --------------------------------------------------
  println(mode);
  if (checkLineCommand( "C?",     "Show commands",              mode)) { checkReceivedLine(line, CM_COMMAND); return; }
  if (checkLineCommand( "CW",     "Show values",                mode)) { checkReceivedLine(line, CM_VALUE);   return; }
  if (checkLineCommand( "?",      "Report",                     mode)) { report();                            return; }
  if (checkLineCommand( "INFO",   "Info",                       mode)) { info();                              return; }
  if (checkLineCommand( "VER",    "HW/FW version",              mode)) { version();                           return; }

  if (mode == CM_NONE) {
    line.toUpperCase();
    Serial.println("Wrong command received: \"" + line + "\"");
  }
} // checkReceivedLine()


bool SerialComm::checkLineCommand(String command, String description, eCheckMode mode) {
  if (mode == CM_VALUE) {
    return false;
  }
  if (!checkLine(command, description, mode)) {
    return false;
  }
  Serial.println();
  return true;
} // checkLineCommand()


bool SerialComm::checkLine(String command, String description, eCheckMode mode) {
  if (mode == CM_COMMAND) {
    printCommando(command, description);
    Serial.println();
    return false;
  }
  command.toLowerCase();
  if (!_line.startsWith(command)) {
    return false;
  }
  _lastCommand = command;
  _line.replace(command, "");
  _line.trim();
  printCommando(command, description);
  return true;
} // checkLine()


bool SerialComm::checkLineInt(String command, String description, eCheckMode mode, int& value) {
  if (mode == CM_VALUE) {
    printValue(command, description, String(value));
    return false;
  }
  if (!checkLine(command, description, mode)) {
    return false;
  }
  if (_line.indexOf('?') == -1 && _line.length() != 0) { // If no '?' in command
    value = _line.toInt();
    Serial.print("SET: ");
  } else {
    Serial.print("GET: ");
  }
  Serial.println(String(value));
  return true;
} // checkLineInt()


bool SerialComm::checkLineFloat(String command, String description, eCheckMode mode, float& value) {
  if (mode == CM_VALUE) {
    printValue(command, description, String(value, 5));
    return false;
  }
  if (!checkLine(command, description, mode)) {
    return false;
  }
  if (_line.indexOf('?') == -1 && _line.length() != 0) { // If no '?' in command
    value = _line.toFloat();
    Serial.print("SET: ");
  } else {
    Serial.print("GET: ");
  }
  Serial.println(String(value, 5));
  return true;
} // checkLineFloat()


bool SerialComm::checkLineBool(String command, String description, eCheckMode mode, bool& value) {
  if (mode == CM_VALUE) {
    printValue(command, description, String(value));
    return false;
  }
  if (!checkLine(command, description, mode)) {
    return false;
  }
  if (_line.indexOf('?') != -1) { // If there is a '?' in command
    Serial.print("GET: ");
  } else if (isDigit(_line.charAt(0))) {
    value = _line.toInt();
    Serial.print("SET: ");
  } else {
    value = !value;
    Serial.print("TOGGLED: ");
  }   
  Serial.println(String(value));
  return true;
} // checkLineBool()


void SerialComm::println(eCheckMode mode) {
  if (mode != CM_NONE) {
    Serial.println();
  }
} // println()


void SerialComm::printCommando(String command, String description) {
  command.toUpperCase();
  Serial.print(padRight(command, 8) + " " + padRight(description, 26) + " ");
} // printCommando()


void SerialComm::printValue(String command, String description, String value) {
  command.toUpperCase();
  Serial.println(padRight(command, 8) + " " + padRight(description, 26) +  " " + value);
} // printValue()


void SerialComm::printGraphicData() {
  if (!_headerShown) {
    Serial.println("GRAPH_HEADER: SpeedRaw-TRPM, Speed-CTRPM, PPR, CTPRM-TRPM, UnbalanceComp, ArmAngleCall, RealPosition, Trackspacing, ArmWeight");
    _headerShown = true;
  }
  Serial.print(SpeedComp.speedRaw - Plateau.targetRpm, 3);
  Serial.print(", ");
  Serial.print(SpeedComp.speed - SpeedComp.centerCompTargetRpm, 3);

  // Serial.print(", ");
  // Serial.print(SpeedComp.speed, 3);
  Serial.print(", ");
  Serial.print((float)SpeedComp.rotationPosition / SpeedComp.pulsesPerRev, 3);

  // Serial.print(", ");
  // Serial.print(SpeedComp._unbalanceFilterCurve[SpeedComp.rotationPosition]);

  // Serial.print(", ");
  // Serial.print(SpeedComp.speedLowPass, 3);

  // Serial.print(", ");
  // Serial.print(SpeedComp._processInterval);

  // Serial.print(", ");
  // Serial.print(SpeedComp.speedLowPass - Plateau.targetRpm, 3);
  // Serial.print(", ");
  // Serial.print(SpeedComp.lowpassRect, 3);
  // Serial.print(", ");
  // Serial.print(SpeedComp.wow, 3);

  // Serial.print(", ");
  // Serial.print(SpeedComp.speedHighPass, 3);

  Serial.print(", ");
  Serial.print(SpeedComp.centerCompTargetRpm - Plateau.targetRpm, 3);

  // Serial.print(", ");
  // Serial.print(SpeedComp.rotationPosition / float(SpeedComp.pulsesPerRev));

  // Serial.print(", ");
  // Serial.print(SpeedComp.preComp, 4);

  Serial.print(", ");
  Serial.print(SpeedComp.unbalanceComp, 4);

  // Serial.print(", ");
  // Serial.print(Plateau._outBuff, 2);

  // Serial.print(", ");
  // Serial.print(Plateau._outBuffPrev, 2);

  // Serial.print(", ");
  // Serial.print(Arm.armAngleRaw); // 1696);

  // Serial.print(", ");
  // Serial.print(_carriage._Dcomp, 4); // 1696);
  Serial.print(", ");
  Serial.print(Arm.armAngleCall, 4); // 1696);

  // Serial.print(", ");
  // Serial.print(Arm.armAngleSlow, 5); // 1696);
  // Serial.print(", ");
  // Serial.print(Arm.armAngleOffset, 5); // 1696);

  // Serial.print(", ");
  // Serial.print(_carriage.position, 3);
  Serial.print(", ");
  Serial.print(_carriage.realPosition, 3);

  // // Serial.print(", ");
  // // Serial.print(SpeedComp.carriagePosMiddle, 3);

  // // Serial.print(", ");
  // // Serial.print(SpeedComp.carriagePosMiddle + SpeedComp.carriageFourier, 3);

  // Serial.print(", ");
  // Serial.print(SpeedComp.carriagePosMiddle + SpeedComp.carriageFourierFilter, 3);

  Serial.print(", ");
  Serial.print(SpeedComp.trackSpacing, 3);

  Serial.print(", ");
  Serial.print(Arm.weight, 3);

 Serial.println();
} // printGraphicData()


void SerialComm::report() {
  Serial.println("-------------------- V" + String(Shared.appversion) + " --------------------");
  Serial.println();
  Serial.println(padRight("WHEEL_TEMPERATURE", PADR) + ": " + String(analogReadTemp(), 2) + " °C");
  Serial.println();
  _storage.info();
  Orientation.info();
  // SpeedComp.info();
  Serial.println("----------------------------------------------");
} // report()


void SerialComm::info() {
  version();
  Serial.println(padRight("WHEEL_UPTIME", PADR) +           ": " + msToString(millisSinceBoot()));
  Serial.println(padRight("WHEEL_TEMPERATURE", PADR) +      ": " + String(analogReadTemp(), 2) + " °C");
  Serial.println(padRight("WHEEL_STATE", PADR) +            ": " + getState(Shared.state));
  Serial.println(padRight("WHEEL_VOLUME", PADR) +           ": " + String(Amplifier.volume));
  Serial.println();
  _storage.info();
  Orientation.info();
  Plateau.info();
  SpeedComp.info();
  _carriage.info();
  _scanner.info();
  Arm.info();
  _buttons.info();
  Shared.info();
  Serial.println("----------------------------------------------");
} // info()


void SerialComm::version() {
  Serial.println("-------------------- V" + String(Shared.appversion) + " --------------------");
  Serial.println();
  Serial.println(padRight("WHEEL_HW_VERSION", PADR) +       ": " + String(BOARD_DESCRIPTION) + String(_bluetooth.wirelessVersion ? " [BT]" : ""));
  Serial.println(padRight("WHEEL_FW_VERSION", PADR) +       ": V" + String(Shared.appversion) + " [" + Shared.appdate + "]");
  // Serial.println(padRight("WHEEL_WIRELESS_VERSION", PADR) + ": " + String(_bluetooth.wirelessVersion ? "YES" : "NO"));
} // version()


//             bytes   cycles                
// LD (HL),d8      2   12
// INC L           1   4
//                 3   16

// LD (HL),d8      2   12
// INC HL          1   8
//                 3   20

// LD (HL+),A      1   8
// LD A,d8         2   8
//                 3   16                

// LD A, [DE]      1   8
// LD (HL+),A      1   8
//                 2   16

// LD SP,d16       3   12
// LD (a16),SP     3   20
//                 6   32
