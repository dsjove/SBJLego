#include "MFRC522Detector.h"

MFRC522Detector::MFRC522Detector(uint32_t number, int ss_pin, int rst_pin, MFRC522Detector::Timing timing)
: _ss_pin(ss_pin)
, _rst_pin(rst_pin)
, _timing(timing)
, _rfid(ss_pin, rst_pin)
, _lastID(number)
, _cooldownLimitMs(0)
, _lastGoodReadMs(0)
, _failReadCount(0)  
{
}

MFRC522Detector::RFID::RFID(uint32_t number)
: _number(number)
, _timestamp(0)
, _length(0)
, _uuid({0})
{
}

void MFRC522Detector::begin()
{
  pinMode(_rst_pin, OUTPUT);
  digitalWrite(_rst_pin, HIGH);
  _rfid.PCD_Init();
}

const MFRC522Detector::RFID* MFRC522Detector::loop() {
  const uint32_t now = millis();

  // Re-init after long inactivity without a successful read
  if (_lastGoodReadMs != 0 && (now - _lastGoodReadMs) > _timing.reinitAfterMs)
  {
    //Serial.println("RFID: Inactivity Reset");
    resetRc522();
    _lastGoodReadMs = now;
  }

  if (_rfid.PICC_IsNewCardPresent())
  {
    // Reader tells new card is present
	//Serial.println("RFID: New Card");

    // Cooldown gate (prevents spamming rf reads and ble writes)
    if (now < _cooldownLimitMs)
    {
      //Serial.println("RFID: Cooldown");
      return NULL;
    }
    // Read the serial number
    if (_rfid.PICC_ReadCardSerial()) 
    {
      // Record a good read time
      _lastGoodReadMs = now;
      _failReadCount = 0;

      _lastID.update(_rfid.uid, now);

      // Always end the (read) conversation with the tag
      _rfid.PICC_HaltA();
      _rfid.PCD_StopCrypto1();

      // Start cooldown after successful read
      _cooldownLimitMs = now + _timing.cooldownMs;

      // Report change ID or timestamp
      return &_lastID;
    }
    else 
    {
      Serial.println("RFID: Read Failed");
      // Cleanup even on failed read (prevents wedged state)
      _rfid.PCD_StopCrypto1();
      _rfid.PICC_HaltA();

      _failReadCount++;

      // Hard reset after repeated failures
      if (_failReadCount >= _timing.failResetCount)
      {
        Serial.println("RFID: Fail Count Reset");
        resetRc522();
      }
    }
  }
  // Hovering not detectable.
  return NULL;
}

void MFRC522Detector::resetRc522()
{
  // Hard reset the RC522 using its RST pin
  digitalWrite(_rst_pin, LOW);
  delay(5);
  digitalWrite(_rst_pin, HIGH);
  delay(5);

  _rfid.PCD_Init();
  // Optional: max gain can improve marginal reads
  _rfid.PCD_SetAntennaGain(_rfid.RxGain_max);
  _failReadCount = 0;
  //Do not reset _lastGoodReadMs
}

void MFRC522Detector::RFID::update(const MFRC522::Uid& u, uint32_t timestamp)
{
  _timestamp = timestamp;
  const uint8_t len = (u.size > 10) ? 10 : u.size;
  _length = len;
  std::copy(u.uidByte, u.uidByte + len, _uuid.begin());
}

void MFRC522Detector::RFID::print() const
{
  Serial.print(_number);
  Serial.print("-");
  Serial.print(_timestamp);
  Serial.print("-");
  Serial.print(_length);
  if (_length) {
    Serial.print("-");
    for (uint8_t i = 0; i < _length; i++) {
      if (_uuid[i] < 0x10) Serial.print('0');
      Serial.print(_uuid[i], HEX);
      if (i < _length-1) Serial.print('.');
    }
  }
}

MFRC522Detector::RFID::Encoded MFRC522Detector::RFID::encode() const
{
  Encoded encoded;
  const uint32_t number = _number;
  std::copy(
    reinterpret_cast<const uint8_t*>(&number),
    reinterpret_cast<const uint8_t*>(&number) + sizeof(number),
    encoded.begin()
  );
  const uint32_t ts = _timestamp;
  std::copy(
    reinterpret_cast<const uint8_t*>(&ts),
    reinterpret_cast<const uint8_t*>(&ts) + sizeof(ts),
    encoded.begin() + 4
  );
  encoded[8] = _length;
  std::copy(_uuid.begin(), _uuid.begin() + _length, encoded.begin() + 9);
  return encoded;
}

void MFRC522Detector::RFID::print(const Encoded& encoded) {
  for (size_t i = 0; i < size_t(4 + 4 + 1 + encoded[8]); i++) {
    if (encoded[i] < 0x10) Serial.print('0');
    Serial.print(encoded[i], HEX);
    if (i == 3 || i == 7 || i == 8) Serial.print('-');
  }
}
