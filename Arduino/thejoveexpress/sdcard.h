#pragma once

#include <Arduino.h>
#include <TaskScheduler.h>

#include <SPI.h>
#include <SD.h>

#include "pins.h"

// microSD (XIAO ESP32S3 Sense expansion board)
// SPI-based SD slot, CS on GPIO21
// Setup-only: initializes SPI + SD card

namespace sdcard
{
  inline void begin()
  {
    // Chip Select
    pins::SdCs.begin();
    pins::SdCs.write(GpioLevel::High);

    // Shared SPI bus (TFT / SD are mutually exclusive at hardware level)
    pins::ensure_spi_begun();

    // Initialize SD over SPI
    if (!SD.begin(pins::SdCs.pin, SPI))
    {
      Serial.println("[sdcard] SD.begin failed");
    }
  }
}
