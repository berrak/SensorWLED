/*!
 * @file SensorWLED.cpp
 *
 * @mainpage Splits the input varying analog signal DC values into components.
 *
 * @see [WLED current sensor hardware]((https://github.com/DebinixTeam/SensorWLED))
 * @section intro_sec Introduction
 *
 * The analog microcontroller reading from analog input is
 * divided, in library methods which returns these as read, while other
 * methods holds the peak-value while decaying with user set decay parameters.
 *
 * @section dependencies Dependencies
 *
 * This library depends on Arduino library EEPROM.
 *
 * @section license License
 *
 * The MIT license.
 *
 */
#ifdef ARDUINO
// Required includes for Arduino libraries always go first.
// The class implementation needs these includes.
#include "Arduino.h"
#endif
// Secondly, include required declarations for this class interface.
#include "SensorWLED.h"



//-----------------------------------------------------------------------------
/*!
 @brief  Sets up the static 'eeprom_offset'-array which hold each instance
         start address for configuration location in EEPROM.

 */
//-----------------------------------------------------------------------------
SensorWLED::SensorWLED(void){

    // Private structs. Version is static, not initialized here, but in begin().
    AllUserData = {};
    StructCRC32 = {};


    // private variables
    previous_in_millis_tm = 0;
    raw_input_value = 0;
    mapped_input_value = 0;
    pk_raw_input_value = 0;
    pk_mapped_input_value = 0;

    // updates the instance array with sizeof(AllUserData), maybe in begin?
    index_eeprom_address = 0;
    setInstanceEEPROMStartAddress();
}


//-----------------------------------------------------------------------------
/*!
 @brief  Deallocate SensorWLED object, set data pin to INPUT.
 */
//-----------------------------------------------------------------------------
SensorWLED::~SensorWLED(void) {
    pinMode(AllUserData.analog_pin, INPUT);

    // FIXME: pinMode(AllUserData.control_pin, INPUT);
}


//-----------------------------------------------------------------------------
/*!
 @brief  Load and initilize the user preferences, and EEPROM CRC32 sums.

 @param  & rUserParams
         Takes a D'ataEEPROMType_t' structure and loads program memory.
 */
//-----------------------------------------------------------------------------
void SensorWLED::begin(DataEEPROMType_t const &rUserParams){

    // VALIDATE ALL EEPROM DATA
    uint32_t table[256];
    generateTableCRC32(table);// CRC32 prevents EEPROM writes unless data change
    uint16_t eeprom_write_counter = 0;

    // Define 'Version-code'
    Version.magic_id = EEPROM_ID;
    Version.major_version = PROGRAM_VERSION_MAJOR;
    Version.minor_version = PROGRAM_VERSION_MINOR;
    Version.patch_version = PROGRAM_VERSION_PATCH;


    // 1. Calculate CRC32 for 'Version-code' struct. Note that a version change
    //    does not nessesary invalidate stored EEPROM user data.
    //    Here 'Version-code' (not EEPROM) is always the canonical truth.
    char *ptrver = (char *) &Version;
    uint16_t slen1 = sizeof(Version);
    uint32_t CRC1 = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen1; cnt++) {
        CRC1 = updateCRC32(table, CRC1, ptrver, 1);
        ptrver++;
    }
    uint16_t code_crc_version = (uint16_t) CRC1;       // done 'Version-code'


    // 2. Is 'Version-EEPROM' struct, identical with 'Version-code'?
    VersionType_t ReadSavedVersion = readVersionEEPROM();

    // Calculate CRC32 for 'Version-EEPROM' struct
    char *ptree = (char *) &ReadSavedVersion;
    uint16_t slen2 = sizeof(ReadSavedVersion);
    uint32_t CRC2 = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen2; cnt++) {
        CRC2 = updateCRC32(table, CRC2, ptree, 1);
        ptree++;
    }
    uint16_t eeprom_crc_version = (uint16_t) CRC2;    // done 'Version-EEPROM'
#ifndef ARDUINO
    PLOG_INFO << "Calculated crc for built-in Version: " << code_crc_version;
    PLOG_INFO << "Calculated crc from EEPROM-Version: " << eeprom_crc_version;
#endif

    // If these out of sync update a) Version-EEPROM' and b) 'CRC-VersionEEPROM'
    StructCRC32.crc_version = code_crc_version; // update runtime struct.

    if(code_crc_version != eeprom_crc_version){
        writeVersionEEPROM();
        eeprom_write_counter++;
        putCRC32EEPROM();
        eeprom_write_counter++;
    }

    // 3. If 'UserData-EEPROM' == CRC-UserDataEEPROM' is true -> data valid.
    //    Reason is that data updates can be changed by Wifi, at runtime,
    //    Use 'UserData-EEPROM' and ignore 'UserData-code' (initial data)

    DataEEPROMType_t ReadSavedUserData = retrieveEEPROM();

    // Calculate CRC32 for 'UserData-EEPROM' struct
    char *ptrud = (char *) &ReadSavedUserData;
    uint16_t slen3 = sizeof(ReadSavedUserData);
    uint32_t CRC3 = 0;
    // Step bytewise, piece-meal through the members in the structure
    for (uint16_t cnt = 0; cnt < slen3; cnt++) {
        CRC3 = updateCRC32(table, CRC3, ptrud, 1);
        ptrud++;
    }
    uint16_t eeprom_crc_userdata = (uint16_t) CRC3; // done 'UserData-EEPROM'
    CRC32Type_t ReadCRC32 = getCRC32EEPROM();

