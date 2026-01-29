// SPIHardware.h
#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <assert.h>

namespace SPIHardware
{
  struct Config
  {
    uint8_t sck  = 0;
    uint8_t miso = 0;
    uint8_t mosi = 0;

    // Default: platform defaults (all conditional compilation lives here)
    constexpr Config()
    {
      // Prefer core-provided SPI pin macros when available.
#if defined(SCK) && defined(MISO) && defined(MOSI)
      sck  = static_cast<uint8_t>(SCK);
      miso = static_cast<uint8_t>(MISO);
      mosi = static_cast<uint8_t>(MOSI);

#elif defined(ARDUINO_ARCH_AVR)
      // AVR Uno/Nano/Pro Mini: D13/D12/D11
      sck  = 13;
      miso = 12;
      mosi = 11;

#elif defined(ARDUINO_AVR_MEGA2560)
      // Mega2560: 52/50/51
      sck  = 52;
      miso = 50;
      mosi = 51;

#else
      // Unknown platform: must be set by .ino via SPIHardware::init(...)
      sck  = 0;
      miso = 0;
      mosi = 0;
#endif
    }

    // Per-value constructor (no conditional compilation here)
    constexpr Config(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
      : sck(sckPin), miso(misoPin), mosi(mosiPin)
    {}

    constexpr bool valid() const
    {
      return sck != 0 && miso != 0 && mosi != 0;
    }
  };

  // Shared inline config. May be overridden in .ino during setup().
  static inline Config config{};

  // Optional .ino helper: explicitly set pins.
  inline void init(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
  {
    config = Config(sckPin, misoPin, mosiPin);
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
      Serial.println("[SPI] Config invalid. Call SPIHardware::init(sck,miso,mosi) in setup().");
      assert(false);
      return;
    }

    SPI.begin(config.sck, config.miso, config.mosi);
    begun = true;
  }
}
