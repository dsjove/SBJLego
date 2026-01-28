#pragma once

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

#include "pins.h"

namespace expander {
  struct ExpanderConfig
  {
    // Address strap pins (true = VCC, false = GND)
    static constexpr bool A0 = false;
    static constexpr bool A1 = false;
    static constexpr bool A2 = false;

    // MCP23017 base address = 0x20
    static constexpr uint8_t address = 0x20 | (A2 << 2) | (A1 << 1) | A0;
  };

  enum class Strap : uint8_t { GND = 0, VCC = 1 };
  constexpr uint8_t mcp23017_addr(Strap a0, Strap a1, Strap a2)
  {
    return 0x20
       | (static_cast<uint8_t>(a2) << 2)
       | (static_cast<uint8_t>(a1) << 1)
       |  static_cast<uint8_t>(a0);
  }
  constexpr uint8_t MCP_ADDR = mcp23017_addr(Strap::GND, Strap::GND, Strap::GND);

  inline Adafruit_MCP23X17 device;

  inline void begin()
  {
    pins::ensure_i2c_begun();
    if (!device.begin_I2C(ExpanderConfig::address, &Wire))
    {
      Serial.println("[MCP23017] begin failed");
      return;
    }
    Mcp23017Backend::attach(device);
  }
}
