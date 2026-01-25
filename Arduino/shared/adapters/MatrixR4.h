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

  MatrixR4Value(bool flipX = false, bool flipY = false, bool invert = false)
    : _value(allOff)
    , _flipX(flipX)
    , _flipY(flipY)
    , _invert(invert)
  {
  }

  MatrixR4Value(const Value& value, bool flipX = false, bool flipY = false, bool invert = false)
    : MatrixR4Value(flipX, flipY, invert)
  {
    update(value);
  }

  bool update(const Value& input);

  inline const uint32_t* data() const { return _value.data(); }
  inline size_t size() const { return _value.size(); }

private:
  Value _value;
  const bool _flipX;
  const bool _flipY;
  const bool _invert;

  inline static bool getBit(const Value& frame, int index)
  {
    return (frame[index / 32] >> (index % 32)) & 0x01u;
  }

  inline static void setBit(Value& frame, int index, bool value)
  {
    uint32_t& word = frame[index / 32];
    const uint32_t mask = 1u << (index % 32);
    word = value ? (word | mask) : (word & ~mask);
  }
};

class MatrixR4
{
public:
  using Value = MatrixR4Value;

  MatrixR4(BLEServiceRunner& ble, const Value& value = MatrixR4Value());

  void begin();

  void update(const MatrixR4Value::Value& data);

private:
  Value _current;

  IDBTCharacteristic _displayChar;
  static void updateDisplay(BLEDevice device, BLECharacteristic characteristic);

  ArduinoLEDMatrix _matrix;

  void set(const MatrixR4Value::Value& data);
};
