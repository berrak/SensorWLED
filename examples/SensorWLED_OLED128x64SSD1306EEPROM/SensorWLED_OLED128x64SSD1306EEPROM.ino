#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_OLED128x64SSD1306EEPROM.ino
// Author      : Created by Debinix Team (c). The MIT License (MIT).
// Version     : Date 2022-09-15.
// Description : The 'SensorWLED' project. Find more information about the
// measurement WLED project at (https://github.com/DebinixTeam/SensorWLED).
// Source code: (https://github.com/berrak/SensorWLED)
//============================================================================

#include <SPI.h>
#include <Wire.h>
// URL: https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>   // 0.96" OLED 128x64 SSD1306
// Set OLED width, height, use I2C, and no-reset-pin on display
int i2c_address = 0x3C;
Adafruit_SSD1306 oled = Adafruit_SSD1306(128, 64, &Wire, -1);

#include <SensorWLED.h>
#include <EEPROM.h>

// #define DEBUG
#ifdef DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// Instantiate a SensorWLED object
SensorWLED WledOne;

double mv_value ;		// Analog momentanous value
double mv_pk_value ;	// Peak analog value

/* ----------------------------------------------------------------------------
 * setup: Setup the OLED display with Adafruit library and SensorWLED
 * ----------------------------------------------------------------------------
 */
void setup() {
    Serial.begin(9600);
    delay(250); // wait for the OLED to power up
    oled.begin(SSD1306_SWITCHCAPVCC, i2c_address);

    Serial.println("Setup completed!");
    delay(1000);  // Normally, the only delay() call.

    // ESP8266
    DataEEPROMType_t ParamsOne = {
        .analog_pin = 0,
        .bits_resolution_adc = bits10,      // For ESP32 use 'bits12'
        .mv_maxvoltage_adc = mv_vcc_3v3,
        .ms_poll_time = 500,
        .ms_hold_time = 250,
        .decay_model = exponential_decay,
        .decay_rate = 0.75,
    };

    WledOne.begin(ParamsOne);

    // Displayed CRC32# for our structs.
    displayEEPROMContent();

}
/* ----------------------------------------------------------------------------
 * loop: Do not add any long delay statements in loop().
 *       It will ruin any feel for momentanous updates.
 * ----------------------------------------------------------------------------
 */
void loop() {

    // Current sensor updates every 500 ms (ms_poll_time).
    if (WledOne.updateAnalogRead() == true) {
        mv_value = (double) WledOne.getMappedValue();
        mv_pk_value = (double) WledOne.getMappedPeakValue();
        updateSmallDisplay();
    }

    debug("Analog input: ");
    debug(mv_value);
    debugln(" mV");
    debug("Peak input  : ");
    debug(mv_pk_value);
    debugln(" mV");

#ifdef DEBUG
    delay(1000);  // Do not completly hog the serial monitor
#endif

}
/*
 *  updateSmallDisplay: Writes read values to SSD1306 OLED display
 */

void updateSmallDisplay(void) {

    double pk_value = mv_pk_value/1000.0;  // Scale up 1000x for display
    double value = mv_value/1000.0;        // Scale up 1000x for display

    oled.clearDisplay();

    // Set heading
    oled.setTextSize(2);
    oled.setTextColor(SSD1306_WHITE);
    // x-, y-coordinates
    oled.setCursor(0, 0);
    // Display static text
    oled.println("Peak  Amp");

    // Print the values, with new larger font
    oled.setTextColor(SSD1306_WHITE);
    // x-, y-coordinates
    oled.setCursor(0, 32);
    // Display static text
    oled.print(pk_value, 2);

    oled.setCursor(64, 32);
    // Display static text
    oled.print(value, 2);

    oled.display();

}

//-----------------------------------------------------------------------------
//                           DEBUG_EEPROM
//-----------------------------------------------------------------------------


// Read what is stored in CRC32-EEPROM
void displayEEPROMContent(void){

    CRC32Type_t MyCRC = WledOne.getCRC32EEPROM();

    // Retrieved CRC32-sums

    Serial.print("CRC32# Struct Version (internal): 0x");
    Serial.println(MyCRC.crc_version, HEX);

    Serial.print("CRC32# Struct UserData (i.e. setup()): 0x");
    Serial.println(MyCRC.crc_user_data, HEX);

    Serial.print("EEPROM writes during this setup(): ");
    Serial.println(WledOne.getSetupWritesEEPROM());

    delay(2000);
}

#endif // ARDUINO

