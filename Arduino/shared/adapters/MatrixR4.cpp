#include "MatrixR4.h"

static MatrixR4* matrixRefR4 = NULL;

MatrixR4::MatrixR4(BLEServiceRunner& ble, bool flipY, bool flipX, bool invert)
: _current()
, _flipY(flipY)
, _flipX(flipX)
, _invert(invert)
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
  matrixRefR4->update(value, false);
}

void MatrixR4::update(const MatrixR4Value::Value& value, bool writeBLE)
{
  if (_current.update(value, _flipY, _flipX, _invert))
  {
    if (writeBLE)
    {
      _displayChar.ble.writeValue(_current.data(), _current.size());
	}
    loadFrame();
  }
}

void MatrixR4::loadFrame()
{
  _matrix.loadFrame(_current.data());
}

bool MatrixR4Value::update(const MatrixR4Value::Value& input, bool flippingY, bool flippingX, bool inverting)
{
  Value newValue;
  if (!flippingY && !flippingX)
  {
    newValue = inverting ? Value{ ~input[0], ~input[1], ~input[2] } : input;
  }
  else
  {
    for (int y = 0; y < Height; ++y)
    {
      for (int x = 0; x < Width; ++x)
      {
        const int srcY = flippingY ? flipY(y) : y;
        const int srcX = flippingX ? flipX(x) : x;
        const int srcIndex = getIndex(srcY, srcX);
        const int dstIndex = getIndex(y, x);
        bool pixel = getBit(input, srcIndex);
        if (inverting) pixel = !pixel;
        setBit(newValue, dstIndex, pixel);
      }
    }
  }
  if (_value == newValue) return false;
  _value = newValue;
  return true;
}
