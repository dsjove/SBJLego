#pragma once

#include <Arduino.h>
#include <type_traits>

// ============================================================================
// PinIO
// - PIN is int so -1 can represent a disabled pin
// - Disabled is valid and results in no-ops
// - Any other invalid pin is a compile-time error
// - Backend owns pin validity via isValidPinNumber(int)
// ============================================================================

// Project-local disabled pin constant.
// Arduino's NOT_A_PIN exists but its numeric value is inconsistent across cores.
inline constexpr int PINIO_DISABLED_PIN = -1;

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

  // Optional hook
  static constexpr bool ready() { return true; }

  // Valid if non-negative. -1 is handled by PinIO as "disabled".
  static constexpr bool isValidPinNumber(int pin)
  {
    return pin >= 0;
  }

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
      return static_cast<bool>(B::ready());
    else
      return true;
  }
}

// ============================================================================
// PinIO
// ============================================================================
template <int PIN, GpioMode MODE, typename Backend = ArduinoGpioBackend>
struct PinIO
{
  static constexpr int pin   = PIN;
  static constexpr GpioMode mode = MODE;

  static constexpr bool is_disabled = (PIN == PINIO_DISABLED_PIN);
  static constexpr bool enabled     = !is_disabled;

  // Hard rule:
  //  - disabled (-1) is always allowed
  //  - otherwise the backend must accept the pin
  static_assert(
    is_disabled || Backend::isValidPinNumber(PIN),
    "Pin number is not valid for this backend (use PINIO_DISABLED_PIN to disable)"
  );

  using Traits     = GpioModeTraits<MODE, Backend>;
  using value_type = typename Traits::value_type;

private:
  static constexpr bool wants_analog = (MODE == GpioMode::AnalogIn);
  static constexpr bool wants_pwm    = (MODE == GpioMode::PWMOut);

  // IMPORTANT:
  // This must be well-formed even when PIN is disabled, because some toolchains
  // will still instantiate enough of the function body to diagnose errors.
  static constexpr uint8_t u8pin()
  {
    if constexpr (is_disabled)
    {
      // Not used (all public ops early-return when disabled), but must compile.
      return 0;
    }
    else
    {
      static_assert(PIN >= 0 && PIN <= 255, "Pin number out of uint8_t range");
      return static_cast<uint8_t>(PIN);
    }
  }

public:
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::beginable, int> = 0>
  static void begin()
  {
    if constexpr (!enabled) { return; }

    if (!pinio_detail::backend_ready<Backend>())
      return;

    static_assert(!(wants_analog && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    static_assert(!(wants_pwm && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");

    Traits::begin(u8pin());
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<
      GpioModeTraits<M, Backend>::beginable &&
      GpioModeTraits<M, Backend>::writable,
      int> = 0>
  static void begin(typename GpioModeTraits<M, Backend>::value_type initial)
  {
    if constexpr (!enabled) { return; }

    if (!pinio_detail::backend_ready<Backend>())
      return;

    static_assert(!(wants_analog && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    static_assert(!(wants_pwm && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");

    Traits::begin(u8pin());
    GpioModeTraits<M, Backend>::write(u8pin(), initial);
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::readable, int> = 0>
  static typename GpioModeTraits<M, Backend>::value_type read()
  {
    if constexpr (!enabled) { return {}; }

    if (!pinio_detail::backend_ready<Backend>())
      return {};

    static constexpr bool wants_analog_m = (M == GpioMode::AnalogIn);
    static_assert(!(wants_analog_m && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");

    return GpioModeTraits<M, Backend>::read(u8pin());
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::writable, int> = 0>
  static void write(typename GpioModeTraits<M, Backend>::value_type v)
  {
    if constexpr (!enabled) { return; }

    if (!pinio_detail::backend_ready<Backend>())
      return;

    static constexpr bool wants_pwm_m = (M == GpioMode::PWMOut);
    static_assert(!(wants_pwm_m && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");

    GpioModeTraits<M, Backend>::write(u8pin(), v);
  }
};
