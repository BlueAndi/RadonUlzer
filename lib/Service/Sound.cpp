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
 * @brief  Sound
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Sound.h>
#include <stdint.h>
#include <Board.h>
#include <Arduino.h>

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

/** Alarm frequency in Hz. */
static const uint16_t ALARM_FREQ = 500;

/** Alarm tone duration in ms. */
static const uint16_t ALARM_DURATION = 333;

/** Silence duration in ms, between two consecutive alarm tones. */
static const uint16_t SILENCE_DURATION = 333;

/** Beep frequency in Hz. */
static const uint16_t BEEP_FREQ = 1000;

/** Beep tone duration in ms. */
static const uint16_t BEEP_DURATION = 333;

/** General volume in digits. */
static const uint8_t VOLUME = 10;

/** Welcome theme melody. */
static const char gWelcomeMelody[] PROGMEM = "O4 T100 V15 L4 MS g12>c12>e12>G6>E12 ML>G2";

/** Star Wars theme melody. */
static const char gStarWarsMelody[] PROGMEM = "! O2 T100 MS"
                                              "a8. r16 a8. r16 a8. r16 f8 r16 >c16"
                                              "ML"
                                              "a8. r16 f8 r16"
                                              "MS"
                                              ">c16 a. r8"
                                              "O3"
                                              "e8. r16 e8. r16 e8. r16 f8 r16 <c16"
                                              "O2 ML"
                                              "a-8. r16"
                                              "MS"
                                              "f8 r16 >c16 a r"

                                              "O3 ML"
                                              "a8. r16 <a8 r16 <a16"
                                              "MS"
                                              "a8. r16 a-8 r16 g16"
                                              "ML V10"
                                              "g-16 f16 g-16 r16 r8 <b-16 r16"
                                              "MS"
                                              "e-8. r16 d8."
                                              "ML"
                                              "d-16"
                                              "c16 <c-16 c16 r16 r8"
                                              "MS O2"
                                              "f16 r16 a-8. r16 f8. a-16"
                                              "O3"
                                              "c8. r16 <a8 r16 c16 e2. r8"

                                              "O3 ML"
                                              "a8. r16 <a8 r16 <a16"
                                              "MS"
                                              "a8. r16 a-8 r16 g16"
                                              "ML V10"
                                              "g-16 f16 g-16 r16 r8 <b-16 r16"
                                              "MS"
                                              "e-8. r16 d8."
                                              "ML"
                                              "d-16"
                                              "c16 <c-16 c16 r16 r8"
                                              "MS O2"
                                              "f16 r16 a-8. r16 f8. >c16"
                                              "ML"
                                              "a8. r16 f8 r16 >c16 a2. r8";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

void Sound::playAlarm()
{
    IBuzzer& buzzer = Board::getInstance().getBuzzer();

    /* Req. 3.4.5-2:
     * The Sound shall play two consecutive sounds of 500Hz frequency and 1/3s duration,
     * interrupted by 1/3s of silence.
     */
    buzzer.playFrequency(ALARM_FREQ, ALARM_DURATION, VOLUME);
    while (true == buzzer.isPlaying())
    {
        /* Only required for simulation:
         * 1) The sound is stopped by the buzzer process(). Without stopping it,
         * the isPlaying() will always return true.
         * 2) The simulation time must be stepped forward, otherwise during process()
         * the sound won't stop. The workaround is here to use a delay() which
         * internally ticks the simulation time.
         */
        buzzer.process();
        delay(1U);
    }

    delay(SILENCE_DURATION);
    buzzer.playFrequency(ALARM_FREQ, ALARM_DURATION, VOLUME);
}

void Sound::playBeep()
{
    IBuzzer& buzzer = Board::getInstance().getBuzzer();

    /* Req. 3.4.5-1:
     * The Sound shall play a sound of 1000Hz frequency and 1s duration.
     */
    buzzer.playFrequency(BEEP_FREQ, BEEP_DURATION, VOLUME);
}

void Sound::playMelody(Melody melody)
{
    IBuzzer&    buzzer = Board::getInstance().getBuzzer();
    const char* song = nullptr;

    switch(melody)
    {
    case MELODY_WELCOME:
        song = gWelcomeMelody;
        break;

    case MELODY_STAR_WARS:
        song = gStarWarsMelody;
        break;

    default:
        break;
    }

    if (nullptr != song)
    {
        buzzer.playMelodyPGM(song);
    }
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
