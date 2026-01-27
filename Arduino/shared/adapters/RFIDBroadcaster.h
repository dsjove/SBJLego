#pragma once

#include "../core/TaskThunk.h"
#include "../core/RFIDDetector.h"
#include "../ble/IDBTCharacteristic.h"

template<typename Traits = RFIDDetectorDefaultTraits>
class RFIDBroadcaster : ScheduledRunner
{
public:
  using Detector = RFIDDetector<Traits>;

  RFIDBroadcaster(Scheduler& scheduler, BLEServiceRunner& ble)
  : _rfid()
  , _idFeedbackChar(ble, writeIndex("05000002", Traits::Number), _rfid.lastID().encode())
  , _rfidTask(scheduler, Traits::loopFrequencyMs, this)
  {
  }

  void begin()
  {
    _rfid.begin();
    Serial.print("RFID: ");
    _rfid.lastID().print();
    Serial.println();
  }

private:
  Detector _rfid;
  IDBTCharacteristic _idFeedbackChar;
  TaskThunk _rfidTask;

  virtual void loop(Task&)
  {
    const RFID* detected = _rfid.loop();
    if (detected)
    {
      auto encoded = detected->encode();
      Serial.print("RFID: ");
      detected->print();
//      Serial.print(" -- ");
//      RFID::print(encoded);
      Serial.println();
//      Serial.println(_idFeedbackChar.uuid.data());
      _idFeedbackChar.ble.writeValue(encoded.data(), detected->encodedSize());
    }
  }
};
