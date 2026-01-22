#include "shared/core/TaskThunk.cpp"

#include "shared/ble/IDBTCharacteristic.cpp"
#include "shared/ble/BLEServiceRunner.cpp"

#include "shared/core/LegoPFIR.cpp"
#include "shared/core/MFRC522Detector.cpp"

#include "shared/adapters/MatrixR4.cpp"
#include "shared/adapters/Lighting.cpp"
#include "shared/adapters/RFIDBroadcaster.cpp"
#include "shared/adapters/LEGOPFTransmitter.cpp"

#include <SPI.h>

Scheduler _runner;
BLEServiceRunner _ble(_runner, "City Center");
MatrixR4 _matrixR4(_ble); // {0xB194a444, 0x44042081, 0x100a0841}
Lighting _lighting(_runner, _ble, {{3, true}, {0, false}}, A0);
RFIDBroadcaster _RFIDBroadcaster(_runner, _ble, 1);
LEGOPFTransmitter _pfTransmitter(_runner, _ble, 7);

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  SPI.begin();

  _ble.begin();
  _matrixR4.begin();
  _lighting.begin();
  _RFIDBroadcaster.begin();
  _pfTransmitter.begin();
}

void loop()
{
  _runner.execute();
}
