#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "drivers/SPIHardware.h"

namespace display
{
  inline Adafruit_ST7789 device(
    pins::TftCs.pin,
    pins::TftDc.pin,
    -1 // RST not connected
  );

  inline void begin()
  {
    pins::TftCs.begin();
    pins::TftDc.begin();

    SPIHardware::begin();

    // adjust to your exact panel and position
    device.init(240, 240);
    device.setRotation(0);
    // Known safe visual state
    device.fillScreen(ST77XX_BLACK);
  }
}
