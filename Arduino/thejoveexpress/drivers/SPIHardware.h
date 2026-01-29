#pragma once

#include <Arduino.h>
#include <SPI.h>

//#include "../shared/core/PinIO.h"

namespace SPIHardware
{
  inline constexpr PinIO<D8,  GpioMode::Reserved> SpiSck{};
  inline constexpr PinIO<D9,  GpioMode::Reserved> SpiMiso{};
  inline constexpr PinIO<D10, GpioMode::Reserved> SpiMosi{};

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
