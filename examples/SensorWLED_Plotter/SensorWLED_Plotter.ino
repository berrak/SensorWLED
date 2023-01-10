#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_Plotter.ino
// Author      : Created by Debinix Team (C). The MIT License (MIT)..
// Version     : Date 2022-12-26.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/berrak/SensorWLED)
// Tested Boards: ESP8266 D1-mini, UM ESP32 TinyPICO.
// I2C displays:
//      1.3" OLED 128x64 SH1106G        (VCC=3V3)
//
// Important Note: Use pull-up resistors (~3.3k) for SDA and SCL.   
// Add an analog signal < 3.3V, via a 10k potentiometer to ANALOG_IN pin. 
//============================================================================

// ------------ debugging ---------------------
// https://github.com/berrak/Rdebug

#define DEBUGLEVEL_OFF  // 'DEBUGLEVEL_OFF' = Setup the Serial Plotter view
//#define DEBUGLEVEL_ON  // 'DEBUGLEVEL_ON' = Serial Monitor and use debug()
#include <Rdebug.h>

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

// ------------ Sensor WLED Probe -----------------------------------
// https://github.com/berrak/SensorWLED
#include <SensorWLED.h>

#if defined(ARDUINO_ARCH_ESP8266)
    #define ANALOG_IN_ONE 0
    #define ADC_RESOLUTION bits10
#elif defined(ARDUINO_ARCH_ESP32)
    #define ANALOG_IN_ONE 33
    #define ADC_RESOLUTION bits12
#endif

// Instantiate one object, with pin#
// Use default calibration values, and no average smoothing
SensorWLED ProbeOne(ANALOG_IN_ONE);
// Add some zero compensation and slope calibration, but no average smoothing
//SensorWLED ProbeOne(ANALOG_IN_ONE, 5, 0.987);
// Use default calibration values, add 10 samples average smoothing
//SensorWLED ProbeOne(ANALOG_IN_ONE,0,1.0,10);

double mv_value ;           // Instant ADC value
double mv_pk_value ;        // Peak ADC value
uint16_t raw_adc_value;     // ADC value (before mapping to voltage)

DynamicDataType_t ParamsOne;// All parameters for instance one

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
	delay(250); 

    oled.begin(i2c_Address, true);
    // oled.setContrast (0); // optionally dim the display
    oled.clearDisplay();

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
    
    #if defined(DEBUGLEVEL_OFF)
        oled.setTextSize(2);
        oled.setTextColor(SH110X_WHITE);
        oled.setCursor(0, 0);
        oled.print("Start the");
        oled.setCursor(0, 24);
        oled.print("IDE Serial");
        oled.setCursor(0, 48);
        oled.print("Plotter!");
        oled.display();
        
        Serial.println("Close Serial Monitor, and start the Serial Plotter now, in <10 seconds.");
        delay(10000);
        
        // Add serial plotter legends to graph
        Serial.println("Instant(mV):,Peak(mV):");
    #endif

}
// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

    if (ProbeOne.updateAnalogRead() == true) {

        mv_value = (double) ProbeOne.getMappedValue();
        mv_pk_value = (double) ProbeOne.getMappedPeakValue();

        #if defined(DEBUGLEVEL_ON)
            raw_adc_value = analogRead(ANALOG_IN_ONE);
            debug("Raw/Instant(mV)/Peak(mV): ");
            debug(raw_adc_value);
            debug(",");   
        #endif
        Serial.print(mv_value);
        Serial.print(",");
        Serial.print(mv_pk_value);
        Serial.println();

        updateOLED1106G();
    }
}

// ------------------------------------------------------------------
//           1.3" OLED 128x64 SH1106G (VCC=3V3)
// ------------------------------------------------------------------
void updateOLED1106G(void) {

    double pk_value = mv_pk_value/1000.0;  // Scale up 1000x for display
    double value = mv_value/1000.0;        // Scale up 1000x for display

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

    // Display static text
    oled.setCursor(0, 48);
    oled.print("  (Volt)");
    oled.display();

}
#endif // ARDUINO

// EOF
