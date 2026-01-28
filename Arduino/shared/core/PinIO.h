// PinIO.h
#pragma once

#include <Arduino.h>
#include <Wire.h>
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
  Reserved
};

// ==================== Architecture-dependent scalar types ====================
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

// -------------------- Native Arduino backend --------------------
struct ArduinoGpioBackend
{
  static constexpr bool supports_analog = true;
  static constexpr bool supports_pwm    = true;

  static void begin_digital_in(uint8_t pin)        { pinMode(pin, INPUT); }
  static void begin_digital_in_pullup(uint8_t pin) { pinMode(pin, INPUT_PULLUP); }
  static void begin_analog_in(uint8_t /*pin*/)     { /* no-op */ }
  static void begin_digital_out(uint8_t pin)       { pinMode(pin, OUTPUT); }
  static void begin_pwm_out(uint8_t pin)           { pinMode(pin, OUTPUT); }

  static GpioLevel read_digital(uint8_t pin)
  {
    return digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }

  static ArchTypes::analog_type read_analog(uint8_t pin)
  {
    return static_cast<ArchTypes::analog_type>(analogRead(pin));
  }

  static void write_digital(uint8_t pin, GpioLevel v)
  {
    digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }

  static void write_pwm(uint8_t pin, ArchTypes::pwm_type v)
  {
    analogWrite(pin, v);
  }
};

#ifdef ADAFRUIT_MCP23X17_H
  #include <Adafruit_MCP23X17.h>
#endif

// -------------------- MCP23017 backend (optional) --------------------
// Only compiled if Adafruit_MCP23X17 is available (include order matters).
#if defined(ADAFRUIT_MCP23X17_H) || __has_include(<Adafruit_MCP23X17.h>)
  #if !defined(ADAFRUIT_MCP23X17_H)
    #include <Adafruit_MCP23X17.h>
  #endif

struct Mcp23017Backend
{
  static constexpr bool supports_analog = false;
  static constexpr bool supports_pwm    = false;

  static inline Adafruit_MCP23X17* dev = nullptr;

  static void attach(Adafruit_MCP23X17& d) { dev = &d; }
  static bool attached() { return dev != nullptr; }

  static void begin_digital_in(uint8_t pin)
  {
    dev->pinMode(pin, INPUT);
  }

  static void begin_digital_in_pullup(uint8_t pin)
  {
    dev->pinMode(pin, INPUT_PULLUP);
  }

  static void begin_analog_in(uint8_t /*pin*/)
  {
    // not supported
  }

  static void begin_digital_out(uint8_t pin)
  {
    dev->pinMode(pin, OUTPUT);
  }

  static void begin_pwm_out(uint8_t /*pin*/)
  {
    // not supported
  }

  static GpioLevel read_digital(uint8_t pin)
  {
    return dev->digitalRead(pin) ? GpioLevel::High : GpioLevel::Low;
  }

  static ArchTypes::analog_type read_analog(uint8_t /*pin*/)
  {
    return 0;
  }

  static void write_digital(uint8_t pin, GpioLevel v)
  {
    dev->digitalWrite(pin, (v == GpioLevel::High) ? HIGH : LOW);
  }

  static void write_pwm(uint8_t /*pin*/, ArchTypes::pwm_type /*v*/)
  {
    // not supported
  }
};
#endif // MCP23017 backend

// ============================================================================
// Mode traits (now backend-aware)
// ============================================================================
template <GpioMode, typename Backend>
struct GpioModeTraits;

// ---- Reserved ----
template <typename Backend>
struct GpioModeTraits<GpioMode::Reserved, Backend>
{
  using value_type = void;

  static constexpr bool beginable = false;
  static constexpr bool readable  = false;
  static constexpr bool writable  = false;
};

// ---- Digital input ----
template <typename Backend>
struct GpioModeTraits<GpioMode::DigitalIn, Backend>
{
  using value_type = ArchTypes::digital_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin) { Backend::begin_digital_in(pin); }
  static value_type read(uint8_t pin) { return Backend::read_digital(pin); }
};

