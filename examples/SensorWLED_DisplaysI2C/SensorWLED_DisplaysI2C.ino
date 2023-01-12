#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_DisplaysI2C.ino
// Author      : Created by Debinix Team (c). The MIT License (MIT).
// Version     : Date 2022-12-19.
// Description : The 'SensorWLED' project.
// Source code: (https://github.com/berrak/SensorWLED)
// Tested Boards: ESP8266 D1-mini, UM ESP32 TinyPICO.
// I2C displays:
//      0.96" OLED 128x64 SSD1306       (VCC=3V3)
//      1.3" OLED 128x64 SH1106G        (VCC=3V3)
//      2x16 or 4x20 LCD I2C display    (VCC=5V)
//
// Important Note: Use pull-up resistors (~3.3k) for SDA and SCL.  
// Add an analog signal < 3.3V, via a 10k potentiometer to ANALOG_IN pin.  
//============================================================================

// ----------------------------------------------------------------------
// If using the SensorWLED library to measure electrical current with
// the 'SensorWLED-DC-Sensor Board, see row ~90 (used scaling factor).
// https://github.com/berrak/WLED-DC-Sensor-Board
// #define AMPERE  // Uncomment to enable the electrical current display.
// ----------------------------------------------------------------------

// ------------ debugging ---------------------
#define DEBUGLEVEL_OFF  // DEBUGLEVEL_OFF disables serial monitor output
#include <Rdebug.h>

//
// Uncomment one of the I2C displays for test
//
//#define SSD1306         // 0.96" OLED 128x64
#define SH1106G      // 1.3" OLED 128x64
//#define LCD4x20      // 4 rows by 20 columns
//#define LCD2x16      // 2 rows by 16 columns

#if defined(SSD1306)
// ------------- 0.96" OLED 128x64 SSD1306 -------------------------
// URL: https://github.com/adafruit/Adafruit_SSD1306
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Set OLED width, height, use I2C, and no-reset-pin on display
int i2c_address = 0x3C;
Adafruit_SSD1306 oled = Adafruit_SSD1306(128, 64, &Wire, -1);

#elif defined(SH1106G)
// ------------- 1.3" OLED 128x64 SH1106G --------------------------
// URL: https://github.com/adafruit/Adafruit_SH110x
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#define i2c_Address 0x3c //initialize with 0x3C, common eBay OLED's
// If you don't want the 'Adafruit-flash logo' at start-up. In their
// header file, uncomment row 35, i.e. //#define SH110X_NO_SPLASH
						// width, height ,I2C, no-reset-pin
Adafruit_SH1106G oled = Adafruit_SH1106G(128, 64, &Wire, -1);

#elif defined(LCD2x16) || defined(LCD4x20)
// -------------- 2x16 or 4x20 LCD I2C display  --------------------
// URL: https://github.com/gavinlyonsrepo/HD44780_LCD_PCF8574
#include<HD44780_LCD_PCF8574.h>
int i2c_address = 0x27;    
    #if defined(LCD2x16)
    //          rows , cols ,I2C address
    HD44780LCD lcd( 2, 16, i2c_address); // instantiate an LCD object
    #elif defined(LCD4x20)
    HD44780LCD lcd( 4, 20, i2c_address); // instantiate an LCD object
    #endif

#else
    #error Display is not defined
#endif

// ------------ Sensor WLED Probe -----------------------------------
// https://github.com/berrak/SensorWLED
#if defined(ARDUINO_ARCH_ESP8266)
    #define ANALOG_IN_ONE 0
    #define ADC_RESOLUTION bits10
#elif defined(ARDUINO_ARCH_ESP32)
    #define ANALOG_IN_ONE 33
    #define ADC_RESOLUTION bits12
#endif


#include <SensorWLED.h>
// Instantiate one object, with pin#
// Use offset compensation for current measurement
#if defined(AMPERE)
SensorWLED ProbeOne(ANALOG_IN_ONE, 45,1);   
#define AMP_SCALEFACTOR 0.3  // Gain * Rshunt = 200 * 0.0015 Ohm
#else
// Use default calibration values, and no average smoothing
SensorWLED ProbeOne(ANALOG_IN_ONE);
#endif

