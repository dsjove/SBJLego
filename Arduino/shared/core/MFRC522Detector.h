#pragma once

#include <MFRC522.h>

/*
Hardware:
HiLetgo 3pcs RFID Kit - Mifare RC522 RF IC Card Sensor Module + S50 Blank Card + Key Ring
Timeskey NFC 20 Pack Mifare Classic 1k NFC Tag RFID Sticker 13.56mhz - ISO14443A Smart 25mm Adhesive Tags
 */

class MFRC522Detector {
public:
  struct Timing {
    Timing() {}
    uint32_t taskFrequency = 20; // Use for loop call frequency
    uint32_t cooldownMs = 800; // Tune for tag movement speed
    uint32_t reinitAfterMs = 30000; // MFRC522 goes bad after a while
    uint8_t failResetCount = 5; // Reset after a failure count
  };

  struct RFID {
    using Encoded = std::array<uint8_t, 4 + 4 + 1 + 10>;

    RFID(uint32_t number);
    void update(const MFRC522::Uid& u, uint32_t timestamp);
    void print() const;

    const uint32_t _number;
    uint32_t _timestamp;
    uint8_t _length;
    std::array<uint8_t, 10> _uuid;

    Encoded encode() const;
    size_t encodedSize() const { return 4 + 4 + 1 + _length; }

    static void print(const Encoded& encoded);
  };

  MFRC522Detector(uint32_t number, int ss_pin = 10, int rst_pin = 9, Timing timing = Timing());

  void begin();

  const RFID* loop();

  const Timing& timing() const { return _timing; }
  const RFID& lastID() const { return _lastID; }

private:
  const int _ss_pin;
  const int _rst_pin;
  const Timing _timing;
  MFRC522 _rfid;
  RFID _lastID;
  uint32_t _cooldownLimitMs;
  uint32_t _lastGoodReadMs;
  uint32_t _failReadCount;

  void resetRc522();
};
