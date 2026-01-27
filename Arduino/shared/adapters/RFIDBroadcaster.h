#pragma once

#include "../core/TaskThunk.h"
#include "../core/MFRC522Detector.h"
#include "../ble/IDBTCharacteristic.h"

class RFIDBroadcaster : ScheduledRunner
{
public:
  RFIDBroadcaster(Scheduler& scheduler, BLEServiceRunner& ble);

  void begin();

private:
  MFRC522Detector<0> _rfid;
  IDBTCharacteristic _idFeedbackChar;
  TaskThunk _rfidTask;

  virtual void loop(Task&);
};
