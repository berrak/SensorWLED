#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_VU.ino
// Author      : Created by Debinix Team (c). The MIT License (MIT).
// Version     : Date 2022-12-29.
// Description : The 'SensorWLED' project.
// Source code: (https://github.com/berrak/SensorWLED)
// Tested Boards: ESP8266 D1-mini, UM ESP32 TinyPICO.
// I2C displays:
//      0.96" OLED 128x64 SSD1306       (VCC=3V3)
//      1.3" OLED 128x64 SH1106G        (VCC=3V3)
//
// Important Note: Use pull-up resistors (~3.3k) for SDA and SCL.    
// Add an analog signal < 3.3V, via a 10k potentiometer to ANALOG_IN pin.
//============================================================================

// ------------ debugging ---------------------
#define DEBUGLEVEL_ON  // DEBUGLEVEL_OFF disables serial monitor output
#include <Rdebug.h>

//
// Uncomment one of the I2C displays for test
//

// #define SSD1306         // 0.96" OLED 128x64
#define SH1106G      // 1.3" OLED 128x64

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
// Use default calibration values, and no average smoothing
SensorWLED ProbeOne(ANALOG_IN_ONE);

static const unsigned char PROGMEM VU_meter[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x60, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x09, 0x04, 0x80, 0x21, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x98, 0x08, 0x06, 0x03, 0x80, 0x21, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xA4, 0x10, 0x09, 0x00, 0x80, 0x21, 0x20, 0x07, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xA4, 0x10, 0x06, 0x03, 0x00, 0x20, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x71, 0x80, 0xA4, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x0A, 0x40, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3C, 0x00, 0x00,
  0x00, 0x00, 0x3A, 0x40, 0x00, 0x00, 0x02, 0x01, 0x00, 0x40, 0x80, 0x07, 0x00, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x42, 0x40, 0x00, 0x08, 0x02, 0x01, 0x08, 0x40, 0x80, 0x00, 0x00, 0x38, 0x00, 0x00,
  0x00, 0x00, 0x79, 0x80, 0x04, 0x08, 0x02, 0x01, 0x08, 0x81, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x02, 0x01, 0x08, 0x81, 0x11, 0x04, 0x00, 0x38, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x02, 0x01, 0x08, 0x81, 0x21, 0x04, 0x00, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x84, 0x02, 0x04, 0x0F, 0xFF, 0xFF, 0xC3, 0xE2, 0x04, 0x00, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x00, 0xC2, 0x01, 0x07, 0xF0, 0x00, 0x00, 0x3B, 0xFE, 0x08, 0x40, 0x40, 0x08, 0x00,
  0x00, 0xFE, 0x00, 0x62, 0x01, 0xF8, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xE8, 0x40, 0x80, 0x7F, 0x00,
  0x00, 0x00, 0x00, 0x21, 0x1E, 0x00, 0x04, 0x00, 0x80, 0x00, 0x7F, 0xFE, 0x80, 0x80, 0x08, 0x00,
  0x00, 0x00, 0x03, 0x31, 0xE0, 0x00, 0x04, 0x00, 0x80, 0x04, 0x01, 0xFF, 0xC1, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x07, 0x1E, 0x00, 0x40, 0x00, 0x00, 0x00, 0x04, 0x00, 0x1F, 0xFA, 0x00, 0x08, 0x00,
  0x00, 0x00, 0x07, 0xF0, 0x00, 0x40, 0x3B, 0x07, 0x60, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x34, 0x81, 0x90, 0xCC, 0xC0, 0x00, 0x3F, 0xC0, 0x00, 0x00,
  0x00, 0x00, 0x0C, 0x00, 0x03, 0x30, 0x0C, 0x82, 0x90, 0x53, 0x20, 0x00, 0x07, 0xF8, 0x00, 0x00,
  0x00, 0x00, 0x70, 0x40, 0x00, 0xC8, 0x3B, 0x02, 0x60, 0x53, 0x20, 0x00, 0x00, 0xFE, 0x00, 0x00,
  0x00, 0x01, 0x80, 0x20, 0x01, 0xC8, 0x00, 0x00, 0x00, 0x4C, 0xC0, 0x00, 0x00, 0x3F, 0x80, 0x00,
  0x00, 0x06, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00,
  0x00, 0x08, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFC, 0x00,
  0x00, 0x30, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00,
  0x00, 0x00, 0x40, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
  0x00, 0x00, 0xA0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x02, 0x02, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x06, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x8C, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x30, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x70, 0x19, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint16_t h_meter = 65;    // horizontal center for needle animation
uint16_t v_meter = 85;    // vertical center for needle animation (outside of dislay limits)
uint16_t r_meter = 80;    // length of needle animation or arch of needle travel

// Use the peak value (hold value) for the VU-meter display
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
        oled.begin(i2c_Address, true);
        // oled.setContrast (0); // optionally dim the display
    #endif

    // --------- SensorWLED setup -----------------
    ParamsOne = {
        .bits_resolution_adc = ADC_RESOLUTION,
        .mv_maxvoltage_adc = mv_vcc_3v3,
        .ms_poll_time = 250,
        .ms_hold_time = 150,  
        .decay_model = exponential_decay,
        .decay_rate = 0.75,
    };

    ProbeOne.begin(ParamsOne);  // Sets all parameters

    Serial.println("Setup completed.");
}
// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

    if (ProbeOne.updateAnalogRead() == true) {
        //mv_value = (double) ProbeOne.getMappedValue();
        mv_pk_value = (double) ProbeOne.getMappedPeakValue();

        debug("Pk: ");
        debug(mv_pk_value);
        debugln(" mV");

        updateVuDisplay();

    }

    yield();

}
// ------------------------------------------------------------------
// HELPERS     HELPERS     HELPERS     HELPERS     HELPERS
// ------------------------------------------------------------------
// ------------------------------------------------------------------
//           0.96" OLED 128x64 SSD1306 (VCC=3V3)
// ------------------------------------------------------------------
#if defined(SSD1306)
void updateVuDisplay(void) {
	//  100% on scale if metervalue has the value= "45".
	//  Multiplication factor for ESP's is 45/Vcc3v3, i.e. 45/3.3 = 13.63 (100%)
    
	double pk_value = mv_pk_value/1000.0;   // scale up mv 1000x for V-display
	double meter_value = pk_value * 13.63 ; // convert volts to arrow information

	oled.clearDisplay();
	meter_value = meter_value - 34;           // shifts needle to zero position
	oled.clearDisplay();                     // refresh display for next step
	oled.drawBitmap(0, 0, VU_meter, 128, 64, SSD1306_WHITE);   // background bmp
	uint16_t x = (h_meter + (sin(meter_value / 57.296) * r_meter)); // meter needle horizontal coordinate
	uint16_t y = (v_meter - (cos(meter_value / 57.296) * r_meter)); // meter needle vertical coordinate
	oled.drawLine(x, y, h_meter, v_meter, SSD1306_WHITE);     // draws needle
	oled.display();

}
#elif defined(SH1106G)
// ------------------------------------------------------------------
//              1.3" OLED 128x64 SH1106G  (VCC=3V3)
// ------------------------------------------------------------------
void updateVuDisplay(void) {
	//  100% on scale if metervalue has the value= "45".
	//  Multiplication factor for ESP's is 45/Vcc3v3, i.e. 45/3.3 = 13.63 (100%)

	double pk_value = mv_pk_value/1000.0;  // scale up mv 1000x for V-display
	double meter_value = pk_value * 13.63 ; // convert volts to arrow information

	oled.clearDisplay();
	meter_value = meter_value - 34;           // shifts needle to zero position
	oled.clearDisplay();                    // refresh display for next step
	oled.drawBitmap(0, 0, VU_meter, 128, 64, SH110X_WHITE);   // background bmp
	uint16_t x = (h_meter + (sin(meter_value / 57.296) * r_meter)); // meter needle horizontal coordinate
	uint16_t y = (v_meter - (cos(meter_value / 57.296) * r_meter)); // meter needle vertical coordinate
	oled.drawLine(x, y, h_meter, v_meter, SH110X_WHITE);     // draws needle
	oled.display();

}

#endif

#endif  // ARDUINO

// EOF