#ifndef ARDUINO
    PLOG_INFO << "Get CRC-EEPROM for UserData-EEPROM: "
                                                    << ReadCRC32.crc_user_data;
    PLOG_INFO << "UserData-EEPROM: calculated crc value is : "
                                                    << eeprom_crc_userdata;
#endif

    if (eeprom_crc_userdata == ReadCRC32.crc_user_data) {
        AllUserData = ReadSavedUserData;
    }
    // 4. else if (3) is false: Save 'UserData-setup' -> 'UserData-EEPROM',
    //    and calculate new setup():CRC32, and update 'CRC-UserDataEEPROM'.
    else
    {
        AllUserData.analog_pin = rUserParams.analog_pin;
        AllUserData.bits_resolution_adc = rUserParams.bits_resolution_adc;
        AllUserData.mv_maxvoltage_adc = rUserParams.mv_maxvoltage_adc;

        AllUserData.ms_poll_time = rUserParams.ms_poll_time;
        AllUserData.ms_hold_time = rUserParams.ms_hold_time;

        AllUserData.decay_model = rUserParams.decay_model;
        AllUserData.decay_rate = rUserParams.decay_rate;

        // Save user data to EEPROM
        storeEEPROM(AllUserData);
        eeprom_write_counter++;

        // Calculate CRC32 for user input data
        char *ptrnew = (char *) &AllUserData;
            uint16_t slen4 = sizeof(AllUserData);
            uint32_t CRC4 = 0;
            // Step bytewise, piece-meal through the members in the structure
            for (uint16_t cnt = 0; cnt < slen4; cnt++) {
                CRC4 = updateCRC32(table, CRC4, ptrnew, 1);
                ptrnew++;
            }
        uint16_t new_crc_userdata = (uint16_t) CRC4; // done 'UserData-setup'

        // Update 'CRC-UserDataEEPROM' for next time.
    #ifndef ARDUINO
    PLOG_INFO << "Put CRC32 value, for new UserData to CRC32-EEPROM: "
                                                         << new_crc_userdata;
    #endif
        StructCRC32.crc_user_data = new_crc_userdata;
        putCRC32EEPROM();
        eeprom_write_counter++;

    }

    // Re-read CRC32 sums after all updates
    #ifndef ARDUINO
    PLOG_INFO << "Get latest updated CRC32-EEPROM sums for use in program";
    #endif
    StructCRC32 = getCRC32EEPROM();

    // EEPROM writes during the setup() process (not saved in CRC32-EEPROM)
    StructCRC32.eeprom_write_cnt = eeprom_write_counter;

    setAnalogPin(rUserParams.analog_pin, INPUT); // analog input to ADC

}


//-----------------------------------------------------------------------------
/*!
 @brief  Set the EEPROM start address for an instance.
 */
//-----------------------------------------------------------------------------
void SensorWLED::setInstanceEEPROMStartAddress(void){
    static int idx = 1;
    DataEEPROMType_t UserData = {};
    // add sizeof Data + previous instance written data
    eeprom_offset[idx] = sizeof(UserData)+eeprom_offset[idx-1];

    // sets the index for this instance
    index_eeprom_address = idx;
    // next instance will use this
    idx++;
}




//-----------------------------------------------------------------------------
/*!
 @brief  Takes user struct, i.e. the setup() parameters and, for each
         instance, write data to a separate EEPROM memory area.

 @param 'DataEEPROMType_t' structure for writing to EEPROM.
 @return true if EEPROM written.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::storeEEPROM(DataEEPROMType_t NewDataStruct)
{
    uint16_t index = eeprom_offset[index_eeprom_address];
    bool is_stored = false;

    AllUserData = NewDataStruct;

    // Write data to EEPROM
    #ifdef ARDUINO
        EEPROM.begin(sizeof(NewDataStruct));
        EEPROM.put(index-sizeof(NewDataStruct), NewDataStruct);
        EEPROM.commit();
        EEPROM.end();
        is_stored = true;
    #else
        PLOG_INFO << "Array index i.e. EEPROM UserData WRITE at: "
                                             << index-sizeof(NewDataStruct);
        is_stored = true;
    #endif

    return is_stored;
}


//-----------------------------------------------------------------------------
/*!
 @brief  Loads user preferences from EEPROM.

 @return DataEEPROMType_t
         The data structure retrieved from EEPROM.
 */