double mv_value ;     // Instant ADC value
double mv_pk_value ;  // Peak ADC value
DynamicDataType_t ParamsOne;

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
	delay(250); 

    #if defined(SSD1306)
        oled.begin(SSD1306_SWITCHCAPVCC, i2c_address);
    #elif defined(SH1106G)
    	delay(1000); 
        oled.begin(i2c_Address, true);
        // oled.setContrast (0); // optionally dim the display
    #elif defined(LCD4x20) || defined(LCD2x16)
        lcd.PCF8574_LCDInit(LCDCursorTypeOff); // removes flickering cursor
        lcd.PCF8574_LCDClearScreen();
        lcd.PCF8574_LCDBackLightSet(true);
    #endif

    // --------- SensorWLED setup -----------------
    ParamsOne = {
        .bits_resolution_adc = ADC_RESOLUTION,
        .mv_maxvoltage_adc = mv_vcc_3v3,
        .ms_poll_time = 250,
        .ms_hold_time = 1000,  
        .decay_model = exponential_decay,
        .decay_rate = 1,
    };

    // Sets all parameters
    ProbeOne.begin(ParamsOne); 

    Serial.println("Setup completed.");
}
// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

    if (ProbeOne.updateAnalogRead() == true) {
        mv_value = (double) ProbeOne.getMappedValue();
        mv_pk_value = (double) ProbeOne.getMappedPeakValue();

        #if defined(AMPERE)
            debug(" (ch1) Instant/Peak (mA): ");
            Serial.print(mv_value/AMP_SCALEFACTOR);
            Serial.print(",");
            Serial.print(mv_pk_value/AMP_SCALEFACTOR);
        #else
            debug(" (ch1) Instant/Peak (mV): ");
            Serial.print(mv_value);
            Serial.print(",");
            Serial.print(mv_pk_value);
        #endif
        
        Serial.println();  

        #if defined(SSD1306)
            updateOLED1306();
        #elif defined(SH1106G)
            updateOLED1106G();
        #elif defined(LCD4x20)
            update4x20LCD();
        #elif defined(LCD2x16)  
            update2x16LCD();  
        #endif

    }

    yield();
}
// ------------------------------------------------------------------
// HELPERS     HELPERS     HELPERS     HELPERS     HELPERS
// ------------------------------------------------------------------
#if defined(SSD1306)
// ------------------------------------------------------------------
//            0.96" OLED 128x64 SSD1306 (VCC=3V3)
// ------------------------------------------------------------------
void updateOLED1306() {

    // Scale up 1000x for display, mV to V.
    double pk_value = mv_pk_value/1000.0; 
    double value = mv_value/1000.0;  

    oled.clearDisplay();

    // Set heading
    oled.setTextSize(2);
    oled.setTextColor(SSD1306_WHITE);
    // x-, y-coordinates
    oled.setCursor(0, 0);
    // Display static text
    oled.println("Peak  DC");

    // Print the values, with new larger font
    oled.setTextColor(SSD1306_WHITE);
    // x-, y-coordinates
    oled.setCursor(0, 32);
    // Display static text
    oled.print(pk_value, 2);

    oled.setCursor(60, 32);
    // Display static text
    oled.print(value, 2);

    oled.display();

}
#elif defined(SH1106G)
// ------------------------------------------------------------------
//           1.3" OLED 128x64 SH1106G (VCC=3V3)
// ------------------------------------------------------------------
void updateOLED1106G(void) {

    double pk_value = mv_pk_value/1000.0;  // Scale up 1000x for display
    double value = mv_value/1000.0;        // Scale up 1000x for display

// If using SensorWLED with the current shunt - scale it for Ampere read out
#ifdef AMPERE
    pk_value /= AMP_SCALEFACTOR;
    value /= AMP_SCALEFACTOR;
#endif

    oled.clearDisplay();

    // Set heading
    oled.setTextSize(2);
    oled.setTextColor(SH110X_WHITE);
    // x-, y-coordinates
    oled.setCursor(0, 0);
    // Display static text
    oled.print("Pk: ");
    oled.print(pk_value, 3);

    // Display static text
    oled.setCursor(0, 24);
    oled.print("DC: ");
    // Display static text
    oled.print(value, 3);

    oled.setCursor(0, 48);
    // Display static text
#ifdef AMPERE
    oled.print(" (Ampere)");
#else
    oled.print("  (Volt)");
#endif

    oled.display();

}
#elif defined(LCD4x20)
// ------------------------------------------------------------------
//                4x20 LCD I2C display (VCC=5V)
// ------------------------------------------------------------------
void update4x20LCD(void) {

    // Scale up 1000x for display, mV to V.
    double pk_value = mv_pk_value/1000.0;  
    double value = mv_value/1000.0;    

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("Peak value (V)");
	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 0);
	lcd.print(pk_value, 3);

	lcd.PCF8574_LCDGOTO(LCDLineNumberThree , 0);
	lcd.PCF8574_LCDSendString("Instant value (V)");
	lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
	lcd.print(value, 3);

}

#elif defined(LCD2x16)
// ------------------------------------------------------------------
//               2x16 LCD I2C display (VCC=5V)
// ------------------------------------------------------------------
void update2x16LCD(void) {

    // Scale up 1000x for display, mV to V.
    double pk_value = mv_pk_value/1000.0; 
    double value = mv_value/1000.0;

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("Peak(V): ");
	lcd.PCF8574_LCDGOTO(LCDLineNumberOne, 10);
	lcd.print(pk_value, 3);

	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo , 0);
	lcd.PCF8574_LCDSendString("  DC(V): ");
	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 10);
	lcd.print(value, 3);

}
#endif

#endif  // ARDUINO

// EOF
