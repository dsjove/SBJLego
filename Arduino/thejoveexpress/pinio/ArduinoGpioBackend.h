#pragma once

#include <Arduino.h>
#include <type_traits>

#include "GpioTypes.h"

struct ArduinoGpioBackend
{
  static constexpr bool pin_supports_analog(int /*pin*/) { return true; }
  static constexpr bool pin_supports_pwm(int /*pin*/)    { return true; }
  static constexpr bool pin_is_reserved(int /*pin*/)     { return false; }

  static constexpr bool assertReady() { return true; }

  static constexpr bool isValidPinNumber(int pin) { return pin >= 0; }

  static void begin_digital_in(uint8_t pin)        { pinMode(pin, INPUT); }
  static void begin_digital_in_pullup(uint8_t pin) { pinMode(pin, INPUT_PULLUP); }
  static void begin_analog_in(uint8_t /*pin*/)     {}
  static void begin_digital_out(uint8_t pin)       { pinMode(pin, OUTPUT); }
  static void begin_pwm_out(uint8_t pin)           { pinMode(pin, OUTPUT); }

  static GpioLevel read_digital(uint8_t pin)
  {
    return digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }

  static GpioArchTypes::analog_type read_analog(uint8_t pin)
  {
    return static_cast<GpioArchTypes::analog_type>(analogRead(pin));
  }

  static void write_digital(uint8_t pin, GpioLevel v)
  {
    digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }

  static void write_pwm(uint8_t pin, GpioArchTypes::pwm_type v)
  {
    analogWrite(pin, v);
  }
};
