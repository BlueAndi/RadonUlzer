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
 * 
 * @addtogroup Service
 *
 * @{
 */

#ifndef SOUND_H
#define SOUND_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

/** The buzzer can play different kind of notification sounds. */
namespace Sound
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Supported melodies.
 */
typedef enum
{
    MELODY_WELCOME = 0, /**< Welcome theme */
    MELODY_STAR_WARS    /**< Star Wars imperial march theme */

} Melody;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Play alarm tone.
 */
void playAlarm();

/**
 * Play beep tone.
 */
void playBeep();

/**
 * Play melody.
 * 
 * @param[in] melody Choose the melody which to play.
 */
void playMelody(Melody melody);

}

#endif /* SOUND_H */
/** @} */
