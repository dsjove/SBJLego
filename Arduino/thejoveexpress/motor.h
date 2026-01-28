#pragma once

#include <Arduino.h>

#include "pins.h"

namespace motor
{
  inline void begin()
  {
    // MCU PWM pins
    pins::MotorPwma.begin();
    pins::MotorPwmb.begin();   // safe even if channel B unused

    // Expander-backed control pins
    pins::MotorAin1.begin();
    pins::MotorAin2.begin();
    pins::MotorStby.begin();

    pins::MotorBin1.begin();
    pins::MotorBin2.begin();

    // Known safe state: disabled, stopped
    pins::MotorStby.write(GpioLevel::Low);
    pins::MotorPwma.write(0);
    pins::MotorPwmb.write(0);
  }
}
