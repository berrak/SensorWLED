#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED.ino
// Author      : Created by Debinix Team (C). The MIT License (MIT).
// Version     : Date 2022-08-29.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/berrak/SensorWLED)
// Add an analog signal < 3.3V, via a 10k potentiometer to ANALOG_IN pin.
// Tested Boards: ESP8266 D1-mini, UM ESP32 TinyPICO.
//============================================================================
// ------------ debugging ---------------------
#define DEBUGLEVEL_OFF  // 'DEBUGLEVEL_ON' adds the raw ADC reading
#include <Rdebug.h>

#if defined(ARDUINO_ARCH_ESP8266)
    #define ANALOG_IN_ONE 0
    #define ADC_RESOLUTION bits10
#elif defined(ARDUINO_ARCH_ESP32)
    #define ANALOG_IN_ONE 33
    #define ADC_RESOLUTION bits12
#else
    #error Unsupported or not tested microcontroller architecture
#endif

// ------------ Sensor WLED Probe -----------------------------------
// https://github.com/berrak/SensorWLED
#include <SensorWLED.h>
// Instantiate one object, with pin#
// Use default calibration values, and no average smoothing
SensorWLED ProbeOne(ANALOG_IN_ONE);
// .. compensate for zero voltage mV offset and Vref (slope) with e.g.,
// SensorWLED ProbeOne(ANALOG_IN_ONE, 5, 0.987);  

double mv_value ;           // Instant ADC value
double mv_pk_value ;        // Peak ADC value
uint16_t raw_adc_value;     // ADC value (before mapping to voltage)
DynamicDataType_t ParamsOne;

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
	delay(250); 

    // --------- SensorWLED setup -----------------
    ParamsOne = {
        .bits_resolution_adc = ADC_RESOLUTION,
        .mv_maxvoltage_adc = mv_vcc_3v3,
        .ms_poll_time = 250,
        .ms_hold_time = 1000,  
        .decay_model = exponential_decay,
        .decay_rate = 1,
    };

    ProbeOne.begin(ParamsOne);  // Sets all parameters

    Serial.println("Setup completed.");
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
        #else
            Serial.print("Instant(mV)/Peak(mV): ");
        #endif
        Serial.print(mv_value);
        Serial.print(",");
        Serial.print(mv_pk_value);
        Serial.println();
    }

}
#endif // ARDUINO

// EOF
