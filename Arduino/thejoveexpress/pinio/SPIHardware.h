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

class SPIHardware
{
public:
  using BeginFn = void (*)(int sckPin, int misoPin, int mosiPin);

  // Call init if defaults do not work.
  // Default values come from platform macros and Arch knowledge.
  static void init(int sckPin, int misoPin, int mosiPin, BeginFn fn = nullptr)
  {
    auto& s = state();
    assert(!s.begun && "SPI init() after begin()");
    s.sck  = sckPin;
    s.miso = misoPin;
    s.mosi = mosiPin;
    if (fn) s.beginFn = fn;
  }

  static bool valid()
  {
    return state().valid();
  }

  // Safe to call multiple times.
  static void begin()
  {
    auto& s = state();
    if (s.begun) return;

    if (!s.valid())
    {
      Serial.println("[SPI] Invalid config. Call SPIHardware::init(sck,miso,mosi[,beginFn]) in setup().");
      assert(false);
      return;
    }

    s.beginFn(s.sck, s.miso, s.mosi);
    s.begun = true;
  }

#ifdef UNIT_TEST
  static void resetForTests()
  {
    state() = State{};
  }
#endif

private:
  static void defaultBegin(int sckPin, int misoPin, int mosiPin)
  {
    SPI_BEGIN(sckPin, misoPin, mosiPin);
  }

  struct State
  {
    int sck =
#if defined(ARDUINO_AVR_MEGA2560)
        52;
#elif defined(ARDUINO_ARCH_AVR)
        13;
#elif defined(SCK)
        static_cast<int>(SCK);
#else
        -1;
#endif

    int miso =
#if defined(ARDUINO_AVR_MEGA2560)
        50;
#elif defined(ARDUINO_ARCH_AVR)
        12;
#elif defined(MISO)
        static_cast<int>(MISO);
#else
        -1;
#endif

    int mosi =
#if defined(ARDUINO_AVR_MEGA2560)
        51;
#elif defined(ARDUINO_ARCH_AVR)
        11;
#elif defined(MOSI)
        static_cast<int>(MOSI);
#else
        -1;
#endif

    BeginFn beginFn = &defaultBegin;
    bool begun = false;

    constexpr bool valid() const
    {
      return (sck != miso) &&
             (sck != mosi) &&
             (miso != mosi) &&
             (sck != -1) &&
             (miso != -1) &&
             (mosi != -1) &&
             (beginFn != nullptr);
    }
  };

  static State& state()
  {
    static State s{};
    return s;
  }
};
