#pragma once

#include <Arduino.h>
#include <type_traits>

// ==================== Semantic digital level ====================
enum class GpioLevel : uint8_t
{
  Low  = 0,
  High = 1
};

// ==================== Pin modes ====================
enum class GpioMode : uint8_t
{
  DigitalIn,
  DigitalInPullup,
  AnalogIn,
  DigitalOut,
  PWMOut,
  Delegated
};

// ==================== Architecture-dependent scalar types ====================
struct GpioArchTypes
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

// -------------------- Native Arduino backend --------------------
struct ArduinoGpioBackend
{
  static constexpr bool supports_analog = true;
  static constexpr bool supports_pwm    = true;

  // Optional hook (PinIO will call if present)
  static constexpr bool ready() { return true; }

  static void begin_digital_in(uint8_t pin)        { pinMode(pin, INPUT); }
  static void begin_digital_in_pullup(uint8_t pin) { pinMode(pin, INPUT_PULLUP); }
  static void begin_analog_in(uint8_t /*pin*/)     { /* no-op */ }
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

// ============================================================================
// Mode traits
// ============================================================================
template <GpioMode, typename Backend>
struct GpioModeTraits;

template <typename Backend>
struct GpioModeTraits<GpioMode::Delegated, Backend>
{
  using value_type = void;
  static constexpr bool beginable = false;
  static constexpr bool readable  = false;
  static constexpr bool writable  = false;
};

template <typename Backend>
struct GpioModeTraits<GpioMode::DigitalIn, Backend>
{
  using value_type = GpioArchTypes::digital_type;
  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin) { Backend::begin_digital_in(pin); }
  static value_type read(uint8_t pin) { return Backend::read_digital(pin); }
};

template <typename Backend>
struct GpioModeTraits<GpioMode::DigitalInPullup, Backend>
{
  using value_type = GpioArchTypes::digital_type;
  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin) { Backend::begin_digital_in_pullup(pin); }
  static value_type read(uint8_t pin) { return Backend::read_digital(pin); }
};

template <typename Backend>
struct GpioModeTraits<GpioMode::AnalogIn, Backend>
{
  using value_type = GpioArchTypes::analog_type;
  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin) { Backend::begin_analog_in(pin); }
  static value_type read(uint8_t pin) { return Backend::read_analog(pin); }
};

template <typename Backend>
struct GpioModeTraits<GpioMode::DigitalOut, Backend>
{
  using value_type = GpioArchTypes::digital_type;
  static constexpr bool beginable = true;
  static constexpr bool readable  = false;
  static constexpr bool writable  = true;

  static void begin(uint8_t pin) { Backend::begin_digital_out(pin); }
  static void write(uint8_t pin, value_type v) { Backend::write_digital(pin, v); }
};

template <typename Backend>
struct GpioModeTraits<GpioMode::PWMOut, Backend>
{
  using value_type = GpioArchTypes::pwm_type;
  static constexpr bool beginable = true;
  static constexpr bool readable  = false;
  static constexpr bool writable  = true;

  static void begin(uint8_t pin) { Backend::begin_pwm_out(pin); }
  static void write(uint8_t pin, value_type v) { Backend::write_pwm(pin, v); }
};

// ============================================================================
// Backend readiness detection (optional hook)
// ============================================================================
namespace pinio_detail
{
  template <typename B, typename = void>
  struct has_ready : std::false_type {};

  template <typename B>
  struct has_ready<B, std::void_t<decltype(B::ready())>> : std::true_type {};

  template <typename B>
  static constexpr bool backend_ready()
  {
    if constexpr (has_ready<B>::value)
    {
      return static_cast<bool>(B::ready());
    }
    else
    {
      return true;
    }
  }
}

// ============================================================================
// PinIO
// ============================================================================
template <uint8_t PIN, GpioMode MODE, typename Backend = ArduinoGpioBackend>
struct PinIO
{
  static constexpr uint8_t pin   = PIN;
  static constexpr GpioMode mode = MODE;

  using Traits     = GpioModeTraits<MODE, Backend>;
  using value_type = typename Traits::value_type;

private:
  static constexpr bool wants_analog = (MODE == GpioMode::AnalogIn);
  static constexpr bool wants_pwm    = (MODE == GpioMode::PWMOut);

public:
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::beginable, int> = 0>
  static void begin()
  {
    if (!pinio_detail::backend_ready<Backend>())
    {
      return;
    }

    static_assert(!(wants_analog && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    static_assert(!(wants_pwm && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");

    Traits::begin(PIN);
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<
      GpioModeTraits<M, Backend>::beginable &&
      GpioModeTraits<M, Backend>::writable,
      int> = 0>
  static void begin(typename GpioModeTraits<M, Backend>::value_type initial)
  {
    if (!pinio_detail::backend_ready<Backend>())
    {
      return;
    }

    static_assert(!(wants_analog && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    static_assert(!(wants_pwm && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");

    Traits::begin(PIN);
    GpioModeTraits<M, Backend>::write(PIN, initial);
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::readable, int> = 0>
  static typename GpioModeTraits<M, Backend>::value_type read()
  {
    if (!pinio_detail::backend_ready<Backend>())
    {
      // For “not ready”, return a sensible zero value for the mode.
      // You can also choose to assert/abort in debug builds.
      return {};
    }

    static constexpr bool wants_analog_m = (M == GpioMode::AnalogIn);
    static_assert(!(wants_analog_m && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    return GpioModeTraits<M, Backend>::read(PIN);
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::writable, int> = 0>
  static void write(typename GpioModeTraits<M, Backend>::value_type v)
  {
    if (!pinio_detail::backend_ready<Backend>())
    {
      return;
    }

    static constexpr bool wants_pwm_m = (M == GpioMode::PWMOut);
    static_assert(!(wants_pwm_m && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");
    GpioModeTraits<M, Backend>::write(PIN, v);
  }
};
