#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <assert.h>

#ifndef SPI_BEGIN
  #if defined(ARDUINO_ARCH_AVR)
    #define SPI_BEGIN(sck, miso, mosi) SPI.begin()
  #else
    #define SPI_BEGIN(sck, miso, mosi) SPI.begin((sck), (miso), (mosi))
  #endif
#endif

namespace SPIHardware
{
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

  inline void init(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
  {
    sck  = sckPin;
    miso = misoPin;
    mosi = mosiPin;
  }

  inline bool valid()
  {
    return (sck != miso) && (sck != mosi) && (miso != mosi);
  }

  inline void begin()
  {
    static bool begun = false;
    if (begun) return;

    if (!valid())
    {
      Serial.println("[SPI] Invalid config. Call SPIHardware::init(sck,miso,mosi) in setup().");
      assert(false);
      return;
    }

    SPI_BEGIN(sck, miso, mosi);

    begun = true;
  }
}
