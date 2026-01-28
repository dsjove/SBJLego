#pragma once

#include <Arduino.h>
#include <TaskScheduler.h>
#include <Wire.h>

#include <Adafruit_VEML7700.h>

#include "pins.h"

// Lighting subsystem
// - Headlights (PWM MOSFET): pins::HeadLightPwm
// - Car lights (PWM MOSFET + enable): pins::CarLightPwm, pins::CarLightEnable
// - Ambient light sensor (Qwiic/I2C): VEML7700 (per BOM)
// Setup-only: initializes pins + sensor, and provides a 500ms task to sample lux.
// No automatic brightness / no business logic.

namespace lighting
{
  // ---- Ambient sensor device ----
  inline Adafruit_VEML7700 device;

  // Cached ambient values (raw reading for later logic)
  inline volatile float lux = 0.0f;

  inline void _tick()
  {
    // Safe even if you don't use it yet; lux may remain 0 if begin failed.
    lux = device.readLux();
  }

  // Sample ambient light twice a second (setup-only)
  inline Task task(500, TASK_FOREVER, &_tick);

  inline void begin(Scheduler& sched)
  {
    // --- Output pins ---
    pins::HeadLightPwm.begin();
    pins::CarLightPwm.begin();
    pins::CarLightEnable.begin();

    // Known safe state: lights off
    pins::HeadLightPwm.write(0);
    pins::CarLightPwm.write(0);
    pins::CarLightEnable.write(GpioLevel::Low);

    // --- Ambient sensor ---
    pins::ensure_i2c_begun();

    if (!device.begin())
    {
      Serial.println("[lighting] VEML7700 begin failed");
      return; // task not enabled if sensor isn't present
    }

    // Setup-ish defaults (not business logic)
    device.setGain(VEML7700_GAIN_1);
    device.setIntegrationTime(VEML7700_IT_100MS);

    // Periodic sampling
    sched.addTask(task);
    task.enable();
  }
}
