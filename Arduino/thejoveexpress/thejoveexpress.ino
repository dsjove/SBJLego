#include <Arduino.h>
#include <TaskScheduler.h>

#include <NimBLEDevice.h>
#include <Adafruit_MCP23X17.h> //expansion board
#include "pins.h"

// Communications
#include "TheBLE.h"
#include "TheTime.h"
#include "TheWifi.h"
// MCU
#include "sdcard.h"
#include "expander.h"
#include "mic.h"
#include "camera.h"
// Motor
#include "motor.h"
#include "docking.h"
// Environment
#include "motion.h"
#include "lighting.h"
// Display
#include "display.h"
#include "audio.h"

const std::string serviceName = "The Jove Express";
Scheduler _taskScheduler;
TheBLE _ble(_taskScheduler, serviceName);
TheWifi _wifi(_taskScheduler, serviceName);

void setup()
{
  Serial.begin(9600);
  while (!Serial);
// Communications
   _ble.begin();
   _wifi.begin();
// MCU
  sdcard::begin();
  expander::begin();
  mic::begin(_taskScheduler);
  camera::begin(_taskScheduler);
// Motor
  motor::begin();
  docking::begin(_taskScheduler);
// Environment
  motion::begin(_taskScheduler);
  lighting::begin(_taskScheduler);
// Display
  display::begin();
  audio::begin();
}

void loop()
{
  _taskScheduler.execute();
}
