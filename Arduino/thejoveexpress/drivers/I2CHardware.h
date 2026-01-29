// I2CHardware.h
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <assert.h>

namespace I2CHardware
{
  struct Config
  {
    uint8_t  sda   = 0;
    uint8_t  scl   = 0;
    uint32_t clock = 0;

    // Default: platform defaults (all conditional compilation lives here)
    constexpr Config()
    {
#if defined(SDA) && defined(SCL)
      sda = static_cast<uint8_t>(SDA);
      scl = static_cast<uint8_t>(SCL);
#else
      // Unknown platform: must be set by .ino via I2CHardware::init(...)
      sda = 0;
      scl = 0;
#endif

#if defined(ARDUINO_ARCH_AVR)
      clock = 100000UL;
#else
      clock = 400000UL;
#endif
    }

    // Per-value constructor (no conditional compilation here)
    constexpr Config(uint8_t sdaPin, uint8_t sclPin, uint32_t hz)
      : sda(sdaPin), scl(sclPin), clock(hz)
    {}

    constexpr bool valid() const
    {
      return sda != 0 && scl != 0 && clock != 0;
    }
  };

  // Shared inline config. May be overridden in .ino during setup().
  static inline Config config{};

  // Optional .ino helper: explicitly set pins/clock.
  inline void init(uint8_t sdaPin, uint8_t sclPin, uint32_t hz)
  {
    config = Config(sdaPin, sclPin, hz);
  }

  inline void begin()
  {
    static bool begun = false;
    if (begun)
    {
      return;
    }

    // Contract: config must be valid by setup() time.
    if (!config.valid())
    {
      Serial.println("[I2C] Config invalid. Call I2CHardware::init(sda,scl,clock) in setup().");
      assert(false);
      return;
    }

    Wire.begin(config.sda, config.scl);
    Wire.setClock(config.clock);

    begun = true;
  }
}
