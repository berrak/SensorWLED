#ifdef ARDUINO
//============================================================================
// Name        : SensorWLED_ESP32EEPROM.ino
// Author      : Created by Debinix Team (C). The MIT License (MIT).
// Version     : Date 2022-12-28.
// Description : The 'SensorWLED' project. Find more information about the
// electrical current project at (https://github.com/berrak/SensorWLED)
// Add an analog signal < 3.3V, via a 10k potentiometer to ANALOG_IN pin.
// Tested board: UM ESP32 TinyPICO
//============================================================================

// ------------ debugging ---------------------
// https://github.com/berrak/Rdebug
// DEBUGLEVEL_ON for serial monitor view.
// DEBUGLEVEL_OFF for less debug messages.
#define DEBUGLEVEL_ON  
#include <Rdebug.h>

// ---------------------- 4x20 LCD I2C display  --------------------
// URL: https://github.com/gavinlyonsrepo/HD44780_LCD_PCF8574
#include<HD44780_LCD_PCF8574.h>
int i2c_address = 0x27;     
//          rows , cols ,I2C address
HD44780LCD lcd( 4, 20, i2c_address); // instantiate an LCD object

// ------------ Sensor WLED Probe -----------------------------------
// https://github.com/berrak/SensorWLED
#include <SensorWLED.h>

#if defined(ARDUINO_ARCH_ESP32)
    #define ANALOG_IN_ONE 33
    #define ANALOG_IN_TWO 32
    #define ADC_RESOLUTION bits12
    // Instantiate two objects, with pin#.
    // Use default calibration values, and no average smoothing
    SensorWLED ProbeOne(ANALOG_IN_ONE);
    SensorWLED ProbeTwo(ANALOG_IN_TWO);
    double mv_value_2 ;     // Instant ADC value
    double mv_pk_value_2 ;	// Peak ADC value
#else
    #error This example is for ESP32 only!
#endif

