#pragma once

#include <Arduino.h>
#include <type_traits>

// -------------------- Semantic digital level --------------------
enum class GpioLevel : uint8_t
{
  Low  = 0,
  High = 1
};

// -------------------- Pin modes --------------------
enum class GpioMode : uint8_t
{
  DigitalIn,
  DigitalInPullup,
  AnalogIn,
  DigitalOut,
  PWMOut,
  Reserved
};

// -------------------- Architecture-dependent scalar types --------------------
struct ArchTypes
{
  using digital_type = GpioLevel;

#if defined(ARDUINO_ARCH_AVR)
  using analog_type = uint16_t;
  using pwm_type    = uint8_t;

#elif defined(ARDUINO_ARCH_RENESAS)
  using analog_type = uint16_t;
  using pwm_type    = uint8_t;

#elif defined(ARDUINO_ARCH_ESP32)
  using analog_type = uint16_t;
  using pwm_type    = uint16_t;

#else
  using analog_type = uint16_t;
  using pwm_type    = uint16_t;
#endif
};

// -------------------- Mode traits --------------------
template <GpioMode>
struct GpioModeTraits;

// ---- Reserved (no ownership, no operations) ----
template <>
struct GpioModeTraits<GpioMode::Reserved>
{
  using value_type = void;

  static constexpr bool beginable = false;
  static constexpr bool readable  = false;
  static constexpr bool writable  = false;

  // Intentionally no begin/read/write
};

// ---- Digital input ----
template <>
struct GpioModeTraits<GpioMode::DigitalIn>
{
  using value_type = ArchTypes::digital_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin)
  {
    pinMode(pin, INPUT);
  }

  static value_type read(uint8_t pin)
  {
    return digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }
};

// ---- Digital input w/ pullup ----
template <>
struct GpioModeTraits<GpioMode::DigitalInPullup>
{
  using value_type = ArchTypes::digital_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin)
  {
    pinMode(pin, INPUT_PULLUP);
  }

  static value_type read(uint8_t pin)
  {
    return digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }
};

// ---- Analog input ----
template <>
struct GpioModeTraits<GpioMode::AnalogIn>
{
  using value_type = ArchTypes::analog_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t /*pin*/)
  {
    // no GpioMode required
  }

  static value_type read(uint8_t pin)
  {
    return static_cast<value_type>(analogRead(pin));
  }
};

// ---- Digital output ----
template <>
struct GpioModeTraits<GpioMode::DigitalOut>
{
  using value_type = ArchTypes::digital_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = false;
  static constexpr bool writable  = true;

  static void begin(uint8_t pin)
  {
    pinMode(pin, OUTPUT);
  }

  static void write(uint8_t pin, value_type v)
  {
    digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }
};

// ---- PWM output ----
template <>
struct GpioModeTraits<GpioMode::PWMOut>
{
  using value_type = ArchTypes::pwm_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = false;
  static constexpr bool writable  = true;

  static void begin(uint8_t pin)
  {
    pinMode(pin, OUTPUT);
  }

  static void write(uint8_t pin, value_type v)
  {
    analogWrite(pin, v);
  }
};

template <uint8_t PIN, GpioMode MODE>
struct PinIO
{
  static constexpr uint8_t pin = PIN;
  static constexpr GpioMode mode = MODE;

  using Traits = GpioModeTraits<MODE>;
  using value_type = typename Traits::value_type;

  // begin() only when beginable
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M>::beginable, int> = 0>
  static void begin()
  {
    Traits::begin(PIN);
  }

  // read() only when readable
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M>::readable, int> = 0>
  static value_type read()
  {
    return Traits::read(PIN);
  }

  // write() only when writable
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M>::writable, int> = 0>
  static void write(value_type v)
  {
    Traits::write(PIN, v);
  }
};
