#include "MFRC522Detector.h"

void RFID::print() const
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

RFID::Encoded RFID::encode() const
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

void RFID::print(const Encoded& encoded) {
  for (size_t i = 0; i < size_t(4 + 4 + 1 + encoded[8]); i++) {
    if (encoded[i] < 0x10) Serial.print('0');
    Serial.print(encoded[i], HEX);
    if (i == 3 || i == 7 || i == 8) Serial.print('-');
  }
}