double mv_value ;     // Instant ADC value
double mv_pk_value ;  // Peak ADC value
DynamicDataType_t ParamsOne;    // ADC first channel
DynamicDataType_t ParamsTwo;    // ADC second channel

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup() {
    Serial.begin(9600);
	delay(250); 

    lcd.PCF8574_LCDInit(LCDCursorTypeOff); // removes flickering cursor
    lcd.PCF8574_LCDClearScreen();
    lcd.PCF8574_LCDBackLightSet(true);

    // --------- SensorWLED setup -----------------
    ParamsOne = {
        .bits_resolution_adc = ADC_RESOLUTION,
        .mv_maxvoltage_adc = mv_vcc_3v3,
        .ms_poll_time = 250,
        .ms_hold_time = 1000,  
        .decay_model = exponential_decay,
        .decay_rate = 1,
    };

    ParamsTwo = {
        .bits_resolution_adc = ADC_RESOLUTION,
        .mv_maxvoltage_adc = mv_vcc_3v3,
        .ms_poll_time = 500,
        .ms_hold_time = 750,
        .decay_model = linear_decay,
        .decay_rate = 0.5,      // always less than 1
    };

    // Sets all parameters
    
    ProbeOne.begin(ParamsOne); 
    ProbeTwo.begin(ParamsTwo);   

    Serial.println("Setup completed.");

    lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("Setup completed");

    // EEPROM version data
    showVersionLCD();
    delay(3000);
    lcd.PCF8574_LCDClearScreen();

    // EEPROM calibration data - instance 1, i.e. first ADC channel used
    showCalibrationLCD(1);
    delay(3000);
    lcd.PCF8574_LCDClearScreen();
    showDynamicLCD(1);
    delay(3000);
    lcd.PCF8574_LCDClearScreen();
    showCRC32SumsLCD(1);
    delay(3000);

    // EEPROM calibration data - instance 2, i.e. second ADC channel used
    lcd.PCF8574_LCDClearScreen();
    showCalibrationLCD(2);      
    delay(3000);
    lcd.PCF8574_LCDClearScreen();
    showDynamicLCD(2);         
    delay(3000);
    lcd.PCF8574_LCDClearScreen();
    showCRC32SumsLCD(2);  
    delay(3000);

    lcd.PCF8574_LCDClearScreen();
    Serial.println();

}
// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

    if (ProbeOne.updateAnalogRead() == true) {
        mv_value = (double) ProbeOne.getMappedValue();
        mv_pk_value = (double) ProbeOne.getMappedPeakValue();

        debug(" (ch1) Instant/Peak (mV): ");
        Serial.print(mv_value);
        Serial.print(",");
        Serial.print(mv_pk_value);

        debug(" (ch2) Instant/Peak (mV): ");
        Serial.print(",");
        Serial.print(mv_value_2);
        Serial.print(",");
        Serial.println(mv_pk_value_2);

        #if defined(DEBUGLEVEL_ON)
            update4x20Display(mv_value, mv_pk_value, mv_value_2, mv_pk_value_2);
        #endif

    }

    if (ProbeTwo.updateAnalogRead() == true) {
        mv_value_2 = (double) ProbeTwo.getMappedValue();
        mv_pk_value_2 = (double) ProbeTwo.getMappedPeakValue();

        debug(" (ch1) Instant/Peak (mV): ");
        Serial.print(mv_value);
        Serial.print(",");
        Serial.print(mv_pk_value);

        debug(" (ch2) Instant/Peak (mV): ");
        Serial.print(",");
        Serial.print(mv_value_2);
        Serial.print(",");
        Serial.println(mv_pk_value_2);

        #if defined(DEBUGLEVEL_ON)
            update4x20Display(mv_value, mv_pk_value, mv_value_2, mv_pk_value_2);
        #endif        

    }

}
// ------------------------------------------------------------------
// HELPERS     HELPERS     HELPERS     HELPERS     HELPERS
// ------------------------------------------------------------------
//  Show EEPROM magic Id and version
// ------------------------------------------------------------------
void showVersionLCD(void) {

    // Retrieve EEPROM stored data
    VersionType_t MyVersion = ProbeOne.readVersionEEPROM();

	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 0);
    lcd.PCF8574_LCDSendString("Unique Id: ");
	lcd.print(MyVersion.magic_id,HEX);
    lcd.PCF8574_LCDGOTO(LCDLineNumberThree, 0);
    lcd.PCF8574_LCDSendString("Version(M.m.p)");
    lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
    lcd.print(MyVersion.major_version);
    lcd.print(".");
    lcd.print(MyVersion.minor_version);
    lcd.print(".");
    lcd.print(MyVersion.patch_version);


    // Magic EEPROM Id marker and Program version
    debugln("Id and Version data from EEPROM");

    debug("Id (0xA5=165)");
    debugln(MyVersion.magic_id);
    debug("Major version (0): ");
    debugln(MyVersion.major_version);
    debug("Minor version (1): ");
    debugln(MyVersion.minor_version);
    debug("Patch version (0): ");
    debugln(MyVersion.patch_version);
    debugln();

}
// ------------------------------------------------------------------
//  Show ADC calibaration data for ADC channel (and pin#)
// ------------------------------------------------------------------
void showCalibrationLCD(uint16_t instance) {

    CalibrationDataType_t MyCalibration;
    // Retrieve calibrations EEPROM stored data
    switch (instance) {
        case 1: 
            MyCalibration = ProbeOne.readCalibrationEEPROM(instance);
            break;
        #if !(defined ARDUINO_ARCH_ESP8266)    
            case 2: 
                MyCalibration = ProbeTwo.readCalibrationEEPROM(instance);
                break;  
        #endif
        default: 
            if(instance > SensorWLED::getInstanceNumber()) {
                Serial.println("ERROR - To many instances given");
            }
            break;
    }

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
    lcd.PCF8574_LCDSendString("Pin:");
    lcd.print(MyCalibration.analog_pin);
	lcd.PCF8574_LCDSendString(" Slope: ");
    lcd.print(MyCalibration.cal_slope);

	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 0);
	lcd.PCF8574_LCDSendString("Offset: ");
    lcd.print(MyCalibration.cal_zero_offset,0);
    lcd.PCF8574_LCDSendString(" mV");

	lcd.PCF8574_LCDGOTO(LCDLineNumberThree , 0);
	lcd.PCF8574_LCDSendString("Sample Cnt: ");
    lcd.print(MyCalibration.sample_count);  

	lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
    lcd.PCF8574_LCDSendString("Sample Tm: ");
	lcd.print(MyCalibration.sample_period);
    lcd.PCF8574_LCDSendString(" ms");    

    // Serial monitor output
    debug("------------------ Instance: ");
    debugln(instance);

    debug("ESP32 Analog pin: ");
    debugln(MyCalibration.analog_pin);

	debug("Cal. offset: ");
    debug(MyCalibration.cal_zero_offset);
    debugln(" mV");

	debug("Cal. slope: ");
    debugln(MyCalibration.cal_slope);

	debug("Sample Count: ");
    debugln(MyCalibration.sample_count);  

    debug("Sample Time: ");
	debug(MyCalibration.sample_period);
    debugln(" ms");    

}
// ------------------------------------------------------------------
//  Show static and dynamic parameters
// ------------------------------------------------------------------
void showDynamicLCD(uint16_t instance) {

    DynamicDataType_t MyParams;
    // Retrieve static/dynamic EEPROM stored data
    switch (instance) {
        case 1: 
            MyParams = ProbeOne.readDynamicEEPROM(instance);
            break;
        #if !(defined ARDUINO_ARCH_ESP8266)    
            case 2: 
                MyParams = ProbeTwo.readDynamicEEPROM(instance);
                break;  
        #endif
        default: 
            if(instance > SensorWLED::getInstanceNumber()) {
                    Serial.println("ERROR - To many instances given");
            }
            break;
    }

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("Bits: ");
    lcd.print(MyParams.bits_resolution_adc);

	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 0);
	lcd.PCF8574_LCDSendString("ADC max: ");
    lcd.print(MyParams.mv_maxvoltage_adc);
    lcd.PCF8574_LCDSendString(" mV");

	lcd.PCF8574_LCDGOTO(LCDLineNumberThree , 0);
	lcd.PCF8574_LCDSendString("P/H-tm: ");
    lcd.print(MyParams.ms_poll_time);
	lcd.PCF8574_LCDSendString("/");    
    lcd.print(MyParams.ms_hold_time);
    lcd.PCF8574_LCDSendString(" ms");  

	lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
    lcd.PCF8574_LCDSendString("Model/Rate: ");
	lcd.print(MyParams.decay_model);
    lcd.PCF8574_LCDSendString("/");    
    lcd.print(MyParams.decay_rate);


    debug("------------------ Instance: ");
    debugln(instance);

    // Serial monitor
    debug("ADC max bits resolution: ");
    debugln(MyParams.bits_resolution_adc);
    debug("ADC voltage range, max value (3300): ");
    debugln(MyParams.mv_maxvoltage_adc);

    debug("Read Poll time: ");
    debugln(MyParams.ms_poll_time);
    debug("Read hold: ");
    debugln(MyParams.ms_hold_time);

    debug("read Decay model (0=linear,1=exponential): ");
    debugln(MyParams.decay_model);
    debug("Read Decay rate: ");
    debugln(MyParams.decay_rate);
    debugln();

}

