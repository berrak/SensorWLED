/*!
 * @file SensorWLED.h
 *
 * This is part of SensorWLED library for the Arduino platform.
 * The SensorWLED project: https://github.com/DebinixTeam/SensorWLED
 * Source: https://github.com/berrak/SensorWLED
 *
 * The MIT license.
 *
 */
#ifndef SensorWLED_H_
#define SensorWLED_H_

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
#else
#include <EEPROM.h>
#endif

/** Microcontroller EEPROM memory locations */
#define  EEPROM_START     0x00   ///< EEPROM instances start addresses
#define  MAXINSTANCES     10     ///< Max number of instantiated EEPROM areas

#define  EEPROM_IDSTART    0xAF  ///< Single EEPROM area: Id and Version
#define  EEPROM_CRC32START 0xFF  ///< Single EEPROM area: (uint16_t)CRC32 sums

/** Memebers in the Version struct */
const uint16_t EEPROM_ID = 0xA5;           ///< EEPROM id marker (never touch)
const uint16_t PROGRAM_VERSION_MAJOR = 0;  ///< Semantic versioning (m.m.p)
const uint16_t PROGRAM_VERSION_MINOR = 1;  ///< Semantic versioning (m.m.p)
const uint16_t PROGRAM_VERSION_PATCH = 0;  ///< Semantic versioning (m.m.p)


/** Sets the microcontroller ADC resolution in bits */
typedef enum : uint16_t {
	bits10 = 1023,			///< ADC max resolution is 10 bits
	bits12 = 4095,			///< ADC max resolution is 12 bits
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
    @brief  Holds CRC32 (casted to 16 bit) sums for structs.
*/
//-----------------------------------------------------------------------------
typedef struct {
    uint16_t crc_version ;       ///< (uint16_t)CRC32 value for Version
    uint16_t crc_user_data;      ///< (uint16_t)CRC32 value for AllUserData
    uint16_t eeprom_write_cnt;   ///< Count writes to EEPROM in begin()
} CRC32Type_t;


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
    @brief  Conglomerat of all data for EEPROM store/retrieve.
*/
//-----------------------------------------------------------------------------
typedef struct {
    uint16_t analog_pin;
    AdcResolutionType_e bits_resolution_adc;
    VoltageVccType_e mv_maxvoltage_adc;
    uint16_t ms_poll_time;
    uint16_t ms_hold_time;
    DecayModelType_e decay_model;
    float decay_rate;
} DataEEPROMType_t;


//-----------------------------------------------------------------------------
/*!
    @brief  Track average and peak DC readings.

    		The analog microcontroller reading from analog input is
            divided and methods returns these as read, while the other methods
            holds the peak-value while decaying with user set parameters.
*/
//-----------------------------------------------------------------------------
class SensorWLED {

public:

    // Constructor: Set up instance 'eeprom_offset' address array
    SensorWLED(void);

    // Destructor: Restore pinMode to default
	~SensorWLED(void);

    // Need +one since we index from 1, and not 0
    inline static uint16_t eeprom_offset[MAXINSTANCES+1];

    // Common information for all instances
    inline static VersionType_t Version ;


    // Setup EEPROM area such that instance get its own EEPROM area
	void setInstanceEEPROMStartAddress(void);

	// Saves user configuration to EEPROM
    bool storeEEPROM(DataEEPROMType_t NewDataStruct);

    // Read and load user configuration from EEPROM
    DataEEPROMType_t retrieveEEPROM(void);

    // Match the capability of the microcontroller ADC with
    // library internals and set user requested time settings.
    void begin(DataEEPROMType_t const &rUserParams);


    // Call continously (in the loop()) for updated ADC values.
	bool updateAnalogRead(void);


    // Map the input analog value to a value optimized
    // for ADC resolution and max supply voltage.
	double getMappedValue(void);
	double getMappedPeakValue(void);




    // Writes EEPROM magic Id to disk, and program version
    bool writeVersionEEPROM(void);


    // Read EEPROM Id & program version. Returns data and update Program struct.
    VersionType_t readVersionEEPROM(void);


    CRC32Type_t getCRC32EEPROM(void);

    uint16_t getSetupWritesEEPROM(void);


	// FIXME: calibration factor for actual WLED sensor board.

private:

	void setAnalogPin(uint16_t a_pin, uint16_t mode = INPUT);
    uint32_t applyDecay(uint32_t peak_value);
    bool putCRC32EEPROM(void);

    // CRC32 utilities
    static void generateTableCRC32(uint32_t(&table)[256]);
    static uint32_t updateCRC32(uint32_t (&table)[256], uint32_t initial,
                                             const void* buf, size_t len);

    DataEEPROMType_t AllUserData;      ///< All conglomerate data structs
    CRC32Type_t StructCRC32;          ///< (uint16_t)CRC32 sums of structs

	uint32_t previous_in_millis_tm;    ///< Holds previous ADC update time

	uint16_t raw_input_value;          ///< ADC raw input at bits capability
	uint16_t mapped_input_value;       ///< ADC values mapped to VCC range

	uint16_t pk_raw_input_value;       ///< ADC peak input at bits capability
	uint16_t pk_mapped_input_value;    ///< ADC peak mapped to VCC range

	uint16_t index_eeprom_address;     ///<  Holds index into array for instance

};
/* class SensorWLED */

#endif /* SensorWLED_H_ */
