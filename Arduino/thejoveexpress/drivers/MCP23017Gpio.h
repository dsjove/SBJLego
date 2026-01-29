#pragma once

#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

#include "../shared/core/PinIO.h"
#include "I2CHardware.h"

template<
  bool A0 = false,
  bool A1 = false,
  bool A2 = false,
  typename WireT = TwoWire,
  WireT& WireRef = Wire
>
struct MCP23017Gpio
{
  static constexpr uint8_t address = uint8_t(0x20 | (uint8_t(A2) << 2) | (uint8_t(A1) << 1) | uint8_t(A0));

  static inline Adafruit_MCP23X17 device{};

  static inline bool begin()
  {
    I2CHardware::begin();

    if (!device.begin_I2C(address, &WireRef))
    {
      Serial.println("[MCP23017] begin failed");
      return false;
    }

    Mcp23017Backend::attach(device);
    return true;
  }
};
