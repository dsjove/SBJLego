#pragma once

#include <Arduino.h>
#include <TaskScheduler.h>

#include "pins.h"

namespace docking
{
  // Cached raw level + convenience bool
  inline volatile GpioLevel dockLevel = GpioLevel::Low;
  inline volatile bool isDocked  = false;

  inline void _tick()
  {
    const auto v = pins::DockDetect.read();
    dockLevel = v;
    isDocked  = (v == GpioLevel::High);
  }

  inline Task task(500, TASK_FOREVER, &_tick);

  inline void begin(Scheduler& sched)
  {
    pins::DockDetect.begin();
    sched.addTask(task);
    task.enable();
  }
}
