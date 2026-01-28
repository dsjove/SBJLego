#pragma once

#include <MFRC522.h>
#include "PinIO.h"

/*
Hardware:
HiLetgo 3pcs RFID Kit - Mifare RC522 RF IC Card Sensor Module + S50 Blank Card + Key Ring
Timeskey NFC 20 Pack Mifare Classic 1k NFC Tag RFID Sticker 13.56mhz - ISO14443A Smart 25mm Adhesive Tags
 */

struct RFID {
  using Encoded = std::array<uint8_t, 4 + 4 + 1 + 10>;

  RFID(uint8_t number)
  : _number(number)
  , _timestamp(0)
  , _length(0)
  , _uuid({0})
  {
  }

  void update(const MFRC522::Uid& u, uint32_t timestamp)
  {
    _timestamp = timestamp;
    const uint8_t len = (u.size > 10) ? 10 : u.size;
    _length = len;
    std::copy(u.uidByte, u.uidByte + len, _uuid.begin());
  }

  void print() const;

  const uint8_t _number;
  uint32_t _timestamp;
  uint8_t _length;
  std::array<uint8_t, 10> _uuid;

  Encoded encode() const;
  size_t encodedSize() const { return 4 + 4 + 1 + _length; }

  static void print(const Encoded& encoded);
};

struct RFIDDetectorDefaultTraits {
  static constexpr uint8_t Number = 0;
  static constexpr uint8_t SsPin  = 10;
  static constexpr uint8_t RstPin = 9;

  static constexpr uint32_t loopFrequencyMs = 20;     // Recommended Task delay
  static constexpr uint32_t cooldownMs      = 800;    // Tune for tag movement speed
  static constexpr uint32_t reinitAfterMs   = 30000;  // MFRC522 goes bad after a while
  static constexpr uint8_t  failResetCount  = 5;      // Reset after repeated failures
};

template <typename Traits = RFIDDetectorDefaultTraits>
class RFIDDetector {
public:
  static constexpr uint8_t number  = Traits::Number;
  static constexpr uint8_t ss_pin  = Traits::SsPin;
  static constexpr uint8_t rst_pin = Traits::RstPin;

  using Ss  = PinIO<Traits::SsPin, GpioMode::Reserved>;
  using Rst = PinIO<Traits::RstPin, GpioMode::DigitalOut>;

  RFIDDetector()
  : _rfid(Traits::SsPin, Traits::RstPin)
  , _lastID(Traits::Number)
  , _cooldownLimitMs(0)
  , _lastGoodReadMs(0)
  , _failReadCount(0)
  {
  }

  const RFID& lastID() const { return _lastID; }

  void begin()
  {
    Rst::begin();
    Rst::write(GpioLevel::High);
    _rfid.PCD_Init();
  }

  const RFID* loop()
  {
    const uint32_t now = millis();
    // Re-init after long inactivity without a successful read
    if (_lastGoodReadMs != 0 && (now - _lastGoodReadMs) > Traits::reinitAfterMs)
    {
      resetRc522();
      _lastGoodReadMs = now;
    }
    if (_rfid.PICC_IsNewCardPresent())
    {
      // Cooldown gate
      if (now < _cooldownLimitMs)
      {
        return nullptr;
      }
      if (_rfid.PICC_ReadCardSerial())
      {
        _lastGoodReadMs = now;
        _failReadCount = 0;

        _lastID.update(_rfid.uid, now);

        _rfid.PICC_HaltA();
        _rfid.PCD_StopCrypto1();

        _cooldownLimitMs = now + Traits::cooldownMs;
        return &_lastID;
      }
      else
      {
        Serial.println("RFID: Read Failed");
        _rfid.PCD_StopCrypto1();
        _rfid.PICC_HaltA();
        _failReadCount++;
        if (_failReadCount >= Traits::failResetCount)
        {
          Serial.println("RFID: Fail Count Reset");
          resetRc522();
        }
      }
    }
    return nullptr;
  }

private:
  MFRC522 _rfid;
  RFID _lastID;
  uint32_t _cooldownLimitMs;
  uint32_t _lastGoodReadMs;
  uint8_t  _failReadCount;

  void resetRc522()
  {
    Rst::write(GpioLevel::Low);
    delay(5);
    Rst::write(GpioLevel::High);
    delay(5);
    _rfid.PCD_Init();
    _rfid.PCD_SetAntennaGain(_rfid.RxGain_max);
    _failReadCount = 0;
    // Do not reset _lastGoodReadMs
  }
};
