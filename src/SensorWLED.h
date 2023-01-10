/*!
 * @file SensorWLED.h
 *
 * This is part of SensorWLED library for the Arduino platform.
 * Source: https://github.com/berrak/SensorWLED
 *
 * The MIT license.
 *
 */
#ifndef SENSORWLED_H_
#define SENSORWLED_H_

// Other includes needed by the contained implementations
#ifndef ARDUINO
#include <cstdint>
#include <cmath>

// Add the plog logging framework
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ConsoleAppender.h>

        // <-- statement line will be removed
  // <-- statement line will be removed
;    // <-- statement line will be removed

#else  // The SensorWLED library is dependent on the Arduino EEPROM library
#include <EEPROM.h>
#endif

/** Microcontroller EEPROM memory locations */
#define  EEPROM_IDSTART    0xA0  ///< Single EEPROM area: Id and Version
#define  MAXINSTANCES     10     ///< Max number of instantiated EEPROM areas

/** Members in the Version struct */
#define EEPROM_ID 0xA5         ///< EEPROM id marker (never touch)
#define VERSION_MAJOR 0        ///< Semantic versioning (M.m.p)
#define VERSION_MINOR 1        ///< Semantic versioning (M.m.p)
#define VERSION_PATCH 0        ///< Semantic versioning (M.m.p)

/** Used ADC conversion time, in microseconds to (optionally) smooth readings */
#if !defined(US_ADC_CONVERSION_TIME)
    #define US_ADC_CONVERSION_TIME 250
#endif

/** Sets the microcontroller ADC resolution in bits */
typedef enum : uint16_t {
	bits10 = 1023,			///< ADC max resolution is 10 bits
	bits12 = 4095,			///< ADC max resolution is 12 bits
    bits13 = 8191,          ///< ADC max resolution is 13 bits
	bits16 = 65535,			///< ADC max resolution is 16 bits
} AdcResolutionType_e;


/** Microcontroller supply voltage VCC (millivolts) */
typedef enum : uint16_t {
	mv_vcc_3v3 = 3300,		///< Microcontroller VCC: 3V3
	mv_vcc_5v = 5000,		///< Microcontroller VCC: 5V
} VoltageVccType_e;


//-----------------------------------------------------------------------------
/** Decay models for the peak value methods */
typedef enum : uint16_t {
	linear_decay,			///< Peak value decay - linear
	exponential_decay,		///< Peak value decay - exponentially
} DecayModelType_e;

//-----------------------------------------------------------------------------
/*!
    @brief  Unique EEPROM Id and code version.
*/
//-----------------------------------------------------------------------------
typedef struct {
    uint16_t magic_id ;               ///< Unique magic number in EEPROM
    uint16_t major_version;           ///< Semantic version number (m.m.p)
    uint16_t minor_version;           ///< Semantic version number (m.m.p)
    uint16_t patch_version;           ///< Semantic version number (m.m.p)
} VersionType_t;

//-----------------------------------------------------------------------------
/*!
    @brief  ADC pin and ADC channel calibration data.
*/
//-----------------------------------------------------------------------------
typedef struct {
    uint16_t analog_pin;
    uint16_t sample_count;
    uint16_t sample_period;
    float cal_zero_offset;
    float cal_slope;
} CalibrationDataType_t;

//-----------------------------------------------------------------------------
/*!
    @brief  Various static, instant and dynamic (peak) data.
*/
//-----------------------------------------------------------------------------
typedef struct {
    AdcResolutionType_e bits_resolution_adc;
    VoltageVccType_e mv_maxvoltage_adc;
    uint16_t ms_poll_time;
    uint16_t ms_hold_time;
    DecayModelType_e decay_model;
    float decay_rate;
} DynamicDataType_t;

//-----------------------------------------------------------------------------
/*!
    @brief  Track instant and peak DC ADC input readings.

    		The analog microcontroller reading from analog input is
            divided and methods returns these as read, while the other methods
            holds the peak-value while decaying with user set parameters.
*/
//-----------------------------------------------------------------------------
class SensorWLED {

public:

    // Constructor, default: no calibration or averaging smooting are applied
    SensorWLED(uint16_t analog_pin, float mv_offset = 0.0, float slope = 1.0, 
                                                        uint16_t samples = 0 );
    
    // Destructor: Restore pinMode to default
	~SensorWLED(void);

    void begin(DynamicDataType_t const &rDynamicParams);

    // Call continously (in the loop()) for updated ADC values.
	bool updateAnalogRead(void);

    // Maps the input analog value to a value optimized
    // for ADC resolution and max supply voltage.
	double getMappedValue(void);
	double getMappedPeakValue(void);


    // Read stored EEPROM Id and program version.
    VersionType_t readVersionEEPROM(void);

    bool writeCalibrationEEPROM(uint16_t instance, uint32_t crc32);
    CalibrationDataType_t readCalibrationEEPROM(uint16_t instance);

    bool writeDynamicEEPROM(uint16_t instance, uint32_t crc32);
    DynamicDataType_t readDynamicEEPROM(uint16_t instance);

    bool writeCRC32EEPROM(uint32_t address, uint32_t crc32);
    uint32_t readCRC32EEPROM(uint32_t address);

    uint32_t calculateCalibrationDataCRC32(CalibrationDataType_t CalibrationData);
    uint32_t calculateDynamicParamsCRC32(DynamicDataType_t DynamicParams);

    uint32_t cal_crc32;      ///< CRC32 sum of stored EEPROM (begin) calibration data
    uint32_t dyn_crc32;      ///< CRC32 sum of stored EEPROM (begin) dynamic data

    CalibrationDataType_t CalibrationData;  ///< ADC channel setup and calibration
    DynamicDataType_t DynamicParams;        ///< Static and dynamic setup parameters


    // -------------------------------------------------------
    //   Common for all class instances (i.e., ADC channels)
    // -------------------------------------------------------
    static uint16_t getInstanceNumber(void);

    // Each instance has its own 'emulated eeprom' flash area
    // storing calibration, static, and dynamic parameters.
    // Note: 0x100 is for the first instance, 0x200 for the second,
    // 0x0 is an unused area (reseved for the future).
    inline static uint16_t eeprom_area[MAXINSTANCES+1] = 
        {0,0x100,0x200,0x300,0x400,0x500,0x600,0x700,0x800,0x900,0xA00};
 
    // Id and program version is stored at location 'EEPROM_IDSTART'.
    inline static VersionType_t Version = 
             {EEPROM_ID, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH};

    inline static uint16_t instance_counter = 0;

private:

	void setAnalogPin(uint16_t a_pin, uint16_t mode = INPUT);
    uint32_t applyDecay(uint32_t peak_value);

	uint32_t previous_poll_millis_tm;    ///< Holds previous ADC poll time
    uint32_t previous_hold_millis_tm;    ///< Holds previous ADC hold time

	uint32_t raw_input_value;          ///< ADC raw input at bits capability
	double mapped_input_value;       ///< ADC values mapped to VCC range

	uint32_t pk_raw_input_value;       ///< ADC peak input at bits capability
	double pk_mapped_input_value;      ///< ADC peak mapped to VCC range

    // EEPROM and CRC32 methods
    static void generateTableCRC32(uint32_t(&table)[256]);
    static uint32_t updateCRC32(uint32_t (&table)[256], uint32_t initial,
                                             const void* buf, size_t len);
    static bool writeVersionEEPROM(void);

    static bool inline eeprom_version_written_flag = false; ///< EEPROM write flag

};
/* class SensorWLED */

#endif /* SENSORWLED_H_ */
