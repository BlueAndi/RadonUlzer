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
 * @brief  Buzzer realization
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Buzzer.h"

#include <Arduino.h>
#include <unistd.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/*                                             n
 * Equal Tempered Scale is given by f  = f  * a
 *                                   n    o
 *
 *  where f  is chosen as A above middle C (A4) at f  = 440 Hz
 *         o                                        o
 *  and a is given by the twelfth root of 2 (~1.059463094359)
 */

/**
 * Note macro
 * x specifies the octave of the note
 * @{
 */
#define NOTE_C(x)       (0 + (x) * 12)
#define NOTE_C_SHARP(x) (1 + (x) * 12)
#define NOTE_D_FLAT(x)  (1 + (x) * 12)
#define NOTE_D(x)       (2 + (x) * 12)
#define NOTE_D_SHARP(x) (3 + (x) * 12)
#define NOTE_E_FLAT(x)  (3 + (x) * 12)
#define NOTE_E(x)       (4 + (x) * 12)
#define NOTE_F(x)       (5 + (x) * 12)
#define NOTE_F_SHARP(x) (6 + (x) * 12)
#define NOTE_G_FLAT(x)  (6 + (x) * 12)
#define NOTE_G(x)       (7 + (x) * 12)
#define NOTE_G_SHARP(x) (8 + (x) * 12)
#define NOTE_A_FLAT(x)  (8 + (x) * 12)
#define NOTE_A(x)       (9 + (x) * 12)
#define NOTE_A_SHARP(x) (10 + (x) * 12)
#define NOTE_B_FLAT(x)  (10 + (x) * 12)
#define NOTE_B(x)       (11 + (x) * 12)
/** @} */

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

/**
 * Frequency table for the lowest 12 allowed notes.
 * The frequency is in tenth of Hertz.
 *
 * Note, every listed note will double the frequency.
 */
