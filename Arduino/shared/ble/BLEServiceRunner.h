#pragma once

#include "../core/TaskThunk.h"
#include "BLEUUID.h"
#include <string>

class BLEServiceRunner: ScheduledRunner
{
public:
  BLEServiceRunner(Scheduler& scheduler, const std::string& serviceName, const std::string& overrideId = "");

  void addCharacteristic(BLECharacteristic& ble);

  const BLEUUID& serviceId() const { return _serviceId; }

  void begin();

private:
  const std::string _name;
  const BLEUUID _serviceId;
  BLEService _bleService;
  TaskThunk _bluetoothTask;

  virtual void loop(Task&);
  static void bluetooth_connected(BLEDevice device);
  static void bluetooth_disconnected(BLEDevice device);
};
