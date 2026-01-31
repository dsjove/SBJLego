#include <Arduino.h>

// The library's main headert'
#include "PinIO.h"
// A testing backend that “looks like pins” but doesn’t touch real hardware.
#include "UnitTestPinIO.h"
// MCP23017 is one of many GPIO expansion boards
#include "Mcp23017Device.h"

/*
  Declares a “pin type” for Arduino GPIO pin 7 configured as DigitalOut.
  The Pin is declared as a static type. Its usage has zero overhead.
  This improves readability declaration of intent, and maintainability.
*/
using LightPinDefault = PinIO<7, GpioMode::DigitalOut>;

/*
  The same pin is declared except with a Unit Test Backend. The same high-level
  code can run in unit tests or simulation, improving testability.
*/
using LightPinUnitTest = PinIO<7, GpioMode::DigitalOut, UnitTestPinIO<>>;

/*
  Just as we can attach the unit test backend we can also attach a 3rd party
  GPIO expansion board.
*/
using LightPinExpansionBoard = PinIO<9, GpioMode::DigitalOut, Mcp23017PinIO<>>;

/*
  PinIO is a a stateless type with no virtual functions. Those three types,
  after compilation resolve to the direct native (or driver) calls.

  Since we are using a type, if any of those 3 parameters change, all code that uses
  that type will get the change behaviors. The obvious benifit is change of pin number.
  This is similar to declaring constant named ints for the pins. But we also now carry
  the pin's board (backend); pin 7 on the MCU is different than pin 7 on the
  expansion board. We also now carry the mode. Compiler errors are nlow generated if you try to make an analog write to a digital port. What used to be runtime errors are now compile-time errors.
*/

/*
  If you design your classes to take the Pins as generic paramters, you can take
  advantage of duck typing. As long as your type looks like a PinIO, you can
  reroute the begin, read, and write calls without changiung the business logic.
  This is compiletime-polymorphism, not runtime.
*/
struct LightPinReroute
{
	static void begin(GpioLevel value) {
		Serial.println(static_cast<uint8_t>(value));
	}
};

/*
 Pass in any Pin
*/
template <typename PIN = LightPinDefault>
class Lighting {
public:
	void begin() {
		PIN::begin(GpioLevel::High);
	}
};
Lighting<> light1;
Lighting<LightPinUnitTest> light2;
Lighting<LightPinExpansionBoard> light3;
Lighting<LightPinReroute> light4;

/*
  This is to activivate this particular expansion board.
*/
Mcp23017Device expansion;

void setup()
{
	expansion.begin();
/*
	All lights have the identical business logic.
	But the actual hardware call has been abstracted out
	with zero overhead.
*/
	light1.begin();
	light2.begin();
	light3.begin();
	light4.begin();
}

void loop()
{
}
