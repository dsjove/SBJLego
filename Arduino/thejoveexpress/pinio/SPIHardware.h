#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <assert.h>

// User-overridable macro hook
#ifndef SPI_BEGIN
  #if defined(ARDUINO_ARCH_AVR)
    #define SPI_BEGIN(sck, miso, mosi) SPI.begin()
  #else
    #define SPI_BEGIN(sck, miso, mosi) SPI.begin((sck), (miso), (mosi))
  #endif
#endif

namespace SPIHardware
{
  // Defaults (can be overridden by init)
  static inline uint8_t sck =
#if defined(ARDUINO_AVR_MEGA2560)
      52;
#elif defined(ARDUINO_ARCH_AVR)
      13;
#elif defined(SCK)
      static_cast<uint8_t>(SCK);
#else
      0;
#endif

  static inline uint8_t miso =
#if defined(ARDUINO_AVR_MEGA2560)
      50;
#elif defined(ARDUINO_ARCH_AVR)
      12;
#elif defined(MISO)
      static_cast<uint8_t>(MISO);
#else
      0;
#endif

  static inline uint8_t mosi =
#if defined(ARDUINO_AVR_MEGA2560)
      51;
#elif defined(ARDUINO_ARCH_AVR)
      11;
#elif defined(MOSI)
      static_cast<uint8_t>(MOSI);
#else
      0;
#endif

  using BeginFn = void (*)(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin);
  inline void defaultBegin(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
  {
    SPI_BEGIN(sckPin, misoPin, mosiPin);
  }
  static inline BeginFn beginFn = &defaultBegin;

  inline bool valid()
  {
    return (sck != miso) && (sck != mosi) && (miso != mosi) && (beginFn != nullptr);
  }

  inline void init(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin, BeginFn fn = nullptr)
  {
    sck  = sckPin;
    miso = misoPin;
    mosi = mosiPin;
    if (fn) beginFn = fn;
  }

  inline void begin()
  {
    static bool begun = false;
    if (begun) return;

    if (!valid())
    {
      Serial.println("[SPI] Invalid config. Call SPIHardware::init(sck,miso,mosi[,beginFn]) in setup().");
      assert(false);
      return;
    }

    beginFn(sck, miso, mosi);

    begun = true;
  }
}
