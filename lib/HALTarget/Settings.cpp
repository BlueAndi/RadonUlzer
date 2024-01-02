/* MIT License
 *
 * Copyright (c) 2023 - 2024 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Settings realization
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Settings.h"
#include <avr/eeprom.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

template<typename T>
static T readData(const void* addr);
template<typename T>
static void writeData(const void* addr, T data);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Magic pattern address in EEPROM.
 */
static const void* MAGIC_PATTERN_ADDR = static_cast<const void*>(0x0000);

/**
 * Magic pattern size in byte.
 */
static const size_t MAGIC_PATTERN_SIZE = sizeof(uint32_t);

/**
 * Data version address in EEPROM.
 */
static const void* DATA_VERSION_ADDR = &static_cast<const uint8_t*>(MAGIC_PATTERN_ADDR)[MAGIC_PATTERN_SIZE];

/**
 * Data version size in byte.
 */
static const size_t DATA_VERSION_SIZE = sizeof(uint8_t);

/**
 * Max. speed address in EEPROM.
 */
static const void* MAX_SPEED_ADDR = &static_cast<const uint8_t*>(DATA_VERSION_ADDR)[DATA_VERSION_SIZE];

/**
 * Max. speed size in byte.
 */
static const size_t MAX_SPEED_SIZE = sizeof(int16_t);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Settings::init()
{
    uint32_t magicPattern = readData<uint32_t>(MAGIC_PATTERN_ADDR);
    uint8_t  dataVersion  = readData<uint8_t>(DATA_VERSION_ADDR);

    if ((MAGIC_PATTERN != magicPattern) || (DATA_VERSION != dataVersion))
    {
        /* Write default values. */
        writeData<int16_t>(MAX_SPEED_ADDR, DEFAULT_MAX_SPEED);

        /* Mark data in EEPROM as valid. */
        writeData<uint32_t>(MAGIC_PATTERN_ADDR, MAGIC_PATTERN);
        writeData<uint8_t>(DATA_VERSION_ADDR, DATA_VERSION_SIZE);
    }
}

int16_t Settings::getMaxSpeed() const
{
    return readData<int16_t>(MAX_SPEED_ADDR);
}

void Settings::setMaxSpeed(int16_t maxSpeed)
{
    writeData<int16_t>(MAX_SPEED_ADDR, maxSpeed);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Read data from EEPROM.
 * 
 * @tparam T    Data type
 * 
 * @param[in]   addr    Address in the EEPROM.
 * 
 * @return Data
 */
template<typename T>
static T readData(const void* addr)
{
    T value;

    eeprom_read_block(&value, addr, sizeof(T));

    return value;
}

/**
 * Read data from EEPROM.
 * 
 * @tparam T    Data type
 * 
 * @param[in]   addr    Address in the EEPROM.
 * @param[in]   data    Data to write.
 */
template<typename T>
static void writeData(const void* addr, T data)
{
    eeprom_write_block(addr, &data, sizeof(T));
}
