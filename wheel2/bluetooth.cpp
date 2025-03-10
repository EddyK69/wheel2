#include "log.h"
#include "bluetooth.h"
#include "pins.h"
#include "helper.h"


void Bluetooth_::init() {
  LOG_DEBUG("bluetooth.cpp", "[init]");
  Serial2.setRX(BT_RXD_PIN);
  Serial2.setTX(BT_TXD_PIN);
  Serial2.setPollingMode(true);
  Serial2.setFIFOSize(1024);
  Serial2.begin(115200);
  // Serial2.begin(9600);

  write("AT+");
} // init()


void Bluetooth_::func() {
  if (millisSinceBoot() < 1000) {
    return;
  }

  if (_initTodo) {
    _initTodo = false;
    init();
    return;
  }

  // if(_checkBeforeStartInterval.once()){
  //   write("AT+");
  // }

  if (_interval.tick()) {
    while (Serial2.available() > 0) {
      char c = Serial2.read();
      _buffer += c;
    }

    if (_buffer != "") {
      LOG_DEBUG("bluetooth.cpp", "[func] BT IN:" + _buffer);
      _buffer = "";
    }

    // while (Serial2.available() > 0) {
    //   char c = Serial2.read();
    //   if (c == '\n' || c == '\r' ) {
    //     // if (c == '\n') {
    //     //   Serial.print("<nl>");
    //     // }
    //     // if (c == '\r') {
    //     //   Serial.print("<cr>");
    //     // }
    //     if (_buffer != "") {
    //       encode();
    //     }

    //     _buffer = "";
    //   } else {
    //     _buffer += c;
    //   }
    // }
  }
} // func()


void Bluetooth_::write(String command) {
  Serial2.print(command + "\r\n");

  // LOG_DEBUG("bluetooth.cpp", "[write] BT OUT:" + command);
  if (debug) {
    Serial.println("BT OUT:" + command);
  }
} // write()


void Bluetooth_::encode() {
  // LOG_DEBUG("bluetooth.cpp", "[encode] BT IN:" + _buffer);
  if (debug) {
    Serial.println("BT IN:" + _buffer);
  }

  if (_buffer.startsWith("income_opid:")) {
    _buffer.replace("income_opid:", "");
    _buffer.trim();

    if (_buffer.startsWith(BT_BUTTON_IN)) {
      _buffer.remove(0, 1);  // remove 'in' from buffer
      LOG_DEBUG("bluetooth.cpp", "[encode] KNOP_IN:" + _buffer);

      // if (_buffer == BT_PLAY) {
      //   if (Shared.state == S_PAUSE || Shared.state == S_PLAYING) { // maybe remove S_PLAYING?
      //     Carriage.pause();
      //   } else if(Shared.state == S_HOME) {
      //     Plateau.play();
      //   }
      // }

      if (_buffer == BT_PLAY) {
        if (Shared.state == S_PAUSE) { 
          Carriage.pause();
        } else if (Shared.state == S_HOME) {
          Plateau.play();
        }
      } else if (_buffer == BT_PAUSE) {
        if (Shared.state == S_PAUSE || Shared.state == S_PLAYING) { // maybe remove S_PLAYING?
          Carriage.pause();
        }
      } else if (_buffer == BT_NEXT_TRACK) {
        if (Shared.state == S_PLAYING || Shared.state == S_PAUSE || Shared.state == S_GOTO_TRACK) {
          Carriage.gotoNextTrack();
        }
      } else if (_buffer == BT_PREV_TRACK) {
        if (Shared.state == S_PLAYING || Shared.state == S_PAUSE || Shared.state == S_GOTO_TRACK) {
          Carriage.gotoPreviousTrack();
        }
      }
    } else if (_buffer.startsWith(BT_BUTTON_OUT)) {
      _buffer.remove(0, 1);  // remove 'in' from buffer
      LOG_DEBUG("bluetooth.cpp", "[encode] BUTTON_OUT:" + _buffer);
    } else {
      LOG_DEBUG("bluetooth.cpp", "[encode] BUTTON_UNKNOWN:" + _buffer);
    }
  }

  if (_buffer.startsWith("OK+")) {
    if (millisSinceBoot() < 4000) {
      _wirelessVersion = true;  // ff checken of er wel een bluetooth module is aangesloten
    }
  }
} // encode()


Bluetooth_ &Bluetooth_::getInstance() {
  static Bluetooth_ instance;
  return instance;
} // getInstance()


Bluetooth_ &Bluetooth = Bluetooth.getInstance();