// ------------------------------------------------------------------
//  Show CRC32 sums for 'CalibrationData' & 'DynamicData' structs.
// ------------------------------------------------------------------
void showCRC32SumsLCD(uint16_t instance) {

    uint32_t crc_cal = 0; 
    uint32_t crc_dyn = 0; 

    // Retrieve the CRC32 sums for struct data
    switch (instance) {
        case 1: 
            crc_cal = ProbeOne.cal_crc32;
            crc_dyn = ProbeOne.dyn_crc32;
            break;
        #if !(defined ARDUINO_ARCH_ESP8266)    
            case 2: 
            crc_cal = ProbeTwo.cal_crc32;
            crc_dyn = ProbeTwo.dyn_crc32;
            break;
        #endif
        default: 
            if(instance > SensorWLED::getInstanceNumber()) {
                Serial.println("ERROR - To many instances given");
            }
            break;
    }

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("Instance: ");
    lcd.print(instance);

	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo , 0);
	lcd.PCF8574_LCDSendString("Cal CRC32: ");
    lcd.print(crc_cal,HEX);

	lcd.PCF8574_LCDGOTO(LCDLineNumberThree , 0);
	lcd.PCF8574_LCDSendString("Instance: ");
    lcd.print(instance);

	lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
	lcd.PCF8574_LCDSendString("Dyn CRC32: ");
    lcd.print(crc_dyn,HEX);


    // Serial monitor
    debug("------------------ Instance: ");
    debugln(instance);

    debug("CRC32# Struct 'CalibrationData': ");
    debugln(crc_cal);

    debug("CRC32# Struct 'DynamicData: ");
    debugln(crc_dyn);

}
// ------------------------------------------------------------------
//  Show analog input values on LCD4x20 display
// ------------------------------------------------------------------
void update4x20Display(double mv_value, double mv_pk_value, 
                           double mv_value_2, double mv_pk_value_2) {

	lcd.PCF8574_LCDGOTO(LCDLineNumberOne , 0);
	lcd.PCF8574_LCDSendString("1-Pk(mV):" );
    lcd.print(mv_pk_value,1);

	lcd.PCF8574_LCDGOTO(LCDLineNumberTwo, 0);
	lcd.PCF8574_LCDSendString("1-Instant(mV):");
	lcd.print(mv_value,1);

	lcd.PCF8574_LCDGOTO(LCDLineNumberThree , 0);
	lcd.PCF8574_LCDSendString("2-Pk(mV):");
    lcd.print(mv_pk_value_2,1);

	lcd.PCF8574_LCDGOTO(LCDLineNumberFour, 0);
	lcd.PCF8574_LCDSendString("2-Instant(mV):");
	lcd.print(mv_value_2,1);

}

#endif // ARDUINO

// EOF