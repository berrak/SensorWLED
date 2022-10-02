#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_OLED128x64SH1106EXP.ino
// Author      : Created by Debinix Team (C). The MIT License (MIT).
// Version     : Date 2022-08-29.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/DebinixTeam/SensorWLED)
// Tested: Arduino Uno,
//============================================================================
// URL: https://github.com/adafruit/Adafruit_SH110x
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> // 1.3" OLED 128x64 SH1106G
// If you don't want the 'Adafruit-flash logo' at start-up. In the
// header file, uncomment row 35, i.e. //#define SH110X_NO_SPLASH

#include <SensorWLED.h>

//#define DEBUG		    // Uncomment to enable serial monitor output
#ifdef DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#define BAUDRATE 9600
#define DECAY_RATE 0.5	    // sets the peak value decay constant, lamda

				  // analog-pin,ADC-bits,vcc, ms_adc-poll-tm, rate-value
SensorWLED WLED1 = SensorWLED(PIN_A0, Bits10, Vcc5v0, 500, DECAY_RATE);

								// width, height ,I2C, no-reset
Adafruit_SH1106G oled = Adafruit_SH1106G(128, 64, &Wire, -1);

double mv_value ;
double mv_pk_value ;

void setup() {
	Serial.begin(BAUDRATE);

	delay(250); // wait for the OLED to power up
	oled.begin(0x3C, true);   // common eBay OLED's I2C address: 0x3C

	// set decay model to use, and hold time (ms), before decaying
	WLED1.begin(ExponentialDecay, 500);
	Serial.println("Setup completed!");

	delay(1000);  // the only delay() call.
}

// Do not add any long delay statements in loop().
// It will ruin any feel for momentanous updates.
void loop() {

	// Current sensor
	if (WLED1.updateAnalogRead() == true) {
		mv_value = (double) WLED1.getMappedValue();
		mv_pk_value = (double) WLED1.getMappedPeakValue();

		update128x64Display();
	}

	debug("Analog input: ");
	debug(mv_value);
	debugln(" mV");
	debug("Peak input  : ");
	debug(mv_pk_value);
	debugln(" mV");
	#ifdef DEBUG
	delay(100);  // not hogging serial monitor
	#endif



}

void update128x64Display(void) {

	double pk_value = mv_pk_value/1000.0;  // scale up 1000x for display
	double value = mv_value/1000.0;        // scale up 1000x for display

	// Clear the buffer.
	oled.clearDisplay();

	// text display tests
	oled.setTextSize(1);
	oled.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text

	oled.setCursor(0, 0);
	oled.println(" Peak ");

	oled.setTextSize(3);
	oled.setTextColor(SH110X_WHITE);
	oled.print("   ");
	oled.println(pk_value,2);

	oled.setTextSize(1);
	oled.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
	oled.println(" Amp ");

	oled.setTextSize(3);
	oled.setTextColor(SH110X_WHITE);
	oled.print("   ");
	oled.println(value,2);

	oled.display();

}

#endif