static uint16_t gNoteIndexToFreq[12] = {
    412, /* note E1 = 41.2 Hz */
    437, /* note F1 = 43.7 Hz */
    463, /* note F#1 = 46.3 Hz */
    490, /* note G1 = 49.0 Hz */
    519, /* note G#1 = 51.9 Hz */
    550, /* note A1 = 55.0 Hz */
    583, /* note A#1 = 58.3 Hz */
    617, /* note B1 = 61.7 Hz */
    654, /* note C2 = 65.4 Hz */
    693, /* note C#2 = 69.3 Hz */
    734, /* note D2 = 73.4 Hz */
    778  /* note D#2 = 77.8 Hz */
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Buzzer::Buzzer(webots::Speaker* speaker) :
    IBuzzer(),
    m_speaker(speaker),
    m_pathToWavFile(),
    m_melodySequence(nullptr),
    m_octave(DEFAULT_OCTAVE),
    m_wholeNoteDuration(DEFAULT_WHOLE_NOTE_DURATION),
    m_noteType(DEFAULT_NOTE_TYPE),
    m_volume(DEFAULT_VOLUME),
    m_staccato(DEFAULT_STACCATO),
    m_staccatoRestDuration(0),
    m_timestamp(0),
    m_duration(DEFAULT_DURATION),
    m_finished(true)
{
    /* Determine absolute path to wav file. */
    if (nullptr != getcwd(m_pathToWavFile, sizeof(m_pathToWavFile)))
    {
        size_t wavFileLen       = strlen(WAV_FILE_440_HZ);
        size_t pathToWavFileLen = strlen(m_pathToWavFile);

        if (MAX_PATH_SIZE > (pathToWavFileLen + wavFileLen))
        {
            strcpy(&m_pathToWavFile[pathToWavFileLen], WAV_FILE_440_HZ);
        }
    }
}

void Buzzer::playFrequency(uint16_t freq, uint16_t duration, uint8_t volume)
{
    if (MAX_VOLUME < volume)
    {
        volume = MAX_VOLUME;
    }

    if ((nullptr != m_speaker) && (0 < duration))
    {
        double BASE_FREQ = 440.0F; /* Hz */
        double simVolume = static_cast<double>(volume) / static_cast<double>(MAX_VOLUME);
        double frequency;
        double pitch;
        double balance = 0.0F;

        /* Determine frequency unit.
         * This is necessary to simulate target behaviour with the PololuBuzzer class.
         */
        if (0 != (freq & DIV_BY_10_BIT))
        {
            freq &= ~DIV_BY_10_BIT;
            frequency = static_cast<double>(freq) / 10.0F;
        }
        else
        {
            frequency = static_cast<double>(freq);
        }

        pitch = frequency / BASE_FREQ;

        m_speaker->playSound(m_speaker, m_speaker, m_pathToWavFile, simVolume, pitch, balance, true);

        m_timestamp = millis();
        m_duration  = duration;
        m_finished  = false;
    }
}

void Buzzer::playMelody(const char* sequence)
{
    m_melodySequence       = sequence;
    m_staccatoRestDuration = 0;

    nextNote();
}

void Buzzer::playMelodyPGM(const char* sequence)
{
    playMelody(sequence);
}

bool Buzzer::isPlaying()
{
    bool isPlaying = false;

    if (nullptr != m_speaker)
    {
        /* Is any sound playing? */
        isPlaying = m_speaker->isSoundPlaying(m_pathToWavFile);
    }

    return isPlaying;
}

void Buzzer::process()
{
    if (nullptr != m_speaker)
    {
        if (false == m_finished)
        {
            uint32_t timestamp = millis();
            uint32_t threshold = m_timestamp + m_duration;

            if (timestamp >= threshold)
            {
                m_speaker->stop(m_pathToWavFile);
                m_finished = true;

                if (nullptr != m_melodySequence)
                {
                    nextNote();
                }
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Buzzer::playNote(uint8_t note, uint16_t duration, uint8_t volume)
{
    /* note = key + octave * 12, where 0 <= key < 12
     *
     * Example: A4 = A + 4 * 12, where A = 9 (so A4 = 57)
     *
     * A note is converted to a frequency by the formula:
     *  Freq(n) = Freq(0) * a^n
     * where
     *  Freq(0) is chosen as A4, which is 440 Hz
     * and
     *  a = 2 ^ (1/12)
     * n is the number of notes you are away from A4.
     * One can see that the frequency will double every 12 notes.
     * This function exploits this property by defining the frequencies of the
     * 12 lowest notes allowed and then doubling the appropriate frequency
     * the appropriate number of times to get the frequency for the specified
     * note.
     *
     * if note = 16, freq = 41.2 Hz (E1 - lower limit as freq must be >40 Hz)
     * if note = 57, freq = 440 Hz (A4 - central value of ET Scale)
     * if note = 111, freq = 9.96 kHz (D#9 - upper limit, freq must be <10 kHz)
     * if note = 255, freq = NA and buzzer is silent (silent note)
     *
     * The most significant bit of freq is the "divide by 10" bit.  If set,
     * the units for frequency are .1 Hz, not Hz, and freq must be divided
     * by 10 to get the true frequency in Hz.  This allows for an extra digit
     * of resolution for low frequencies without the need for using floats.
     */
    if ((SILENT_NOTE == note) || (0 == volume))
    {
        playFrequency(1000, duration, 0);
    }
    else
    {
        const uint8_t FREQ_DOUBLE_COUNT = sizeof(gNoteIndexToFreq) / sizeof(gNoteIndexToFreq[0]);
        const uint8_t NOTE_MIN          = 16;  /* Lower limit must be > 40 Hz. */
        const uint8_t NOTE_MAX          = 111; /* Upper limit must be < 10 kHz */
        const uint8_t NOTE_COUNT        = 7;   /* C, D, E, F, G, A, B */
        uint16_t      freq              = 0;   /* 0.1 Hz */
        uint8_t       offsetNote;
        uint8_t       exponent;
        uint8_t       noteIndex;

        if (NOTE_MIN > note)
        {
            note = NOTE_MIN;
        }
        else if (NOTE_MAX < note)
        {
            note = NOTE_MAX;
        }
        else
        {
            ;
        }

        offsetNote = note - NOTE_MIN;
        exponent   = offsetNote / FREQ_DOUBLE_COUNT;
        noteIndex  = offsetNote % FREQ_DOUBLE_COUNT;
        freq       = gNoteIndexToFreq[noteIndex];

        if (NOTE_COUNT > exponent)
        {
            freq = freq << exponent; /* frequency *= 2 ^ exponent */

            /* If the frequency is greater than 160 Hz, we don't need the extra resolution otherwise
             * keep the added digit of resolution.
             */
            if (1 < exponent)
            {
                freq = (freq + 5) / 10;
            }
            else
            {
                /* Indicates 0.1 Hz resolution. */
                freq += DIV_BY_10_BIT;
            }
        }
        else
        {
            /* freq * 2^7 / 10 without int overflow. */
            freq = (freq * 64 + 2) / 5;
        }

        playFrequency(freq, duration, volume);
    }
}

void Buzzer::nextNote()
{
    if (nullptr != m_melodySequence)
    {
        /* If we are playing staccato, after every note we play a rest. */
        if ((true == m_staccato) && (0 < m_staccatoRestDuration))
        {
            playNote(SILENT_NOTE, m_staccatoRestDuration, 0);
            m_staccatoRestDuration = 0;
        }
        else
        {
            uint8_t  note     = 0;
            uint16_t duration = m_duration;
            uint8_t  volume   = m_volume;

            if (true == interprete(note, duration, volume))
            {
                playNote(note, duration, volume);

                /* Take volume over the other notes. */
                m_volume = volume;
            }
        }
    }
}

bool Buzzer::interprete(uint8_t& note, uint16_t& duration, uint8_t& volume)
{
    bool     repeat;
    bool     rest   = false;
    uint8_t  octave = m_octave;
    uint16_t dotAdd;
    char     character;

    do
    {
        character = getCurrentCharacter();

        if (nullptr != m_melodySequence)
        {
            ++m_melodySequence;
        }

        repeat = false;

        /* Melody finished? */
        if ('\0' == character)
        {
            m_melodySequence = nullptr;
        }
        /* Continue melody. */
        else
        {
            /* Interpret the character. */
            switch (character)
            {
            case '>':
                /* Shift the octave temporarily up. */
                ++octave;
                repeat = true;
                break;

            case '<':
                /* Shift the octave temporarily down. */
                --octave;
                repeat = true;
                break;

            case 'a':
                note = NOTE_A(0);
                break;

            case 'b':
                note = NOTE_B(0);
                break;

            case 'c':
                note = NOTE_C(0);
                break;

            case 'd':
                note = NOTE_D(0);
                break;

            case 'e':
                note = NOTE_E(0);
                break;

            case 'f':
                note = NOTE_F(0);
                break;

            case 'g':
                note = NOTE_G(0);
                break;

            case 'l':
                /* Set the default note duration. */
                m_noteType = getNumber();
                m_duration = m_wholeNoteDuration / m_noteType;
                repeat     = true;
                break;

            case 'm':
                /* Set music staccato or legato. */
                if ('l' == getCurrentCharacter())
                {
                    m_staccato = false;
                }
                else
                {
                    m_staccato             = true;
                    m_staccatoRestDuration = 0;
                }
                ++m_melodySequence;
                repeat = true;
                break;

            case 'o':
                /* Set the octave permanently. */
                m_octave  = getNumber();
                octave = m_octave;
                repeat    = true;
                break;

            case 'r':
                /* Rest - the note value doesn't matter. */
                rest = true;
                break;

            case 't':
                /* Set the tempo. */
                m_wholeNoteDuration = 60 * 400 / getNumber() * 10;
                m_duration          = m_wholeNoteDuration / m_noteType;
                repeat              = true;
                break;

            case 'v':
                /* Set the volume. */
                volume = getNumber();
                repeat   = true;
                break;

            case '!':
                /* Reset to defaults. */
                m_octave            = DEFAULT_OCTAVE;
                m_wholeNoteDuration = DEFAULT_WHOLE_NOTE_DURATION;
                m_noteType          = DEFAULT_NOTE_TYPE;
                m_duration          = DEFAULT_DURATION;
                m_volume            = DEFAULT_VOLUME;
                m_staccato          = DEFAULT_STACCATO;

                /* Reset temporary variables that depend on the defaults. */
                octave   = m_octave;
                duration = m_duration;

                repeat = true;
                break;
            default:
                m_melodySequence = nullptr;
                break;
            }
        }
    } while (true == repeat);

    if (nullptr != m_melodySequence)
    {
        note += octave * 12;

        /* Handle sharps and flats. */
        character = getCurrentCharacter();

        while (('+' == character) || ('#' == character))
        {
            ++m_melodySequence;
            ++note;

            character = getCurrentCharacter();
        }

        while ('-' == character)
        {
            ++m_melodySequence;
            --note;

            character = getCurrentCharacter();
        }

        /* Set the duration of just this note. */
        duration = m_duration;

        /* If the input is 'c16', make it a 16th note, etc. */
        if (('0' < character) && ('9' > character))
        {
            duration = m_wholeNoteDuration / getNumber();
        }

        /* Handle dotted notes - the first dot adds 50%, and each
         * additional dot adds 50% of the previous dot.
         */
        dotAdd = duration / 2;

        while ('.' == getCurrentCharacter())
        {
            ++m_melodySequence;
            duration += dotAdd;
            dotAdd /= 2;
        }

        if (true == m_staccato)
        {
            m_staccatoRestDuration = duration / 2;
            duration -= m_staccatoRestDuration;
        }

        if (true == rest)
        {
            note = SILENT_NOTE;
            
        }
    }

    return (nullptr != m_melodySequence);
}

char Buzzer::getCurrentCharacter()
{
    char character = '\0';

    while ((nullptr != m_melodySequence) && ('\0' != *m_melodySequence))
    {
        character = *m_melodySequence;

        if (('A' <= character) && ('Z' >= character))
        {
            character += 'a' - 'A';
        }

        if (' ' != character)
        {
            break;
        }

        ++m_melodySequence;
    }

    return character;
}

uint16_t Buzzer::getNumber()
{
    uint16_t value     = 0;
    char     character = getCurrentCharacter();

    /* Read all digits, one at a time. */
    while (('0' <= character) && ('9' >= character))
    {
        value *= 10;
        value += character - '0';

        ++m_melodySequence;

        character = getCurrentCharacter();
    }

    return value;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
