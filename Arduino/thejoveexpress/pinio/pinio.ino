#include <Arduino.h>

#include "PinIO.h"
#include "UnitTestPinIO.h"
//Example GPIO expansion board
#include "Mcp23017Device.h"

using LightPinDefault = PinIO<7, GpioMode::DigitalOut>;
using LightPinUnitTest = PinIO<7, GpioMode::DigitalOut, UnitTestPinIO<>>;
using LightPinExpansionBoard = PinIO<9, GpioMode::DigitalOut, Mcp23017PinIO<>>;

struct LightPinReroute
{
	static void begin(GpioLevel value) {
		//send the call to something that is not a GPIO pin
		Serial.println(static_cast<uint8_t>(value));
	}
};


template <typename PIN = LightPinDefault>
class Lighting {
public:
	void begin() {
		PIN::begin(GpioLevel::High);
	}

	void loop() {
	// Compiler error
	// auto v =  light1.read();
	}
};

Lighting<> light1;
Lighting<LightPinUnitTest> light2;
Lighting<LightPinExpansionBoard> light3;
Lighting<LightPinReroute> light4;

Mcp23017Device expansion;

void setup()
{
	expansion.begin();
	light1.begin();
	light2.begin();
	light3.begin();
	light4.begin();
}

void loop()
{
}
