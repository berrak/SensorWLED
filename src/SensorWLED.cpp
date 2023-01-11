/*!
 * @file SensorWLED.cpp
 *
 * @mainpage Provides methods to retrieve instant and peak values from the ADC input.
 *
 * @see [WLED current sensor hardware](https://github.com/berrak/WLED-DC-Sensor-Board)
 * @section intro_sec Introduction
 *
 * The library SensorWLED splits the input from a varying analog signal from the ADC 
 * into components, i.e., provides the capability of a sample-and-hold circuit.
 *
 * @section dependencies Dependencies
 *
 * This library depends on Arduino library EEPROM to emulate storage in flash.
 *
 * @section license License
 *
 * The MIT license.
 *
 */
#ifdef ARDUINO
// Required includes for Arduino libraries always go first.
// The class implementation needs these includes.
#include <Arduino.h>
#endif

// Secondly, include required declarations for this class interface (only).
#include "SensorWLED.h"

//-----------------------------------------------------------------------------
/*!
 @brief  Sets up the static 'eeprom_offset'-array which hold each instance
         start address for configuration location in EEPROM.
 @param analog_pin
 ADC analog input pin
 @param mv_offset
 ADC zero offset compensation (mV)
 @param slope
 Adjust deviation of read ADC value
 @param samples
 Number of samples for smoothing ADC values
 */
//-----------------------------------------------------------------------------
SensorWLED::SensorWLED(uint16_t analog_pin, float mv_offset, float slope, uint16_t samples ){

    float tmp_slope = 1.0;
    float tmp_mv_offset = 0.0;

    previous_poll_millis_tm = 0;
    previous_hold_millis_tm = 0;

    raw_input_value = 0;
    mapped_input_value = 0;
    pk_raw_input_value = 0; 
    pk_mapped_input_value = 0;

    cal_crc32 = 0;
    dyn_crc32 = 0;

    DynamicParams = {};

    if(slope > 0) {
        tmp_slope = slope;
    }

    if(mv_offset >= 0) {
        tmp_mv_offset = mv_offset;
    }

    CalibrationData = {analog_pin, samples, US_ADC_CONVERSION_TIME, tmp_mv_offset, tmp_slope};

}

//-----------------------------------------------------------------------------
/*!
 @brief  Deallocate SensorWLED object, set data pin to INPUT.
 */
//-----------------------------------------------------------------------------
SensorWLED::~SensorWLED(void) {
    pinMode(CalibrationData.analog_pin, INPUT);
}


//-----------------------------------------------------------------------------
/*!
 @brief  Load and initilize the user preferences, and EEPROM CRC32 sums.

 @param  & rUserParams
         Takes a D'ataEEPROMType_t' structure and loads program memory.
 */


//-----------------------------------------------------------------------------
/*!
 @brief  Applies user parameters.

 @param  UserDynamicParams
         Takes a DynamicDataType_t' structure and loads it into memory, and 
         also writes the data to emulated EEPROM area for this instance.
 */
//-----------------------------------------------------------------------------
void SensorWLED::begin(DynamicDataType_t const &UserDynamicParams){

float tmp_decay_rate = 1.0;

    DynamicParams.bits_resolution_adc = UserDynamicParams.bits_resolution_adc;
    DynamicParams.mv_maxvoltage_adc = UserDynamicParams.mv_maxvoltage_adc;
    DynamicParams.ms_poll_time = UserDynamicParams.ms_poll_time;
    DynamicParams.ms_hold_time = UserDynamicParams.ms_hold_time;
    DynamicParams.decay_model = UserDynamicParams.decay_model;

    if (UserDynamicParams.decay_rate > 0) {
        tmp_decay_rate = UserDynamicParams.decay_rate;
    }
    DynamicParams.decay_rate = tmp_decay_rate;

    setAnalogPin(CalibrationData.analog_pin, INPUT); // analog input to ADC

    // Common for all instances - changes only with revison changes
    VersionType_t TmpVersion = readVersionEEPROM();
    if (eeprom_version_written_flag == false) {
        writeVersionEEPROM();
        eeprom_version_written_flag = true;
    } else if (TmpVersion.magic_id != EEPROM_ID || TmpVersion.major_version != VERSION_MAJOR || 
        TmpVersion.minor_version != VERSION_MINOR || TmpVersion.patch_version != VERSION_PATCH) {
        writeVersionEEPROM();
    }
    
    // CRC32 checks minimize flash writes (i.e. emulated EEPROM).
    uint32_t table[CRC32_TABLE_SIZE];
    generateTableCRC32(table);


    // CRC32 for 'CalibrationData'
    char *ptrcal = (char *) &CalibrationData;
    uint16_t slen1 = sizeof(CalibrationData);
    uint32_t crc1 = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen1; cnt++) {
        crc1 = updateCRC32(table, crc1, ptrcal, 1);
        ptrcal++;
    }
    cal_crc32 = crc1;

    // CRC32 for 'DynamicParams'
    char *ptrdyn = (char *) &DynamicParams;
    uint16_t slen2 = sizeof(DynamicParams);
    uint32_t crc2 = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen2; cnt++) {
        crc2 = updateCRC32(table, crc2, ptrdyn, 1);
        ptrdyn++;
    }
    dyn_crc32 = crc2;


    // Each instance (ADC channel) has it own EEPROM area for calibration data
    // Each instance (ADC channel) has it own EEPROM area for static/dynamic data
    instance_counter++;
    writeCalibrationEEPROM(instance_counter, cal_crc32);
    writeDynamicEEPROM(instance_counter, dyn_crc32);
}

