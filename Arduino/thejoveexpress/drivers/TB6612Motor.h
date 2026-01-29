#pragma once

#include "../shared/core/PinIO.h"

template <class Traits>
struct TB6612Motor
{
  // ======== Public API types ========

  enum class Channel : uint8_t { A, B };

  enum class StopMode : uint8_t
  {
    Coast,
    Brake
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

  // ======== Begin / enable ========

  inline static void begin()
  {
    // PWM
    Traits::MotorPwma.begin();
    Traits::MotorPwmb.begin();

    // Control pins
    Traits::MotorAin1.begin();
    Traits::MotorAin2.begin();
    Traits::MotorStby.begin();
    Traits::MotorBin1.begin();
    Traits::MotorBin2.begin();

    // Known safe state: disabled, stopped
    setStby(false);
    applyStop(Channel::A, StopMode::Coast);
    applyStop(Channel::B, StopMode::Coast);
  }

  inline static void enable(bool enabled, StopMode stop = StopMode::Coast)
  {
    if (!enabled)
    {
      applyStop(Channel::A, stop);
      applyStop(Channel::B, stop);
      setStby(false);
      return;
    }
    setStby(true);
  }

  // ======== Commands ========

  inline static void stop(Channel ch, StopMode mode = StopMode::Coast)
  {
    applyStop(ch, mode);
  }

  inline static void set(Channel ch, const Command& cmd)
  {
    if (cmd.speed == 0)
    {
      applyStop(ch, cmd.stop);
      return;
    }

    // Optional “auto-wake”: bring chip out of standby to execute the command.
    setStby(true);

    applyRun(ch, cmd.dir, cmd.speed);
  }

  // Signed speed: -255..255
  inline static void set(Channel ch, int speedSigned, StopMode stopWhenZero = StopMode::Coast)
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

  inline static void setA(int speedSigned, StopMode stopWhenZero = StopMode::Coast)
  {
    set(Channel::A, speedSigned, stopWhenZero);
  }

  inline static void setB(int speedSigned, StopMode stopWhenZero = StopMode::Coast)
  {
    set(Channel::B, speedSigned, stopWhenZero);
  }

  inline static void emergencyStop(bool goStandby = true)
  {
    applyStop(Channel::A, StopMode::Brake);
    applyStop(Channel::B, StopMode::Brake);
    if (goStandby) setStby(false);
  }

  // ======== Optional ramp helper (still stateless externally) ========
  // You hold the state (current/target) outside, we just provide a step function.

  struct RampState
  {
    int current = 0; // -255..255
    int target  = 0; // -255..255
  };

  inline static void rampTick(Channel ch, RampState& s, uint8_t step = 5, StopMode stopWhenZero = StopMode::Coast)
  {
    // Clamp target defensively
    if (s.target < -255) s.target = -255;
    if (s.target >  255) s.target =  255;

    if (s.current == s.target)
    {
      // Re-apply final state (safe if other code toggled standby/pins)
      set(ch, s.current, stopWhenZero);
      return;
    }

    if (s.target > s.current)
    {
      int next = s.current + step;
      s.current = (next > s.target) ? s.target : next;
    }
    else
    {
      int next = s.current - step;
      s.current = (next < s.target) ? s.target : next;
    }

    set(ch, s.current, stopWhenZero);
  }

private:
  // ======== Helpers ========

  inline static uint8_t clampU8(int v)
  {
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return static_cast<uint8_t>(v);
  }

  inline static void setStby(bool enabled)
  {
    Traits::MotorStby.write(enabled ? GpioLevel::High : GpioLevel::Low);
  }

  inline static void writePwmA(uint8_t pwm) { Traits::MotorPwma.write(pwm); }
  inline static void writePwmB(uint8_t pwm) { Traits::MotorPwmb.write(pwm); }

  // TB6612 truth table:
  // IN1 IN2: H L = fwd, L H = rev, L L = coast, H H = brake
  inline static void setInputsA(GpioLevel in1, GpioLevel in2)
  {
    Traits::MotorAin1.write(in1);
    Traits::MotorAin2.write(in2);
  }

  inline static void setInputsB(GpioLevel in1, GpioLevel in2)
  {
    Traits::MotorBin1.write(in1);
    Traits::MotorBin2.write(in2);
  }

  inline static void applyStop(Channel ch, StopMode mode)
  {
    // PWM must be 0 for stop modes
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

  inline static void applyRun(Channel ch, Direction dir, uint8_t speed)
  {
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
};
