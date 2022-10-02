#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_LCD16x2.ino
// Author      : Created by Debinix Team (C). Licensed under GPL-3.0.
// Version     : Date 2022-08-29.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/DebinixTeam/SensorWLED)
// Tested: Arduino Uno,
//============================================================================

#include<Wire.h>
#include<LiquidCrystal_I2C.h>		// for AVR based microcontrollers
#include <SensorWLED.h>

//#define DEBUG		// Uncomment to enable serial monitor output
#ifdef DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#define BAUDRATE 9600

			// FixMe: What parameters, hold_time .. or maybe in begin()?
            // analog_pin, ADC bits, vcc, ms_poll_tm, (ms_refresh_tm)
SensorWLED WLED1 = SensorWLED(PIN_A0, Bits10, Vcc5v0, 500, 1000);

LiquidCrystal_I2C lcd(0x27,16,2); // // instantiate an LCD object

double mv_value ;
double pk_mv_value ;

void setup() {

	Serial.begin(BAUDRATE);

	lcd.init();
	lcd.backlight();

	// set analog read pin and ...
	WLED1.begin(WledLCD);
	Serial.println("Setup completed!");

	delay(1000);  // the only delay() call.
}

// Do not add any long delay statements in loop().
// It will affect the feel for momentanous updates.
void loop() {

	// Current sensor
	WLED1.updateAnalogRead();

	// The 'LiquidCrystal_I2C' library *needs* 'doubles' for correct output
	mv_value = (double) WLED1.getMappedValue();
	pk_mv_value = (double) WLED1.getMappedPeakValue();

	debug("Analog input: ");
	debug(mv_value);
	debugln(" mV");
	debug("Peak input  : ");
	debug(pk_mv_value);
	debugln(" mV");
	#ifdef DEBUG
	delay(100);  // not hogging serial monitor
	#endif

	update2x16Display(mv_value, pk_mv_value);

}

void update2x16Display(void) {

	lcd.setCursor(0,0);   // col, row
	lcd.print("Peak : ");
	lcd.setCursor(8,0);   // col, row
	lcd.print(pk_mv_value);

	lcd.setCursor(0,1);   // col, row
	lcd.print("Input: ");
	lcd.setCursor(8,1);   // col, row
	lcd.print(mv_value);
}

#endif

