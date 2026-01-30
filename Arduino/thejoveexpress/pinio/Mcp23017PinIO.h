#pragma once

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

#include "PinIO.h"

// ============================================================================
// Mcp23017PinIO
// - Template parameter controls runtime readiness checking
//   * CheckReady = false (default): verifyReady() is a no-op (always true)
//   * CheckReady = true:  verifyReady() checks dev != nullptr
// ============================================================================
template <bool CheckReady = false>
struct Mcp23017PinIO
{
  // If not using a Mcp23017Device, this must be called.
  // Do not call any begin, read, or write until this method is called.
  static void attach(Adafruit_MCP23X17& d) { dev = &d; }

  static constexpr bool pin_exists(int pin)      { return pin >= 0 && pin <= 15; }
  static constexpr bool pin_is_reserved(int)     { return false; }
  static constexpr bool pin_supports_analog(int) { return false; }
  static constexpr bool pin_supports_pwm(int)    { return false; }

  static bool verifyReady()
  {
    if constexpr (!CheckReady) return true; else return dev != nullptr;
  }

  static void begin_digital_in(uint8_t pin)        { dev->pinMode(pin, INPUT); }
  static void begin_digital_in_pullup(uint8_t pin) { dev->pinMode(pin, INPUT_PULLUP); }
  static void begin_digital_out(uint8_t pin)       { dev->pinMode(pin, OUTPUT); }

  static GpioLevel read_digital(uint8_t pin)
  {
    return dev->digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }

  static void write_digital(uint8_t pin, GpioLevel v)
  {
    dev->digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }

private:
  static inline Adafruit_MCP23X17* dev = nullptr;
};
