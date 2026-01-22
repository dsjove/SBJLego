#include "MatrixR4.h"

static MatrixR4* matrixRefR4 = NULL;

MatrixR4::MatrixR4(BLEServiceRunner& ble, const Value& value)
: _current(value)
, _displayChar(ble, "07020000", &_current, updateDisplay)
{
  matrixRefR4 = this;
}

void MatrixR4::begin()
{
  _matrix.begin();
  _matrix.loadFrame(_current.data());
}

void MatrixR4::updateDisplay(BLEDevice, BLECharacteristic characteristic)
{
  //Serial.println(characteristic.uuid());
  Value value;
  characteristic.readValue(value.data(), sizeof(value));
  matrixRefR4->set(value);
}

void MatrixR4::set(const Value& data)
{
  if (data != _current)
  {
    _current = data;
    _matrix.loadFrame(_current.data());
  }
}
