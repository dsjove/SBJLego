#pragma once

#include <Arduino_LED_Matrix.h>
#include "../ble/IDBTCharacteristic.h"
#include <array>
#include <cstdint>

/*
Hardware:
Arduino UNO R4 WiFi
 */

#include <array>
#include <cstdint>

class MatrixR4Value
{
public:
  static constexpr int Width      = 12;
  static constexpr int Height     = 8;
  static constexpr int WordCount  = 3;
  static constexpr int TotalBits  = Width * Height;

  using Value = std::array<uint32_t, WordCount>;

  static constexpr Value allOff = { 0x00000000u, 0x00000000u, 0x00000000u };
  static constexpr Value allOn  = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };
  static constexpr Value circle = { 0x06009010u, 0x82042041u, 0x08090060u };

  MatrixR4Value(bool flipY = false, bool flipX = false, bool invert = false)
    : _value(allOff)
    , _flipY(flipY)
    , _flipX(flipX)
    , _invert(invert)
  {
  }

  MatrixR4Value(const Value& value, bool flipY = false, bool flipX = false, bool invert = false)
    : MatrixR4Value(flipY, flipX, invert)
  {
    update(value);
  }

  bool update(const Value& input);

  inline const uint32_t* data() const { return _value.data(); }
  inline size_t size() const { return sizeof(_value); }

private:
  Value _value;
  const bool _flipY;
  const bool _flipX;
  const bool _invert;

  inline static int getIndex(int y, int x)
  {
    return y * Width + x;
  }

  inline static int flipY(int y)
  {
    return Height - 1 - y;
  }

  inline static int flipX(int x)
  {
    return Width - 1 - x;
  }

  inline static bool getBit(const Value& frame, int index)
  {
    const uint32_t word = frame[index / 32];
    const uint32_t mask = 1u << (31 - (index % 32));  // MSB-first
    return (word & mask) != 0;
  }

  inline static void setBit(Value& frame, int index, bool value)
  {
    uint32_t& word = frame[index / 32];
    const uint32_t mask = 1u << (31 - (index % 32));  // MSB-first
    word = value ? (word | mask) : (word & ~mask);
  }
};

class MatrixR4
{
public:
  using Value = MatrixR4Value;

  MatrixR4(BLEServiceRunner& ble, const Value& value = MatrixR4Value());

  void begin();

  void update(const MatrixR4Value::Value& value);

private:
  Value _current;

  IDBTCharacteristic _displayChar;
  static void bleUpdate(BLEDevice device, BLECharacteristic characteristic);

  ArduinoLEDMatrix _matrix;

  void loadFrame();
};
