#include <Arduino.h>
#include <TaskScheduler.h>

#include <Adafruit_MCP23X17.h> //expansion board
#include "pins.h"

#include "expander.h"
#include "motor.h"
#include "display.h"
#include "audio.h"
#include "docking.h"
#include "sdcard.h"
#include "mic.h"
#include "camera.h"
#include "motion.h"
#include "lighting.h"
#include "ble.h"

Scheduler _taskScheduler;

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  expander::begin();
  motor::begin();
  display::begin();
  audio::begin();
  docking::begin(_taskScheduler);
  sdcard::begin();
  mic::begin(_taskScheduler);
  camera::begin(_taskScheduler);
  motion::begin(_taskScheduler);
  lighting::begin(_taskScheduler);
}

void loop()
{
  _taskScheduler.execute();
}
