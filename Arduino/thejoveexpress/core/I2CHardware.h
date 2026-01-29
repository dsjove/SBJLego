#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <assert.h>

#ifndef I2C_BEGIN
  #if defined(ARDUINO_ARCH_AVR)
    #define I2C_BEGIN(sda, scl) Wire.begin()
  #else
    #define I2C_BEGIN(sda, scl) Wire.begin((sda), (scl))
  #endif
#endif

namespace I2CHardware
{
  static inline uint8_t sda =
#if defined(SDA)
      static_cast<uint8_t>(SDA);
#else
      0;
#endif

  static inline uint8_t scl =
#if defined(SCL)
      static_cast<uint8_t>(SCL);
#else
      0;
#endif

  static inline uint32_t clock =
#if defined(ARDUINO_ARCH_AVR)
      100000UL;
#else
      400000UL;
#endif

  inline void init(uint8_t sdaPin, uint8_t sclPin, uint32_t hz)
  {
    sda = sdaPin;
    scl = sclPin;
    clock = hz;
  }

  inline bool valid()
  {
    return (clock != 0) && (sda != scl);
  }

  inline void begin()
  {
    static bool begun = false;
    if (begun) return;

    if (!valid())
    {
      Serial.println("[I2C] Invalid config. Call I2CHardware::init(sda,scl,clock) in setup().");
      assert(false);
      return;
    }

    I2C_BEGIN(sda, scl);
    Wire.setClock(clock);

    begun = true;
  }
}
