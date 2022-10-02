#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED.ino
// Author      : Created by Debinix Team (C). Licensed under GPL-3.0.
// Version     : Date 2022-08-29.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/DebinixTeam/SensorWLED)
// Tested: Arduino Uno,
//============================================================================

#include <SensorWLED.h>

#define BAUDRATE 9600

// Optional, uncomment '#define LEDTASK' to demo non-preemptive multitasking.
//#define LEDTASK
#ifdef LEDTASK
#include <LedTask.h>

// Instantiate a few led objects, to simulate other tasks.
LedTask LedOne = LedTask(12);
LedTask LedTwo = LedTask(13);
LedTask LedThree = LedTask(8);
LedTask LedFour = LedTask(7);
#endif
			// FixMe: What parameters, hold_time .. or maybe in begin()?
            // analog_pin, ADC bits, vcc, ms_poll_tm, (ms_refresh_tm)
SensorWLED WLED1 = SensorWLED(PIN_A0, BITS10, VCC5V0, 500, 1000);

uint16_t mv_value, pk_mv_value ;

void setup() {
	Serial.begin(BAUDRATE);
	delay(1000);

#ifdef LEDTASK
	//          led_pin#,on_ms,off_ms
		LedOne.begin(100, 400);
		LedTwo.begin(350, 350);
		LedThree.begin(125, 250);
		LedFour.begin(500, 400);
#endif
	// set analog read pin
	WLED1.begin(WLED_LCD);	// FixMe: What parameters, if any should it be?
	Serial.println("Setup completed...");
}

// Do not add long delay statements in loop().
// Arduino runs in non-preemptive multitasking.
void loop() {

#ifdef LEDTASK
	// Demo of four independent (blinking LEDs) tasks
	LedOne.updateBlinkLed();
	LedTwo.updateBlinkLed();
	LedThree.updateBlinkLed();
	LedFour.updateBlinkLed();
#endif

	// Current sensor
	mv_value = WLED1.getMappedValue();
	pk_mv_value = WLED1.getMappedPeakValue();

	Serial.print("Analog input: ");
	Serial.print(mv_value);
	Serial.println(" mV");
	Serial.print("Peak input  : ");
	Serial.print(pk_mv_value);
	Serial.println(" mV");

	delay(100);  // not hogging serial monitor

}

#endif

