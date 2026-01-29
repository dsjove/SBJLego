#pragma once

#include <Arduino.h>

#include "pins.h"

// TB6612FNG business logic
namespace motor
{
  enum class Channel : uint8_t
  {
    A,
    B
  };
  enum class StopMode : uint8_t
  {
    Coast, // outputs off-ish (freewheel) for this driver configuration
    Brake  // short brake (both inputs high per TB6612 truth table)
  };
  enum class Direction : uint8_t
  {
    Forward,
    Reverse
  };

  struct Command
  {
    uint8_t   speed = 0; // 0..255
    Direction dir   = Direction::Forward;
    StopMode  stop  = StopMode::Coast;
  };

  // ======== Internal helpers ========

  inline uint8_t clampU8(int v)
  {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return static_cast<uint8_t>(v);
  }

  inline void writePwmA(uint8_t pwm) { pins::MotorPwma.write(pwm); }
  inline void writePwmB(uint8_t pwm) { pins::MotorPwmb.write(pwm); }

  inline void setStby(bool enabled)
  {
    pins::MotorStby.write(enabled ? GpioLevel::High : GpioLevel::Low);
  }

  // TB6612 truth table (typical):
  // IN1 IN2
  //  H   L   -> forward
  //  L   H   -> reverse
  //  L   L   -> stop (coast)
  //  H   H   -> short brake
  inline void setInputsA(GpioLevel in1, GpioLevel in2)
  {
    pins::MotorAin1.write(in1);
    pins::MotorAin2.write(in2);
  }

  inline void setInputsB(GpioLevel in1, GpioLevel in2)
  {
    pins::MotorBin1.write(in1);
    pins::MotorBin2.write(in2);
  }

  inline void applyStop(Channel ch, StopMode mode)
  {
    // For both stop modes, PWM should be 0
    if (ch == Channel::A) writePwmA(0);
    else                  writePwmB(0);

    if (mode == StopMode::Brake)
    {
      if (ch == Channel::A) setInputsA(GpioLevel::High, GpioLevel::High);
      else                  setInputsB(GpioLevel::High, GpioLevel::High);
    }
    else // Coast
    {
      if (ch == Channel::A) setInputsA(GpioLevel::Low, GpioLevel::Low);
      else                  setInputsB(GpioLevel::Low, GpioLevel::Low);
    }
  }

  inline void applyRun(Channel ch, Direction dir, uint8_t speed)
  {
    // Set direction pins first, then PWM
    if (ch == Channel::A)
    {
      if (dir == Direction::Forward) setInputsA(GpioLevel::High, GpioLevel::Low);
      else                           setInputsA(GpioLevel::Low,  GpioLevel::High);
      writePwmA(speed);
    }
    else
    {
      if (dir == Direction::Forward) setInputsB(GpioLevel::High, GpioLevel::Low);
      else                           setInputsB(GpioLevel::Low,  GpioLevel::High);
      writePwmB(speed);
    }
  }

  // ======== Core control functions ========

  inline void begin()
  {
    // MCU PWM pins
    pins::MotorPwma.begin();
    pins::MotorPwmb.begin();

    // Expander-backed control pins
    pins::MotorAin1.begin();
    pins::MotorAin2.begin();
    pins::MotorStby.begin();

    pins::MotorBin1.begin();
    pins::MotorBin2.begin();

    // Known safe state: disabled, stopped
    setStby(false);
    applyStop(Channel::A, StopMode::Coast);
    applyStop(Channel::B, StopMode::Coast);
  }

  // Puts the TB6612 into/out of standby. When disabled, motors are also stopped.
  inline void enable(bool enabled, StopMode stop = StopMode::Coast)
  {
    if (!enabled)
    {
      // Ensure no PWM, stop outputs, then standby low
      applyStop(Channel::A, stop);
      applyStop(Channel::B, stop);
      setStby(false);
      return;
    }

    setStby(true);
  }

  // Stop a single channel with a chosen stop behavior.
  inline void stop(Channel ch, StopMode mode = StopMode::Coast)
  {
    // If driver is in standby, this still leaves pins in a safe state.
    applyStop(ch, mode);
  }

  // Direct command (speed 0..255, direction).
  inline void set(Channel ch, const Command& cmd)
  {
    // If speed == 0 treat as stop.
    if (cmd.speed == 0)
    {
      applyStop(ch, cmd.stop);
      return;
    }

    // If we’re not in standby-high, bring it up automatically.
    // (If you prefer strict control, delete this line.)
    setStby(true);

    applyRun(ch, cmd.dir, cmd.speed);
  }

  // Signed speed convenience: -255..255
  //  >0 forward, <0 reverse, 0 stop
  inline void set(Channel ch, int speedSigned, StopMode stopWhenZero = StopMode::Coast)
  {
    if (speedSigned == 0)
    {
      applyStop(ch, stopWhenZero);
      return;
    }

    Command c;
    c.dir   = (speedSigned > 0) ? Direction::Forward : Direction::Reverse;
    c.speed = clampU8(speedSigned > 0 ? speedSigned : -speedSigned);
    c.stop  = stopWhenZero;

    set(ch, c);
  }

  // Convenience wrappers
  inline void setA(int speedSigned, StopMode stopWhenZero = StopMode::Coast)
  {
    set(Channel::A, speedSigned, stopWhenZero);
  }

  inline void setB(int speedSigned, StopMode stopWhenZero = StopMode::Coast)
  {
    set(Channel::B, speedSigned, stopWhenZero);
  }

  // Emergency stop: brakes both channels (and optionally puts the chip into standby).
  inline void emergencyStop(bool goStandby = true)
  {
    applyStop(Channel::A, StopMode::Brake);
    applyStop(Channel::B, StopMode::Brake);
    if (goStandby) setStby(false);
  }

  // ======== Optional: simple ramping helper ========
  // Call periodically (e.g., every 10-20ms) if you want smooth acceleration.
  struct Ramp
  {
    Channel  ch;
    int      current = 0;   // -255..255
    int      target  = 0;   // -255..255
    uint8_t  step    = 5;   // how much to change per tick (1..255)
    StopMode stop    = StopMode::Coast;

    void setTarget(int t) { target = (t < -255) ? -255 : (t > 255) ? 255 : t; }
    void tick()
    {
      if (current == target)
      {
        // keep applying final state in case something else toggled standby/pins
        motor::set(ch, current, stop);
        return;
      }

      if (target > current)
      {
        int next = current + step;
        current = (next > target) ? target : next;
      }
      else
      {
        int next = current - step;
        current = (next < target) ? target : next;
      }

      motor::set(ch, current, stop);
    }
  };
}

// ======== Example usage ========
//
// void setup() {
//   motor::begin();
//   motor::enable(true);
//   motor::setA(+180);                 // A forward, pwm 180
//   motor::setB(-200);                 // B reverse, pwm 200
// }
//
// void loop() {
//   // stop with brake:
//   motor::stop(motor::Channel::A, motor::StopMode::Brake);
//   motor::stop(motor::Channel::B, motor::StopMode::Brake);
//   // motor::enable(false); // optionally standby
// }
