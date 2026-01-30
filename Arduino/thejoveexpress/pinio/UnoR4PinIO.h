#pragma once

#include <Arduino.h>

#inlcude "GpioTypes.h"

// PinIO backend for Arduino UNO R4 (Minima / WiFi share the same UNO-style headers).
// Notes:
//  - Digital pins: D0..D13
//  - Analog pins:  A0..A5 map to D14..D19
//  - PWM pins (tilde-marked): D3, D5, D6, D9, D10, D11
struct UnoR4PinIO
{
  // A0..A5 exist on UNO R4 cores; typically A0=14..A5=19.
  static constexpr bool pin_supports_analog(int pin)
  {
#if defined(A0) && defined(A5)
    return (pin >= A0 && pin <= A5) || (pin >= 14 && pin <= 19);
#else
    return (pin >= 14 && pin <= 19);
#endif
  }

  // Arduino API PWM pins for UNO R4 (Minima/WiFi): 3,5,6,9,10,11
  static constexpr bool pin_supports_pwm(int pin)
  {
    return (pin == 3) || (pin == 5) || (pin == 6) || (pin == 9) || (pin == 10) || (pin == 11);
  }

  // Reserve nothing by default (hook point for strap/USB/UART/etc if you want later)
  static constexpr bool pin_is_reserved(int /*pin*/) { return false; }

  // UNO-style numbering: D0..D13, plus A0..A5 (commonly 14..19)
  static constexpr bool isValidPinNumber(int pin)
  {
    return pin >= 0 && pin <= 19;
  }

  // Native backend is always "ready"
  static constexpr bool assertReady() { return true; }

  static void begin_digital_in(uint8_t pin)        { ::pinMode(pin, INPUT); }
  static void begin_digital_in_pullup(uint8_t pin) { ::pinMode(pin, INPUT_PULLUP); }
  static void begin_analog_in(uint8_t /*pin*/)     {}
  static void begin_digital_out(uint8_t pin)       { ::pinMode(pin, OUTPUT); }
  static void begin_pwm_out(uint8_t pin)           { ::pinMode(pin, OUTPUT); }

  static GpioLevel read_digital(uint8_t pin)
  {
    return ::digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }

  static GpioArchTypes::analog_type read_analog(uint8_t pin)
  {
    return static_cast<GpioArchTypes::analog_type>(::analogRead(pin));
  }

  static void write_digital(uint8_t pin, GpioLevel v)
  {
    ::digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }

  static void write_pwm(uint8_t pin, GpioArchTypes::pwm_type v)
  {
    ::analogWrite(pin, v);
  }
};
