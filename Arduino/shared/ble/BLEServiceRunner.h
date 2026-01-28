#pragma once

#define _TASK_LTS_POINTER
#include <TaskScheduler.h>
#include "BLEUUID.h"
#include <string>

class BLEServiceRunner
{
public:
  BLEServiceRunner(Scheduler& scheduler, const std::string& serviceName, int pollMS = 100, const std::string& overrideId = "");

  void addCharacteristic(BLECharacteristic& ble);

  const BLEUUID& serviceId() const { return _serviceId; }

  void begin();

private:
  const std::string _name;
  const BLEUUID _serviceId;
  BLEService _bleService;
  Task _bluetoothTask;

  static void loop();
  static void bluetooth_connected(BLEDevice device);
  static void bluetooth_disconnected(BLEDevice device);
};
