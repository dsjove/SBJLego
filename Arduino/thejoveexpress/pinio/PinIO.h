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
  // Per-pin capabilities / reservations
  // - Keep these constexpr so PinIO can SFINAE away unsupported APIs.
  static constexpr bool pin_supports_analog(int /*pin*/) { return true; }
  static constexpr bool pin_supports_pwm(int /*pin*/)    { return true; }
  static constexpr bool pin_is_reserved(int /*pin*/)     { return false; }

  // Soft readiness assertion hook (all backends provide this).
  // Return false to make PinIO operations no-op for this call site.
  static constexpr bool assertReady() { return true; }

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
// Backend pin capability helpers
// ============================================================================
namespace pinio_detail
{
  template <typename Backend, int Pin>
  static constexpr bool pin_allowed()
  {
    // Disabled pins are always allowed and compile (operations become no-ops).
    if constexpr (Pin == PINIO_DISABLED_PIN)
      return true;
    else
      return !Backend::pin_is_reserved(Pin);
  }

  template <typename Backend, int Pin>
  static constexpr bool pin_supports_analog()
  {
    if constexpr (Pin == PINIO_DISABLED_PIN)
      return true;
    else
      return Backend::pin_supports_analog(Pin);
  }

  template <typename Backend, int Pin>
  static constexpr bool pin_supports_pwm()
  {
    if constexpr (Pin == PINIO_DISABLED_PIN)
      return true;
    else
      return Backend::pin_supports_pwm(Pin);
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

  static_assert(
    pinio_detail::pin_allowed<Backend, PIN>(),
    "Pin is reserved for this backend"
  );

  using Traits     = GpioModeTraits<MODE, Backend>;
  using value_type = typename Traits::value_type;

private:
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
    typename std::enable_if_t<
      GpioModeTraits<M, Backend>::beginable &&
      (M != GpioMode::AnalogIn || pinio_detail::pin_supports_analog<Backend, PIN>()) &&
      (M != GpioMode::PWMOut   || pinio_detail::pin_supports_pwm<Backend, PIN>()),
      int> = 0>
  static void begin()
  {
    if constexpr (!enabled) { return; }

    if (!Backend::assertReady())
      return;

    Traits::begin(u8pin());
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<
      GpioModeTraits<M, Backend>::beginable &&
      GpioModeTraits<M, Backend>::writable &&
      (M != GpioMode::AnalogIn || pinio_detail::pin_supports_analog<Backend, PIN>()) &&
      (M != GpioMode::PWMOut   || pinio_detail::pin_supports_pwm<Backend, PIN>()),
      int> = 0>
  static void begin(typename GpioModeTraits<M, Backend>::value_type initial)
  {
    if constexpr (!enabled) { return; }

    if (!Backend::assertReady())
      return;

    Traits::begin(u8pin());
    GpioModeTraits<M, Backend>::write(u8pin(), initial);
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<
      GpioModeTraits<M, Backend>::readable &&
      (M != GpioMode::AnalogIn || pinio_detail::pin_supports_analog<Backend, PIN>()),
      int> = 0>
  static typename GpioModeTraits<M, Backend>::value_type read()
  {
    if constexpr (!enabled) { return {}; }

    if (!Backend::assertReady())
      return {};

    return GpioModeTraits<M, Backend>::read(u8pin());
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<
      GpioModeTraits<M, Backend>::writable &&
      (M != GpioMode::PWMOut || pinio_detail::pin_supports_pwm<Backend, PIN>()),
      int> = 0>
  static void write(typename GpioModeTraits<M, Backend>::value_type v)
  {
    if constexpr (!enabled) { return; }

    if (!Backend::assertReady())
      return;

    GpioModeTraits<M, Backend>::write(u8pin(), v);
  }
};
