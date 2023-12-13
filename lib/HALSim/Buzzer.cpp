/* MIT License
 *
 * Copyright (c) 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Buzzer realization
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Buzzer.h"

#include <unistd.h>

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

const char* Buzzer::WAV_FILE_440_HZ = "/sounds/440hz.wav";
const char* Buzzer::WAV_FILE_4_KHZ  = "/sounds/4khz.wav";
const char* Buzzer::WAV_FILE_10_KHZ = "/sounds/10khz.wav";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Buzzer::Buzzer(webots::Speaker* speaker) : IBuzzer(), m_speaker(speaker), m_path(), m_pathLen(0)
{
    /* Get current working directory. */
    if (nullptr != getcwd(m_path, sizeof(m_path)))
    {
        m_pathLen = strlen(m_path);
    }
}

void Buzzer::playFrequency(uint16_t freq, uint16_t duration, uint8_t volume)
{
    const char* soundFileToPlay = nullptr;

    (void)duration;

    if (440 == freq)
    {
        soundFileToPlay = WAV_FILE_440_HZ;
    }
    else if (4000 == freq)
    {
        soundFileToPlay = WAV_FILE_4_KHZ;
    }
    else if (10000 == freq)
    {
        soundFileToPlay = WAV_FILE_10_KHZ;
    }
    else
    {
        ;
    }

    if (nullptr != soundFileToPlay)
    {
        size_t wavFileLen = strlen(soundFileToPlay);

        if ((MAX_PATH_SIZE > (m_pathLen + wavFileLen)) && (nullptr != m_speaker))
        {
            char   fullPath[MAX_PATH_SIZE];
            double simVolume = static_cast<double>(volume) / 255.0f;
            double pitch     = 1.0f;
            double balance   = 0.0f;

            strcpy(fullPath, m_path);
            strcpy(&fullPath[m_pathLen], soundFileToPlay);

            m_speaker->playSound(m_speaker, m_speaker, fullPath, simVolume, pitch, balance, false);
        }
    }
}

void Buzzer::playMelody(const char* sequence)
{
    /* TODO */
}

void Buzzer::playMelodyPGM(const char* sequence)
{
    /* TODO */
}

bool Buzzer::isPlaying()
{
    bool isPlaying = false;

    if (nullptr != m_speaker)
    {
        /* Is any sound playing? */
        isPlaying = m_speaker->isSoundPlaying("");
    }

    return isPlaying;
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
