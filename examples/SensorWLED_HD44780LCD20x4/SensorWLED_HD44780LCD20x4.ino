#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_HD44780LCD20x4.ino
// Author      : Created by Debinix Team (C). Licensed under GPL-3.0.
// Version     : Date 2022-08-29.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/DebinixTeam/SensorWLED)
// Tested: Arduino Uno,
//============================================================================
// URL: https://github.com/gavinlyonsrepo/HD44780_LCD_PCF8574
#include<HD44780_LCD_PCF8574.h>  // for a wide range of controllers
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

//            analog_pin, ADC bits, vcc, ms_poll_tm, (ms_refresh_tm)
SensorWLED WLED1 = SensorWLED(PIN_A0, Bits10, Vcc5v0, 500, 1000);

//          rows , cols ,I2C address
HD44780LCD lcd( 4, 20, 0x27); // instantiate an LCD object

double mv_value ;
double pk_mv_value ;

void setup() {
	Serial.begin(BAUDRATE);

	lcd.PCF8574_LCDInit(LCDCursorTypeOff); // removes flickering cursor
	lcd.PCF8574_LCDClearScreen();
	lcd.PCF8574_LCDBackLightSet(true);

	// set analog read pin and ...
	WLED1.begin(WledLCD);
	Serial.println("Setup completed!");

	delay(1000);  // the only delay() call.
}

// Do not add any long delay statements in loop().
// It will ruin any feel for momentanous updates.
void loop() {

	// Current sensor
	WLED1.updateAnalogRead();

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

	update4x20Display();

}

void update4x20Display(void) {

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("Measured peak value");
	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 0);
	lcd.print(pk_mv_value);

	lcd.PCF8574_LCDGOTO(LCDLineNumberThree , 0);
	lcd.PCF8574_LCDSendString("Momentanous value");
	lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
	lcd.print(mv_value);

}

#endif

