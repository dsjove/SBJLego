#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "../shared/core/PinIO.h"

namespace I2CHardware
{
  inline constexpr PinIO<D4, GpioMode::Delegated> I2cSda{};
  inline constexpr PinIO<D5, GpioMode::Delegated> I2cScl{};

  inline void begin()
  {
    static bool begun = false;
    if (!begun)
    {
      Wire.begin(I2cSda.pin, I2cScl.pin);
      Wire.setClock(400000);
      begun = true;
    }
  }
}
