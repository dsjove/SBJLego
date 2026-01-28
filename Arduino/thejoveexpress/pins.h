#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "shared/core/PinIO.h"

namespace pins
{
  // ---- SPI bus (TFT + onboard microSD) ----
  inline constexpr PinIO<D8,  GpioMode::Reserved> SpiSck{};
  inline constexpr PinIO<D9,  GpioMode::Reserved> SpiMiso{};
  inline constexpr PinIO<D10, GpioMode::Reserved> SpiMosi{};

  // ---- I2C bus (sensors + MCP23017) ----
  inline constexpr PinIO<D4, GpioMode::Reserved> I2cSda{};
  inline constexpr PinIO<D5, GpioMode::Reserved> I2cScl{};

  // ---- Display (ST7789 SPI control) ----
  inline constexpr PinIO<D0, GpioMode::DigitalOut> TftCs{};
  inline constexpr PinIO<D1, GpioMode::DigitalOut> TftDc{};

  // ---- Audio (MAX98357A I2S) ----
  inline constexpr PinIO<D2, GpioMode::DigitalOut> I2sDin{};
  inline constexpr PinIO<D6, GpioMode::DigitalOut> I2sBclk{};
  inline constexpr PinIO<D7, GpioMode::DigitalOut> I2sLrc{};

  // ---- Motor driver (TB6612) ----
  // Channel A (train motor)
  inline constexpr PinIO<D3, GpioMode::PWMOut> MotorPwma{};

  // Channel B (optional second motor)
  inline constexpr PinIO<A1, GpioMode::PWMOut> MotorPwmb{};

  // ---- TB6612 direction + standby ----
  inline constexpr PinIO<0, GpioMode::DigitalOut, Mcp23017Backend> MotorAin1{}; // GPA0
  inline constexpr PinIO<1, GpioMode::DigitalOut, Mcp23017Backend> MotorAin2{}; // GPA1
  inline constexpr PinIO<4, GpioMode::DigitalOut, Mcp23017Backend> MotorStby{}; // GPA4

  inline constexpr PinIO<8, GpioMode::DigitalOut, Mcp23017Backend> MotorBin1{}; // GPB0
  inline constexpr PinIO<9, GpioMode::DigitalOut, Mcp23017Backend> MotorBin2{}; // GPB1

  // ---- PF Lights (headlights, MOSFET) ----
  inline constexpr PinIO<A0, GpioMode::PWMOut> HeadLightPwm{};

  // ---- Passenger car LED strip (MOSFET) ----
  inline constexpr PinIO<A2, GpioMode::PWMOut> CarLightPwm{};
  inline constexpr PinIO<A3, GpioMode::DigitalOut> CarLightEnable{};

  // ---- Dock detect ----
  inline constexpr PinIO<15, GpioMode::DigitalIn, Mcp23017Backend> DockDetect{}; // GPB7

  // ---- microSD (XIAO ESP32S3 Sense onboard SD) ----
  inline constexpr PinIO<21, GpioMode::DigitalOut> SdCs{};

  // ---- Sense built-in PDM Microphone ----
  // I2S clock (BCLK / PDM CLK)
inline constexpr PinIO<42, GpioMode::Reserved> MicBclk{};

// I2S word select (LRCLK / PDM WS)
inline constexpr PinIO<41, GpioMode::Reserved> MicLrc{};

// I2S data in (PDM DATA)
inline constexpr PinIO<2,  GpioMode::Reserved> MicDin{};

  // ================= Camera data pins =================
  // OV2640 Y2–Y9
  inline constexpr PinIO<5,  GpioMode::Reserved> CamY2{};
  inline constexpr PinIO<18, GpioMode::Reserved> CamY3{};
  inline constexpr PinIO<19, GpioMode::Reserved> CamY4{};
  inline constexpr PinIO<21, GpioMode::Reserved> CamY5{};
  inline constexpr PinIO<36, GpioMode::Reserved> CamY6{};
  inline constexpr PinIO<39, GpioMode::Reserved> CamY7{};
  inline constexpr PinIO<34, GpioMode::Reserved> CamY8{};
  inline constexpr PinIO<35, GpioMode::Reserved> CamY9{};

  // ================= Camera sync / clock =================
  inline constexpr PinIO<10, GpioMode::Reserved> CamXclk{};
  inline constexpr PinIO<13, GpioMode::Reserved> CamPclk{};
  inline constexpr PinIO<38, GpioMode::Reserved> CamVsync{};
  inline constexpr PinIO<9,  GpioMode::Reserved> CamHref{};

  // ================= Camera SCCB (I2C-like) =================
  inline constexpr PinIO<8,  GpioMode::Reserved> CamSda{};
  inline constexpr PinIO<7,  GpioMode::Reserved> CamScl{};

  inline void ensure_i2c_begun()
  {
    static bool begun = false;
    if (!begun)
    {
      Wire.begin(pins::I2cSda.pin, pins::I2cScl.pin);
      Wire.setClock(400000);
      begun = true;
    }
  }

  inline void ensure_spi_begun()
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