// ---- Digital input w/ pullup ----
template <typename Backend>
struct GpioModeTraits<GpioMode::DigitalInPullup, Backend>
{
  using value_type = ArchTypes::digital_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin) { Backend::begin_digital_in_pullup(pin); }
  static value_type read(uint8_t pin) { return Backend::read_digital(pin); }
};

// ---- Analog input ----
template <typename Backend>
struct GpioModeTraits<GpioMode::AnalogIn, Backend>
{
  using value_type = ArchTypes::analog_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = true;
  static constexpr bool writable  = false;

  static void begin(uint8_t pin) { Backend::begin_analog_in(pin); }
  static value_type read(uint8_t pin) { return Backend::read_analog(pin); }
};

// ---- Digital output ----
template <typename Backend>
struct GpioModeTraits<GpioMode::DigitalOut, Backend>
{
  using value_type = ArchTypes::digital_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = false;
  static constexpr bool writable  = true;

  static void begin(uint8_t pin) { Backend::begin_digital_out(pin); }
  static void write(uint8_t pin, value_type v) { Backend::write_digital(pin, v); }
};

// ---- PWM output ----
template <typename Backend>
struct GpioModeTraits<GpioMode::PWMOut, Backend>
{
  using value_type = ArchTypes::pwm_type;

  static constexpr bool beginable = true;
  static constexpr bool readable  = false;
  static constexpr bool writable  = true;

  static void begin(uint8_t pin) { Backend::begin_pwm_out(pin); }
  static void write(uint8_t pin, value_type v) { Backend::write_pwm(pin, v); }
};

// ============================================================================
// PinIO (now backend-selectable; defaults to ArduinoGpioBackend)
// ============================================================================
template <uint8_t PIN, GpioMode MODE, typename Backend = ArduinoGpioBackend>
struct PinIO
{
  static constexpr uint8_t pin  = PIN;
  static constexpr GpioMode mode = MODE;

  using Traits     = GpioModeTraits<MODE, Backend>;
  using value_type = typename Traits::value_type;

private:
  // Compile-time guardrails for backends that don't support features.
  static constexpr bool wants_analog = (MODE == GpioMode::AnalogIn);
  static constexpr bool wants_pwm    = (MODE == GpioMode::PWMOut);

public:
  // begin() only when beginable
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::beginable, int> = 0>
  static void begin()
  {
    // If backend supports "attach" semantics, you can optionally assert here.
#if defined(ADAFRUIT_MCP23X17_H) || __has_include(<Adafruit_MCP23X17.h>)
    if constexpr (std::is_same_v<Backend, Mcp23017Backend>)
    {
      // Runtime check: ensure attached before use.
      // (No Serial dependency here; just a hard fail if misused.)
      if (!Backend::attached())
      {
        // Avoid UB; do nothing if not attached.
        return;
      }
    }
#endif

    static_assert(!(wants_analog && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    static_assert(!(wants_pwm && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");

    Traits::begin(PIN);
  }

  // read() only when readable
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::readable, int> = 0>
  static typename GpioModeTraits<M, Backend>::value_type read()
  {
    static constexpr bool wants_analog_m = (M == GpioMode::AnalogIn);
    static_assert(!(wants_analog_m && !Backend::supports_analog),
                  "Selected backend does not support AnalogIn");
    return GpioModeTraits<M, Backend>::read(PIN);
  }

  // write() only when writable
  template <
    GpioMode M = MODE,
    typename std::enable_if_t<GpioModeTraits<M, Backend>::writable, int> = 0>
  static void write(typename GpioModeTraits<M, Backend>::value_type v)
  {
    static constexpr bool wants_pwm_m = (M == GpioMode::PWMOut);
    static_assert(!(wants_pwm_m && !Backend::supports_pwm),
                  "Selected backend does not support PWMOut");
    GpioModeTraits<M, Backend>::write(PIN, v);
  }
};
