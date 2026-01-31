# PinIO

**PinIO** is a **zero-overhead, declarative GPIO abstraction** for Arduino and RaspberryPi with embedded C++ 17.

PinIO treats a pin not as a magic number, but as a **type** that encodes:
- the pin identifier
- the pin’s capability (input, output, etc.)
- the hardware backend that implements it

This approach improves **readability**, **maintainability**, and **testability** by pushing GPIO intent and correctness into the **type system**, while compiling down to direct hardware access with no runtime cost.

---

## Why PinIO?

Traditional Arduino GPIO code is imperative and loosely coupled:

```cpp
pinMode(7, OUTPUT);
digitalWrite(7, HIGH);
```

This style makes it easy to:
- reuse the wrong pin number
- write to an input pin
- read from an output pin
- forget where a pin is physically connected
- break behavior during refactors

PinIO replaces this with **declarative pin definitions** and **compile-time safety**.

---

## Core Concepts

### 1. A Pin Is a Type

A pin is declared once, as a type:

```cpp
using LightPinDefault = PinIO<7, GpioMode::DigitalOut>;
```

This single line declares:
- *which* pin
- *how* it is used
- *what operations are valid*

There is no state stored in the pin object itself. The compiler resolves everything at build time.

---

### 2. Zero-Overhead Abstraction

PinIO avoids:
- virtual functions
- dynamic allocation
- runtime dispatch

When optimized, PinIO calls compile down to the same instructions as direct
`digitalWrite()` or backend-specific register writes.

The abstraction exists only at **compile time**.

---

### 3. Bidirectional HAL

PinIO’s HAL works in both directions:

- **Backends** implement hardware access (MCU GPIO, I²C expanders, SPI devices)
- **Application code** consumes typed pin capabilities

This allows business logic to be written once and reused across hardware targets.

---

### 4. Backends Are Swappable by Type

The same pin declaration can be retargeted to different backends simply by changing the type:

```cpp
using LightPinDefault = PinIO<7, GpioMode::DigitalOut>;
using LightPinUnitTest = PinIO<7, GpioMode::DigitalOut, UnitTestPinIO<>>;
using LightPinExpansion = PinIO<9, GpioMode::DigitalOut, Mcp23017PinIO<>>;
```

No application logic changes.

---

### 5. Mode Is Compile-Time Safety

Because the pin mode is part of the type:
- calling `read()` on a `DigitalOut` pin is a compile-time error
- calling `write()` on a `DigitalIn` pin is a compile-time error
- invalid capability combinations fail to compile

---

### 6. “Things That Look Like Pins”

Any type that provides the required static interface can behave like a pin.

Example: reroute a pin to Serial output:

```cpp
struct LightPinReroute {
  static void begin(GpioLevel value) {
    Serial.println(static_cast<uint8_t>(value));
  }
};
```

---

## Hardware-Agnostic Business Logic Example

```cpp
template <typename PIN>
class Lighting {
public:
  void begin() {
    PIN::begin(GpioLevel::High);
  }
};
```

---

## Design Goals

- Declarative GPIO usage
- Zero runtime overhead
- Strongly-typed pin capabilities
- Backend-agnostic application logic
- First-class unit testing support

---

## Status

PinIO is under active development.
Breaking API changes may occur prior to a 1.0 release.

---

## License

PinIO is licensed under the Mozilla Public License 2.0 (MPL-2.0).

Modifications to PinIO source files must be made available under the same license.
Use in proprietary firmware is permitted.
