#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <assert.h>

// User-overridable macro hook
#ifndef I2C_BEGIN
  #if defined(ARDUINO_ARCH_AVR)
    #define I2C_BEGIN(sda, scl) Wire.begin()
  #else
    #define I2C_BEGIN(sda, scl) Wire.begin((sda), (scl))
  #endif
#endif

namespace I2CHardware
{
  // Defaults (can be overridden by init)
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

  using BeginFn = void (*)(uint8_t sdaPin, uint8_t sclPin);
  inline void defaultBegin(uint8_t sdaPin, uint8_t sclPin)
  {
    I2C_BEGIN(sdaPin, sclPin);
  }
  static inline BeginFn beginFn = &defaultBegin;

  inline bool valid()
  {
    return (clock != 0) && (sda != scl) && (beginFn != nullptr);
  }

  inline void init(uint8_t sdaPin, uint8_t sclPin, uint32_t hz, BeginFn fn = nullptr)
  {
    sda = sdaPin;
    scl = sclPin;
    clock = hz;
    if (fn) beginFn = fn;
  }

  inline void begin()
  {
    static bool begun = false;
    if (begun) return;

    if (!valid())
    {
      Serial.println("[I2C] Invalid config. Call I2CHardware::init(sda,scl,clock[,beginFn]) in setup().");
      assert(false);
      return;
    }

    beginFn(sda, scl);
    Wire.setClock(clock);

    begun = true;
  }
}