//-----------------------------------------------------------------------------
/*!
 @brief  Call continously (in loop()) for updated values.
 @return Returns true when a new value is available.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::updateAnalogRead(void) {

    // Check to see if it's time to read from the analog input
    uint32_t current_millis = millis();

    //
    // Decay the peak value, depending on the hold time (not the poll time)
    //
    if (current_millis - previous_hold_millis_tm >= DynamicParams.ms_hold_time) {
        pk_raw_input_value = applyDecay(pk_raw_input_value);
        // Remember the time
        previous_hold_millis_tm = current_millis; 

    }

    //
    // Get a new input value (poll time)
    //
    if (current_millis - previous_poll_millis_tm >= DynamicParams.ms_poll_time) {
        // Remember the time
        previous_poll_millis_tm = current_millis;           

        // The first ADC reading
        raw_input_value = analogRead(CalibrationData.analog_pin);  

        // Map raw input value to resolution range (without smoothing)
        if (CalibrationData.sample_count == 0) {
        mapped_input_value = (double) map(raw_input_value, 0,
                DynamicParams.bits_resolution_adc, 0, DynamicParams.mv_maxvoltage_adc);
        
        // or apply smooting with a fixed rate not to jeopardize the ADC conversion cycle          
        } else {

            // Initial short delay before additional ADC inpt readings
            delayMicroseconds(CalibrationData.sample_period);

            for (int cnt=1; cnt < CalibrationData.sample_count; cnt++) {
                    raw_input_value += analogRead(CalibrationData.analog_pin);
                    delayMicroseconds(CalibrationData.sample_period);
            }
            
            mapped_input_value = (double) map(raw_input_value/CalibrationData.sample_count, 0,
                               DynamicParams.bits_resolution_adc, 0, DynamicParams.mv_maxvoltage_adc);
        }
        
        // Apply slope calibration compensation -----
        mapped_input_value *= CalibrationData.cal_slope ;

        // Apply zero offset compensation -----
        if (CalibrationData.cal_zero_offset <= mapped_input_value) {
        	mapped_input_value -= CalibrationData.cal_zero_offset;
        } else {
        	mapped_input_value = 0;
        }

        // Updates our peak values, if greater than last time
        if (raw_input_value >= pk_raw_input_value) {
            pk_raw_input_value = raw_input_value;
            pk_mapped_input_value = mapped_input_value;
        }
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Map the input analog value to a value optimized for actual
 capability of the controllers ADC and suppl voltage.

 @return The mapped instantanous analog value.

 */
//-----------------------------------------------------------------------------
double SensorWLED::getMappedValue(void) {
    return mapped_input_value;

}

//-----------------------------------------------------------------------------
/*!
 @brief  Map the input analog value to a value optimized for actual
 capability of the controllers ADc and suppl voltage.

 @return The mapped peak analog value.

 */
