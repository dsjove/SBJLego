#pragma once

#include <Arduino.h>
#include <type_traits>

#include "GpioTypes.h"

#if defined(ARDUINO)
// Arduino build
  #include <Arduino.h>
  #include "ArduinoGpioBackend.h"
  using DefaultPinIOBackend = ArduinoGpioBackend;
#elif defined(__linux__)
// Linux build (Raspberry Pi)
  #include <cstdint>
  #include "RaspberryPiGpioBackend.h"
  using DefaultPinIOBackend = RaspberryPiGpioBackend;
#else
  #error "PinIO: Unsupported platform. Expected ARDUINO or __linux__."
#endif

// ============================================================================
// Mode traits
// ============================================================================
template <GpioMode, typename Backend>
struct GpioModeTraits;

template <typename Backend>
struct GpioModeTraits<GpioMode::Delegated, Backend>
{
  using value_type = GpioArchTypes::empty_type;
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
// Pin legality checks (type-level)
// ============================================================================
namespace pinio_detail
{
  template <typename Backend, int Pin, GpioMode Mode>
  static consteval bool pin_mode_allowed()
  {
    // Delegated means "PinIO is not responsible for validating this pin/mode".
    if constexpr (Mode == GpioMode::Delegated)
    {
      return true;
    }

    // Disabled pins are always allowed for any non-delegated mode (becomes no-op).
    if constexpr (Pin == PINIO_DISABLED_PIN)
    {
      return true;
    }

    // Hard disallow reserved pins for non-delegated use.
    if constexpr (Backend::pin_is_reserved(Pin))
    {
      return false;
    }

    // Mode-specific capabilities.
    if constexpr (Mode == GpioMode::AnalogIn)
    {
      return Backend::pin_supports_analog(Pin);
    }

    if constexpr (Mode == GpioMode::PWMOut)
    {
      return Backend::pin_supports_pwm(Pin);
    }

    // DigitalIn / DigitalInPullup / DigitalOut: OK if pin is otherwise valid.
    return true;
  }
}

// ============================================================================
// PinIO
// ============================================================================
template <int PIN, GpioMode MODE, typename Backend = DefaultPinIOBackend>
struct PinIO
{
  static constexpr int pin = PIN;
  static constexpr GpioMode mode = MODE;

  static constexpr bool is_disabled = (PIN == PINIO_DISABLED_PIN);
  static constexpr bool enabled     = !is_disabled;

  static_assert(
    (MODE == GpioMode::Delegated) || is_disabled || Backend::pin_exists(PIN),
    "Pin number is not valid for this backend (use PINIO_DISABLED_PIN to disable)"
  );

  static_assert(
    pinio_detail::pin_mode_allowed<Backend, PIN, MODE>(),
    "Pin has illegal mode for this backend"
  );

  using Traits     = GpioModeTraits<MODE, Backend>;
  using value_type = typename Traits::value_type;

private:
  static constexpr uint8_t u8pin()
  {
    if constexpr (is_disabled)
    {
      return 0; // Never executed but requred for compilation.
    }
    else
    {
      static_assert(PIN >= 0 && PIN <= 255, "Pin number out of uint8_t range");
      return static_cast<uint8_t>(PIN);
    }
  }

public:
  // Readiness query
  static bool isReady()
  {
    if constexpr (is_disabled)
      return true;

    return Backend::verifyReady();
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::beginable, int> = 0>
  static void begin()
  {
    if constexpr (is_disabled) { return; }
    if (!Backend::verifyReady()) { return; }

    Traits::begin(u8pin());
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::beginable &&
                              GpioModeTraits<M, Backend>::writable, int> = 0>
  static void begin(typename GpioModeTraits<M, Backend>::value_type initial)
  {
    if constexpr (is_disabled) { return; }
    if (!Backend::verifyReady()) { return; }

    Traits::begin(u8pin());
    GpioModeTraits<M, Backend>::write(u8pin(), initial);
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::readable, int> = 0>
  static typename GpioModeTraits<M, Backend>::value_type read()
  {
    if constexpr (is_disabled) { return {}; }
    if (!Backend::verifyReady()) { return {}; }

    return GpioModeTraits<M, Backend>::read(u8pin());
  }

  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::writable, int> = 0>
  static void write(typename GpioModeTraits<M, Backend>::value_type v)
  {
    if constexpr (is_disabled) { return; }
    if (!Backend::verifyReady()) { return; }

    GpioModeTraits<M, Backend>::write(u8pin(), v);
  }

  template <
     GpioMode M = MODE,
     typename std::enable_if_t<(M == GpioMode::PWMOut) &&
                                GpioModeTraits<M, Backend>::writable, int> = 0>
  static void writeScaled(uint32_t value, uint32_t scaleMax)
  {
    using pwm_type = typename GpioModeTraits<M, Backend>::value_type;
    if (!Backend::verifyReady()) return;

    // Avoid divide-by-zero
    if (scaleMax == 0) {
      write(static_cast<pwm_type>(0));
      return;
    }
    const uint32_t maxv = static_cast<uint32_t>(Backend::pwmMax(PIN));
    // Clamp input
    if (value >= scaleMax) {
      write(static_cast<pwm_type>(maxv));
      return;
    }
    // Map [0..scaleMax] -> [0..maxv], rounded
    const uint32_t mapped = (value * maxv + (scaleMax / 2)) / scaleMax;
    write(static_cast<pwm_type>(mapped));
  }

  template <GpioMode M = MODE,
  typename std::enable_if_t<(M == GpioMode::PWMOut) &&
                             GpioModeTraits<M, Backend>::writable, int> = 0>
  static void writeNormalized(float x)
  {
    using pwm_type = typename GpioModeTraits<M, Backend>::value_type;
    if (!Backend::verifyReady()) return;
    
    if (x <= 0.0f) { write(static_cast<pwm_type>(0)); return; }
    if (x >= 1.0f) { write(Backend::pwmMax(PIN)); return; }
    // Use scaled to avoid duplicating mapping logic.
    // 65535 gives decent resolution without huge overflow risk in the multiply.
    constexpr uint32_t SCALE = 65535u;
    const uint32_t v = static_cast<uint32_t>(x * static_cast<float>(SCALE) + 0.5f);
    writeScaled(v, SCALE);
  }
};
