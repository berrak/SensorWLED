#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_OLED128x64SSD1306EXP.ino
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

#define DEBUG_EEPROM    // Show saved EEPROM data

#define DEBUG
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

#ifdef DEBUG_EEPROM
    displayEEPROMContent();
#endif

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

#ifdef DEBUG_EEPROM

// Read what is stored in EEPROM
void displayEEPROMContent(void){

    DataEEPROMType_t ReadDataOne = WledOne.retrieveEEPROM();
    VersionType_t MyVersion = WledOne.readVersionEEPROM();

    // Magic EEPROM Id marker and Program version
    Serial.print("Version data from EEPROM: ");

    Serial.print("Magic Id (A5): ");
    Serial.println(MyVersion.magic_id);
    Serial.print("Major version (0): ");
    Serial.println(MyVersion.major_version);
    Serial.print("Minor version (1): ");
    Serial.println(MyVersion.minor_version);
    Serial.print("Patch version (0): ");
    Serial.println(MyVersion.patch_version);
    Serial.println();

    // User configuration
    Serial.print("EEPROM saved user configuration data: ");

    Serial.print("Analog pin (0): ");
    Serial.println(ReadDataOne.analog_pin);
    Serial.print("ADC max bits resolution (1024-1): ");
    Serial.println(ReadDataOne.bits_resolution_adc);
    Serial.print("ADC voltage range, max value (3300): ");
    Serial.println(ReadDataOne.mv_maxvoltage_adc);

    Serial.print("Read Poll time (500): ");
    Serial.println(ReadDataOne.ms_poll_time);
    Serial.print("Read hold (250): ");
    Serial.println(ReadDataOne.ms_hold_time);

    Serial.print("read Decay model (0=linear,1=exponential): ");
    Serial.println(ReadDataOne.decay_model);
    Serial.print("Read Decay rate (0.75): ");
    Serial.println(ReadDataOne.decay_rate);
    Serial.println();

    delay(5000);

}

#endif // DEBUG_EEPROM

#endif // ARDUINO

