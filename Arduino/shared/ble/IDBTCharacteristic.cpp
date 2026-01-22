#include "IDBTCharacteristic.h"
#include "BLEServiceRunner.h"
#include "BLEUUID.cpp"

unsigned char adjustPermissions(
    unsigned char base,
    const void* value,
    BLECharacteristicEventHandler eventHandler)
{
  unsigned char p = base;
  if (value)
  {
    p |= BLERead;
    p |= BLENotify;
  }
  if (eventHandler)
  {
    p |= BLEWriteWithoutResponse;
  }
  return p;
}

IDBTCharacteristic::IDBTCharacteristic(
    BLEServiceRunner& runner,
    const std::string& propertyId,
    size_t valueSize,
    const void* value,
    BLECharacteristicEventHandler eventHandler)
: uuid(btutil::makeUuidWithProperty(propertyId, runner.serviceId()))
, ble(uuid.data(), adjustPermissions(0, value, eventHandler), valueSize)
{
  if (eventHandler)
  {
    ble.setEventHandler(BLEWritten, eventHandler);
  }
  if (value)
  {
    ble.writeValue(static_cast<const unsigned char*>(value), valueSize);
  }
  runner.addCharacteristic(ble);
}
