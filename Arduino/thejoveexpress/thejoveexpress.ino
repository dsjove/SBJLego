#include "shared/core/TaskThunk.cpp"
#include "shared/ble/BLEServiceRunner.cpp"
#include "shared/adapters/ServoMotor.cpp"
#include "shared/adapters/Lighting.cpp"

Scheduler _runner;
BLEServiceRunner _ble(_runner, "Jove Express");
Lighting _lighting(_runner, _ble, {{3, true}, {0, false}}, A0);
ServoMotor _servoMotor(_ble, 9);

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  _ble.begin();
  _lighting.begin();
  _servoMotor.begin();
}

void loop()
{
  _runner.execute();
}
