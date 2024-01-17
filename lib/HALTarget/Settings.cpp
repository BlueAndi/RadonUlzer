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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * The magic pattern which is used to determine whether the EEPROM
 * is initialized or not.
 */
static const uint32_t MAGIC_PATTERN = 0xC0FFEE;

/**
 * Data version is used to detect whether the data in the EEPROM is
 * compatible with the current settings version.
 *
 * Increase the version number by 1 for every change!
 */
static const uint8_t DATA_VERSION = 1U;

/**
 * Max. speed default values in steps/s.
 */
static const int16_t DEFAULT_MAX_SPEED = 0;

/* ---------- Attention! ----------
 * Keep the following order of variables in the EEPROM.
 * Add new values at the tail.
 * Increase the data version (DATA_VERSION) for every change!
 */

/**
 * Magic pattern in EEPROM.
 */
static uint32_t EEMEM gMagicPattern = MAGIC_PATTERN;

/**
 * Data version in EEPROM.
 */
static uint8_t EEMEM gDataVersion = DATA_VERSION;

/**
 * Max. speed in EEPROM.
 */
static int16_t EEMEM gMaxSpeed = DEFAULT_MAX_SPEED;

/* ---------- Tail of EEPROM data. ---------- */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Settings::init()
{
    uint32_t magicPattern = getMagicPattern();
    uint8_t  dataVersion  = getDataVersion();

    if ((MAGIC_PATTERN != magicPattern) || (DATA_VERSION != dataVersion))
    {
        /* Write default values. */
        setMaxSpeed(DEFAULT_MAX_SPEED);

        /* Mark data in EEPROM as valid. */
        setMagicPattern(MAGIC_PATTERN);
        setDataVersion(DATA_VERSION);
    }
}

int16_t Settings::getMaxSpeed() const
{
    void*     vAddr = &gMaxSpeed; /* Avoid reinterpret_cast<>() */
    uint16_t* addr  = static_cast<uint16_t*>(vAddr);

    return static_cast<int16_t>(eeprom_read_word(addr));
}

void Settings::setMaxSpeed(int16_t maxSpeed)
{
    void*     vAddr = &gMaxSpeed; /* Avoid reinterpret_cast<>() */
    uint16_t* addr  = static_cast<uint16_t*>(vAddr);

    eeprom_update_word(addr, static_cast<uint16_t>(maxSpeed));
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint32_t Settings::getMagicPattern() const
{
    return eeprom_read_dword(&gMagicPattern);
}

void Settings::setMagicPattern(uint32_t value) const
{
    eeprom_update_dword(&gMagicPattern, value);
}

uint8_t Settings::getDataVersion() const
{
    return eeprom_read_byte(&gDataVersion);
}

void Settings::setDataVersion(uint8_t value) const
{
    eeprom_update_byte(&gDataVersion, value);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
