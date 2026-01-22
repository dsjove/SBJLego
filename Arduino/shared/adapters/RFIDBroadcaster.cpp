#include "RFIDBroadcaster.h"

RFIDBroadcaster::RFIDBroadcaster(Scheduler& scheduler, BLEServiceRunner& ble, uint32_t number, int ss_pin, int rst_pin)
: _rfid(number, ss_pin, rst_pin)
, _idFeedbackChar(ble, "05000002", _rfid.lastID().encode())
, _rfidTask(scheduler, _rfid.timing().taskFrequency, this)
{
}

void RFIDBroadcaster::begin()
{
  _rfid.begin();
  Serial.print("RFID: ");
  _rfid.lastID().print();
  Serial.println();
}

void RFIDBroadcaster::loop(Task&)
{
  const MFRC522Detector::RFID* detected = _rfid.loop();
  if (detected)
  {
    auto encoded = detected->encode();
    Serial.print("RFID: ");
    detected->print();
//    Serial.print(" -- ");
//    MFRC522Detector::RFID::print(encoded);
    Serial.println();
//    Serial.println(_idFeedbackChar.uuid.data());
    _idFeedbackChar.ble.writeValue(encoded.data(), detected->encodedSize());
  }
}
