#pragma once
#include "shared/core/PinIO.h"
#include "drivers/Mcp23017PinIO.h"

namespace pins
{
  // ---- Display (ST7789 SPI control) ----
  inline constexpr PinIO<D0, GpioMode::DigitalOut> TftCs{};
  inline constexpr PinIO<D1, GpioMode::DigitalOut> TftDc{};

  // ---- Audio (MAX98357A I2S) ----
  inline constexpr PinIO<D2, GpioMode::DigitalOut> I2sDin{};
  inline constexpr PinIO<D6, GpioMode::DigitalOut> I2sBclk{};
  inline constexpr PinIO<D7, GpioMode::DigitalOut> I2sLrc{};

  // ---- PF Lights (headlights, MOSFET) ----
  // ---- Passenger car LED strip (MOSFET) ----
  inline constexpr PinIO<A0, GpioMode::PWMOut> HeadLightPwm{};
  inline constexpr PinIO<A2, GpioMode::PWMOut> CarLightPwm{};
  inline constexpr PinIO<A3, GpioMode::DigitalOut> CarLightEnable{};

  // ---- Dock detect ----
  inline constexpr PinIO<15, GpioMode::DigitalIn, Mcp23017PinIO> DockDetect{}; // GPB7

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
}