//-----------------------------------------------------------------------------
double SensorWLED::getMappedPeakValue(void) {
    return pk_mapped_input_value;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Applies the decay model to peak value.

 @param  peak_value
 Value to decay with set model, rate and times.
 @return The reduced, decayed input peak value.

 */
//-----------------------------------------------------------------------------
uint32_t SensorWLED::applyDecay(uint32_t peak_value) {

    // FIXME: sanity checks, in particular linear must be < 1!

    if (DynamicParams.decay_model == linear_decay) {
        return (peak_value * DynamicParams.decay_rate);
    }
    else if (DynamicParams.decay_model == exponential_decay) {
        return (peak_value * exp(-DynamicParams.decay_rate));
    }

    return 0;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Saves static 'Version' struct to EEPROM.
         Writes to fixed EEPROM address, common for all instances.

 @return true if EEPROM written.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::writeVersionEEPROM(void)
{
    bool is_written = false;

// The check to 'require' a new write to Version EEPROM is done in the call
#ifdef ARDUINO
        EEPROM.begin(sizeof(Version));
        EEPROM.put(EEPROM_IDSTART, Version);
        EEPROM.commit();
        EEPROM.end();
        is_written = true;

#else
    PLOG_INFO << "EEPROM Version (common) WRITE at: " << EEPROM_IDSTART;
    is_written = true;
#endif

    return is_written;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Memory load of EEPROM Id and Pgm version, and returns the information.
         This use a fixed EEPROM address, common for all instances.

 @return The Version struct data.         

 */
//-----------------------------------------------------------------------------
VersionType_t SensorWLED::readVersionEEPROM(void) {

#ifdef ARDUINO
    EEPROM.begin(sizeof(Version));
    EEPROM.get(EEPROM_IDSTART, Version);
    EEPROM.end();
#else
    PLOG_INFO << "EEPROM Version (common) READ at: " << EEPROM_IDSTART;
#endif

return Version;
}

//-----------------------------------------------------------------------------
/*!
@brief  Saves static 'CalibrationData' struct to EEPROM.

@param instance  
       The actual instance (ADC channel) for which the data belongs to.      

@param crc32  
       The calculated CRC32 sum for the struct to be written. 

@return Boolean (true) value when data is written.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::writeCalibrationEEPROM(uint16_t instance, uint32_t crc32)
{
    bool is_written = false;

    // CRC32 checks minimize flash writes (i.e. emulated EEPROM).
    uint32_t table[CRC32_TABLE_SIZE];
    generateTableCRC32(table);
    CalibrationDataType_t StoredCalibrationData = readCalibrationEEPROM(instance);
    
    // CRC32 for existing 'CalibrationData'
    char *ptrcal = (char *) &StoredCalibrationData;
    uint16_t slen1 = sizeof(StoredCalibrationData);
    uint32_t oldcrc = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen1; cnt++) {
        oldcrc = updateCRC32(table, oldcrc, ptrcal, 1);
        ptrcal++;
    }
    
    if(oldcrc != crc32) {

        #ifdef ARDUINO                                                       
                EEPROM.begin(sizeof(CalibrationData));
                EEPROM.put(eeprom_area[instance], CalibrationData);
                EEPROM.commit();
                EEPROM.end();
                is_written = true;
        #else
            PLOG_INFO << "EEPROM CalibrationData WRITE at: " << eeprom_area[instance];
            is_written = true;
        #endif
    }

    return is_written;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Memory load of EEPROM 'CalibrationData', and returns the struct.

 @param instance
 The specified instance for the EEPROM read

 @return The CalibrationData struct data.   
 */
//-----------------------------------------------------------------------------
CalibrationDataType_t SensorWLED::readCalibrationEEPROM(uint16_t instance) {

#ifdef ARDUINO
    EEPROM.begin(sizeof(CalibrationData));
    EEPROM.get(eeprom_area[instance], CalibrationData);
    EEPROM.end();
#else
    PLOG_INFO << "EEPROM CalibrationDataREAD at: " << eeprom_area[instance];
#endif

return CalibrationData;
}

//-----------------------------------------------------------------------------
/*!
@brief  Saves static 'DynamicData' struct to EEPROM.
        
@param instance  
       The actual instance (ADC channel) for which the data belongs to.      

@param crc32  
       The calculated CRC32 sum for the struct to be written. 

@return Boolean (true) value when data is written.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::writeDynamicEEPROM(uint16_t instance, uint32_t crc32)
{
    bool is_written = false;

    // CRC32 checks minimize flash writes (i.e. emulated EEPROM).
    uint32_t table[CRC32_TABLE_SIZE];
    generateTableCRC32(table);
    DynamicDataType_t StoredDynamicParams = readDynamicEEPROM(instance);
    
    // CRC32 for existing 'DynamicParams'
    char *ptrcal = (char *) &StoredDynamicParams;
    uint16_t slen1 = sizeof(StoredDynamicParams);
    uint32_t oldcrc = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen1; cnt++) {
        oldcrc = updateCRC32(table, oldcrc, ptrcal, 1);
        ptrcal++;
    }
    
    if(oldcrc != crc32) {    

        #ifdef ARDUINO
                EEPROM.begin(sizeof(DynamicParams));
                EEPROM.put(eeprom_area[instance]+sizeof(CalibrationData), DynamicParams);
                EEPROM.commit();
                EEPROM.end();
                is_written = true;
        #else
            PLOG_INFO << "EEPROM CalibrationData WRITE at: " << eeprom_area[instance]+sizeof(CalibrationData);
            is_written = true;
        #endif

    }

    return is_written;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Memory load of EEPROM 'DynamicData', and returns the struct.
 @param instance
 The specified instance for the EEPROM read
 @return The DynamicData struct data.   
 */
//-----------------------------------------------------------------------------
DynamicDataType_t SensorWLED::readDynamicEEPROM(uint16_t instance) {

#ifdef ARDUINO
    EEPROM.begin(sizeof(DynamicParams));
    EEPROM.get(eeprom_area[instance]+sizeof(CalibrationData), DynamicParams);
    EEPROM.end();
#else
    PLOG_INFO << "EEPROM CalibrationData READ at: " << eeprom_area[instance]+sizeof(CalibrationData);
#endif

return DynamicParams;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Sets the analog pin for ADC input on the microcontroller.

 @param  a_pin
         Pin number for analog readings on the microcontroller.
 @param  mode
         Arduino pin mode for analog input.
 */
//-----------------------------------------------------------------------------
void SensorWLED::setAnalogPin(uint16_t a_pin, uint16_t mode) {
    pinMode(a_pin, mode);
}

//-----------------------------------------------------------------------------
/*!
@brief Gets the total number of created instances.

@return Instances created.
 */
//-----------------------------------------------------------------------------
uint16_t SensorWLED::getInstanceNumber(void) {
    return instance_counter;
}

//-----------------------------------------------------------------------------
/*!
 @brief  Generate 32 bit CRC table.

 @param  table
         The table that will be populated for CRC32 calculations.
 */
//-----------------------------------------------------------------------------
void SensorWLED::generateTableCRC32(uint32_t(&table)[CRC32_TABLE_SIZE]) {

    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < CRC32_TABLE_SIZE; i++)
    {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++)
        {
            if (c & 1) {
                c = polynomial ^ (c >> 1);
            }
            else {
                c >>= 1;
            }
        }
        table[i] = c;
    }
}

//-----------------------------------------------------------------------------
/*!
 @brief  Calculate 32 bit CRC.

 @param  table
         CRC32 table required for calculations.
 @param  initial
         Initial CRC32 value. 0 if first update, can be called repetingly.
 @param  buf
         Pointer to start address of memory.
 @param  len
         Size of contingent memory area to apply the CRC32 algorithm.
 @return Calculated CRC32 value.
 */
//-----------------------------------------------------------------------------
uint32_t SensorWLED::updateCRC32(uint32_t (&table)[CRC32_TABLE_SIZE], uint32_t initial,
                                             const void* buf, size_t len) {

    uint32_t c = initial ^ 0xFFFFFFFF;
    const uint8_t* u = static_cast<const uint8_t*>(buf);
    for (size_t i = 0; i < len; ++i)
    {
        c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}

//-----------------------------------------------------------------------------
/*!
@brief  Calculates the CRC32 sum for 'CalibrationData'.

@param CalibrationData  
       The actual instance 'CalibrationData' struct.      

@return The calculated CRC32 sum for the struct 'CalibrationData'.
 */
//-----------------------------------------------------------------------------
uint32_t SensorWLED::calculateCalibrationDataCRC32(CalibrationDataType_t CalibrationData)
{
    uint32_t crc32sum = 0;

    uint32_t table[CRC32_TABLE_SIZE];
    generateTableCRC32(table);
    
    // calculate CRC32 for 'CalibrationData'
    char *ptrcal = (char *) &CalibrationData;
    uint16_t slen1 = sizeof(CalibrationData);
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen1; cnt++) {
        crc32sum = updateCRC32(table, crc32sum, ptrcal, 1);
        ptrcal++;
    }

    return crc32sum;
}

//-----------------------------------------------------------------------------
/*!
@brief  Calculates the CRC32 sum for 'DynamicParams'.

@param DynamicParams  
       The actual instance 'DynamicParams' struct.      

@return The calculated CRC32 sum for the struct 'DynamicParams'.
 */
//-----------------------------------------------------------------------------
uint32_t SensorWLED::calculateDynamicParamsCRC32(DynamicDataType_t DynamicParams)
{
    uint32_t crc32sum = 0;

    uint32_t table[CRC32_TABLE_SIZE];
    generateTableCRC32(table);
    
    // calculate CRC32 for 'DynamicParams'
    char *ptrcal = (char *) &DynamicParams;
    uint16_t slen1 = sizeof(DynamicParams);
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen1; cnt++) {
        crc32sum = updateCRC32(table, crc32sum, ptrcal, 1);
        ptrcal++;
    }

    return crc32sum;
}

// EOF
