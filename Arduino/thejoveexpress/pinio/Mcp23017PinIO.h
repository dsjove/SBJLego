#pragma once

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

#include "PinIO.h"

// ============================================================================
// Mcp23017PinIO
// - Template parameter controls runtime readiness checking
//   * CheckReady = false (default): assertReady() is a no-op (always true)
//   * CheckReady = true:  assertReady() checks dev != nullptr
// ============================================================================
template <bool CheckReady = false>
struct Mcp23017PinIO
{
  static constexpr bool pin_supports_analog(int /*pin*/) { return false; }
  static constexpr bool pin_supports_pwm(int /*pin*/)    { return false; }
  static constexpr bool pin_is_reserved(int /*pin*/)     { return false; }

  static constexpr bool isValidPinNumber(int pin)
  {
    return pin >= 0 && pin <= 15;
  }

  static inline Adafruit_MCP23X17* dev = nullptr;

  static void attach(Adafruit_MCP23X17& d) { dev = &d; }

  // --------------------------------------------------------------------------
  // Soft readiness assertion hook
  // --------------------------------------------------------------------------
  static bool assertReady()
  {
    if constexpr (!CheckReady)
    {
      return true;
    }
    else
    {
      return dev != nullptr;
    }
  }

  static void begin_digital_in(uint8_t pin)        { dev->pinMode(pin, INPUT); }
  static void begin_digital_in_pullup(uint8_t pin) { dev->pinMode(pin, INPUT_PULLUP); }
  static void begin_analog_in(uint8_t /*pin*/)     {}
  static void begin_digital_out(uint8_t pin)       { dev->pinMode(pin, OUTPUT); }
  static void begin_pwm_out(uint8_t /*pin*/)       {}

  static GpioLevel read_digital(uint8_t pin)
  {
    return dev->digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }

  static GpioArchTypes::analog_type read_analog(uint8_t /*pin*/)
  {
    return 0;
  }

  static void write_digital(uint8_t pin, GpioLevel v)
  {
    dev->digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }

  static void write_pwm(uint8_t /*pin*/, GpioArchTypes::pwm_type /*v*/) {}
};
