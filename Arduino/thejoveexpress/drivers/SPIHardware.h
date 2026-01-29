#pragma once

#include <Arduino.h>
#include <SPI.h>

//#include "../shared/core/PinIO.h"

namespace SPIHardware
{
  inline constexpr PinIO<D8,  GpioMode::Delegated> SpiSck{};
  inline constexpr PinIO<D9,  GpioMode::Delegated> SpiMiso{};
  inline constexpr PinIO<D10, GpioMode::Delegated> SpiMosi{};

  inline void begin()
  {
    static bool begun = false;
    if (!begun)
    {
      SPI.begin(
        SpiSck.pin,
        SpiMiso.pin,
        SpiMosi.pin);
      begun = true;
    }
  }
}
