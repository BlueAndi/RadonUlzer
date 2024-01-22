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
#include <ArduinoJson.h>
#include <stdio.h>

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

static long getFileSize(FILE* fd);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

const char*   Settings::SETTINGS_FILE_NAME = "settings.json";
const uint8_t Settings::DATA_VERSION       = 1U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Settings::init()
{
    bool isSuccessful = loadSettings();

    if (false == isSuccessful)
    {
        /* Write settings with default values. */
        saveSettings();
    }
}

int16_t Settings::getMaxSpeed() const
{
    return m_maxSpeed;
}

void Settings::setMaxSpeed(int16_t maxSpeed)
{
    m_maxSpeed = maxSpeed;
    saveSettings();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool Settings::loadSettings()
{
    bool  isSuccessful = false;
    FILE* fd           = fopen(SETTINGS_FILE_NAME, "rb");

    if (nullptr != fd)
    {
        long fileSize = getFileSize(fd);
        char buffer[fileSize];

        if (fileSize == fread(buffer, sizeof(char), fileSize, fd))
        {
            const size_t         JSON_DOC_SIZE = 4096U;
            DynamicJsonDocument  jsonDoc(JSON_DOC_SIZE);
            DeserializationError status = deserializeJson(jsonDoc, buffer);

            if (DeserializationError::Ok == status)
            {
                JsonVariantConst jsonVersion = jsonDoc["version"];

                if (false == jsonVersion.isNull())
                {
                    uint8_t version = jsonVersion.as<uint8_t>();

                    if (DATA_VERSION == version)
                    {
                        JsonVariantConst jsonMaxSpeed = jsonDoc["maxSpeed"];

                        if (false == jsonMaxSpeed.isNull())
                        {
                            m_maxSpeed = jsonMaxSpeed.as<int16_t>();

                            isSuccessful = true;
                        }
                    }
                }
            }
        }

        (void)fclose(fd);
    }

    return isSuccessful;
}

void Settings::saveSettings()
{
    const size_t        JSON_DOC_SIZE = 4096U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    size_t              jsonBufferSize;

    /* Don't forget to bump the DATA_VERSION every time you modify
     * the settings!
     */

    jsonDoc["version"]  = DATA_VERSION;
    jsonDoc["maxSpeed"] = m_maxSpeed;

    jsonBufferSize = measureJsonPretty(jsonDoc); /* Size without string termination. */

    {
        char jsonBuffer[jsonBufferSize + 1U];

        if (jsonBufferSize == serializeJsonPretty(jsonDoc, jsonBuffer, jsonBufferSize))
        {
            FILE* fd = fopen(SETTINGS_FILE_NAME, "wb");

            if (nullptr != fd)
            {
                (void)fwrite(jsonBuffer, jsonBufferSize, 1, fd);

                (void)fclose(fd);
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get file size in byte.
 * The file must be opened already.
 *
 * @param[in] fd    File descriptor.
 *
 * @return File size in byte
 */
static long getFileSize(FILE* fd)
{
    long current = ftell(fd); /* Remember current position. */
    long begin;
    long end;

    (void)fseek(fd, 0, SEEK_SET);
    begin = ftell(fd);
    (void)fseek(fd, 0, SEEK_END);
    end = ftell(fd);

    /* Restore position. */
    (void)fseek(fd, current, SEEK_SET);

    return end - begin;
}
