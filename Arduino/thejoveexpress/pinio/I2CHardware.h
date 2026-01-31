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

class I2CHardware
{
public:
  using BeginFn = void (*)(int sdaPin, int sclPin);

  // Call init if defaults do not work.
  static void init(int sdaPin, int sclPin, uint32_t hz, BeginFn fn = nullptr)
  {
    auto& s = state();
    assert(!s.begun && "I2C init() after begin()");
    s.sda   = sdaPin;
    s.scl   = sclPin;
    s.clock = hz;
    if (fn) s.beginFn = fn;
  }

  static bool valid()
  {
    return state().valid();
  }

  static void begin()
  {
    auto& s = state();
    if (s.begun) return;

    if (!s.valid())
    {
      Serial.println("[I2C] Invalid config. Call I2CHardware::init(sda,scl,clock[,beginFn]) in setup().");
      assert(false);
      return;
    }

    s.beginFn(s.sda, s.scl);
    Wire.setClock(s.clock);

    s.begun = true;
  }

#ifdef UNIT_TEST
  static void resetForTests()
  {
    state() = State{};
  }
#endif

private:
  static void defaultBegin(int sdaPin, int sclPin)
  {
    I2C_BEGIN(sdaPin, sclPin);
  }

  struct State
  {
    int sda =
#if defined(SDA)
        static_cast<int>(SDA);
#else
        -1;
#endif

    int scl =
#if defined(SCL)
        static_cast<int>(SCL);
#else
        -1;
#endif

    uint32_t clock =
#if defined(ARDUINO_ARCH_AVR)
        100000UL;
#else
        400000UL;
#endif

    BeginFn beginFn = &defaultBegin;
    bool begun = false;

    constexpr bool valid() const
    {
      return (clock != 0) &&
             (sda != scl) &&
             (sda != -1) &&
             (scl != -1) &&
             (beginFn != nullptr);
    }
  };

  static State& state()
  {
    static State s{};
    return s;
  }
};
