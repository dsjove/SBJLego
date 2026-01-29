#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "shared/core/PinIO.h"
#include "drivers/SPIHardware.h"

namespace display
{
  // ---- Display (ST7789 SPI control) ----
  inline constexpr PinIO<D0, GpioMode::DigitalOut> TftCs{};
  inline constexpr PinIO<D1, GpioMode::DigitalOut> TftDc{};
  
  inline Adafruit_ST7789 device(
    TftCs.pin,
    TftDc.pin,
    -1 // RST not connected
  );

  inline void begin()
  {
    TftCs.begin();
    TftDc.begin();

    SPIHardware::begin();

    // adjust to your exact panel and position
    device.init(240, 240);
    device.setRotation(0);
    // Known safe visual state
    device.fillScreen(ST77XX_BLACK);
  }
}
