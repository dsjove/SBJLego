#include "MatrixR4.h"

static MatrixR4* matrixRefR4 = NULL;

MatrixR4::MatrixR4(BLEServiceRunner& ble, const Value& value)
: _current(value)
, _displayChar(ble, "07020000", _current.size(), _current.data(), bleUpdate)
{
  matrixRefR4 = this;
}

void MatrixR4::begin()
{
  _matrix.begin();
  _matrix.loadFrame(_current.data());
}

void MatrixR4::bleUpdate(BLEDevice, BLECharacteristic characteristic)
{
  //Serial.println(characteristic.uuid());
  MatrixR4Value::Value value;
  characteristic.readValue(value.data(), sizeof(value));
  if (matrixRefR4->_current.update(value))
  {
    matrixRefR4->loadFrame();
  }
}

void MatrixR4::update(const MatrixR4Value::Value& value)
{
  if (_current.update(value))
  {
    _displayChar.ble.writeValue(_current.data(), _current.size());
    loadFrame();
  }
}

void MatrixR4::loadFrame()
{
  _matrix.loadFrame(_current.data());
}

bool MatrixR4Value::update(const MatrixR4Value::Value& input)
{
  Value newValue;
  if (!_flipX && !_flipY)
  {
    newValue = _invert ? Value{ ~input[0], ~input[1], ~input[2] } : input;
  }
  else
  {
    for (int y = 0; y < Height; ++y)
    {
      for (int x = 0; x < Width; ++x)
      {
        const int srcY = _flipY ? flipY(y) : y;
        const int srcX = _flipX ? flipX(x) : x;
        const int srcIndex = getIndex(srcY, srcX);
        const int dstIndex = getIndex(y, x);
        bool pixel = getBit(input, srcIndex);
        if (_invert) pixel = !pixel;
        setBit(newValue, dstIndex, pixel);
      }
    }
  }
  if (_value == newValue) return false;
  _value = newValue;
  return true;
}
