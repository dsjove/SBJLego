#pragma once

#include <Arduino.h>
#include <SD.h>

#include "../drivers/SPIHardware.h"

namespace TheSDCard
{
  inline constexpr PinIO<21, GpioMode::DigitalOut> SdCs{};

  inline void begin()
  {
    SdCs.begin(GpioLevel::High);
    SPIHardware::begin();
    if (!SD.begin(SdCs.pin, SPI))
    {
      Serial.println("[sdcard] SD.begin failed");
    }
  }
}