//-----------------------------------------------------------------------------
DataEEPROMType_t SensorWLED::retrieveEEPROM(void) {
    DataEEPROMType_t InstanceDataEEPROM = {};
    int index = eeprom_offset[index_eeprom_address];

    #ifdef ARDUINO
        EEPROM.begin(sizeof(InstanceDataEEPROM));
        EEPROM.get(index-sizeof(InstanceDataEEPROM), InstanceDataEEPROM);
        EEPROM.end();
    #else
        PLOG_INFO << "Array index i.e. EEPROM UserData READ at: "
                                   << index-sizeof(InstanceDataEEPROM);
    #endif

    // Update internal runtime struct with retrived EEPROM data
    AllUserData = InstanceDataEEPROM;
    return AllUserData;
}



//-----------------------------------------------------------------------------
/*!
 @brief  Call continously (in loop()) for updated values.
 @return Returns true when a new value is available.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::updateAnalogRead(void) {

    // check to see if it's time to read from the analog input
    uint32_t current_millis = millis();

    // decay faster faster update time, i.e. the hold tm
    if (current_millis - previous_in_millis_tm >= AllUserData.ms_hold_time) {
        pk_raw_input_value = applyDecay(pk_raw_input_value);
    }

    // FIXME: Checks for resonable values

    if (current_millis - previous_in_millis_tm >= AllUserData.ms_poll_time) {
        previous_in_millis_tm = current_millis;           // Remember the time
        raw_input_value = analogRead(AllUserData.analog_pin);  // Read input
        mapped_input_value = map(raw_input_value, 0,
                AllUserData.bits_resolution_adc, 0, AllUserData.mv_maxvoltage_adc);

        // updates our peak values, if greater than last time
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
 capability of the controllers ADc and suppl voltage.

 @return The mapped instantanous analog value.

 */
//-----------------------------------------------------------------------------
double SensorWLED::getMappedValue(void) {
    return (double) mapped_input_value;
}


//-----------------------------------------------------------------------------
/*!
 @brief  Map the input analog value to a value optimized for actual
 capability of the controllers ADc and suppl voltage.

 @return The mapped peak analog value.

 */
//-----------------------------------------------------------------------------
double SensorWLED::getMappedPeakValue(void) {
    return (double) pk_mapped_input_value;
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

    if (AllUserData.decay_model == linear_decay) {
        return (peak_value * AllUserData.decay_rate);
    }
    else if (AllUserData.decay_model == exponential_decay) {
        return (peak_value * exp(-AllUserData.decay_rate));
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
 @brief  Read CRC32 struct from EEPROM, and returns the information.
         This use a fixed EEPROM address, common for all instances.
         FIXME: Should realy be unique for each instance.
         - Maybe put the CRC32 in the written structures itself.
         - or, if index will work, dedicate continous multiinstances CRC-areas.
         - Worth a Arduino Library if solution could be worked out.

 @return CRC32 structure
 */
//-----------------------------------------------------------------------------
CRC32Type_t SensorWLED::getCRC32EEPROM(void) {

#ifdef ARDUINO
    EEPROM.begin(sizeof(StructCRC32));
    EEPROM.get(EEPROM_CRC32START, StructCRC32);
    EEPROM.end();
#else
    PLOG_INFO << "EEPROM CRC32 READ at address: " << EEPROM_CRC32START;
#endif

return StructCRC32;
}


//-----------------------------------------------------------------------------
/*!
 @brief  Put current content of instance member 'Struct32CRC' to EEPROM.
         Every instance writes to a common EEPROM area.

         FIXME: Should realy be unique for each instance.
         - Maybe put the CRC32 in the written structures itself.
         - or, if index will work, dedicate continous multiinstances CRC-areas.
         - Worth a Arduino Library if solution could be worked out.

 @return true if EEPROM written.
 */
//-----------------------------------------------------------------------------
bool SensorWLED::putCRC32EEPROM(void)
{
    bool is_put = false;

    // Update CRC32 sums for other saved structs
    #ifdef ARDUINO
        EEPROM.begin(sizeof(StructCRC32));
        EEPROM.put(EEPROM_CRC32START, StructCRC32);
        EEPROM.commit();
        EEPROM.end();
        is_put = true;
    #else
        PLOG_INFO << "EEPROM StructCRC32 WRITE (common) at: " << EEPROM_CRC32START;
        is_put = true;
    #endif

    return is_put;
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
 @brief  Generate 32 bit CRC table.

 @param  table[256]
         The table that will be populated for CRC32 calculations.
 */
//-----------------------------------------------------------------------------
void SensorWLED::generateTableCRC32(uint32_t(&table)[256]) {

    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++)
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

 @param  table[256]
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
uint32_t SensorWLED::updateCRC32(uint32_t (&table)[256], uint32_t initial,
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
 @brief  The setup() number of EEPROM writes.

 @return Number of write accesses.
 */
//-----------------------------------------------------------------------------
uint16_t SensorWLED::getSetupWritesEEPROM(void) {
    return StructCRC32.eeprom_write_cnt;
}



// EOF
