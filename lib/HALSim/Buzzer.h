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
 *
 * @addtogroup HALSim
 *
 * @{
 */

#ifndef BUZZER_H
#define BUZZER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IBuzzer.h"

#include <webots/Speaker.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the robot simulation buzzer. */
class Buzzer : public IBuzzer
{
public:
    /**
     * Constructs the buzzer.
     */
    Buzzer(webots::Speaker* speaker);

    /**
     * Destroys the buzzer.
     */
    ~Buzzer()
    {
    }

    /**
     * If the frequency is in 0.1 Hz, this bit must be set otherwise the
     * frequency unit will be considered in Hz.
     */
    static const uint16_t DIV_BY_10_BIT = (1 << 15);

    /**
     * Max. volume.
     */
    static const uint8_t MAX_VOLUME = 15;

    /**
     * Plays the specified frequency for the specified duration.
     *
     * This function plays the note in the background while your program continues
     * to execute. If you call another buzzer function while the note is playing,
     * the new function call will overwrite the previous and take control of the
     * buzzer.
     *
     * @warning @a frequency &times; @a duration / 1000 must be no greater than
     * 0xFFFF (65535). This means you can't use a duration of 65535 ms for
     * frequencies greater than 1 kHz. For example, the maximum duration you can
     * use for a frequency of 10 kHz is 6553 ms. If you use a duration longer than
     * this, you will produce an integer overflow that can result in unexpected
     * behavior.
     *
     * @param[in] freq        Frequency to play in Hz or 0.1 Hz depended on divBy10 bit.
     * @param[in] duration    Duration of the note in milliseconds.
     * @param[in] volume      Volume of the note (0-15).
     */
    void playFrequency(uint16_t freq, uint16_t duration, uint8_t volume) final;

    /**
     * Plays a melody sequence out of RAM.
     *
     * @param[in] sequence Melody sequence in RAM
     */
    void playMelody(const char* sequence) final;

    /**
     * Plays a melody sequence out of program space.
     *
     * @param[in] sequence Melody sequence in program space
     */
    void playMelodyPGM(const char* sequence) final;

    /**
     * Checks whether a note, frequency, or sequence is being played.
     * 
     * Note: Avoid calling this method inside a loop without processing the
     * buzzer. On the simulation additional the simulation time needs to run!
     *
     * @return If the buzzer is current playing a note, frequency, or sequence it will
     * return true otherwise false.
     */
    bool isPlaying() final;

    /**
     * Process the buzzer to handle sound timings.
     */
    void process() final;

private:
    /** Max. size of a full path. */
    static const size_t MAX_PATH_SIZE = 1024;

    /** Sound file with 440 Hz signal. */
    static const char* WAV_FILE_440_HZ;

    /** The default octave. */
    static const uint8_t DEFAULT_OCTAVE = 4;

    /** The default duration of a whole note in ms. */
    static const uint32_t DEFAULT_WHOLE_NOTE_DURATION = 2000;

    /** The default note type, 4 for quarter and etc. */
    static const uint32_t DEFAULT_NOTE_TYPE = 4;

    /** The default note duration in ms. */
    static const uint32_t DEFAULT_DURATION = 500;

    /** The default note volume. */
    static const uint8_t DEFAULT_VOLUME = MAX_VOLUME;

    /** The default staccato state. */
    static const bool DEFAULT_STACCATO = false;

    /** Silences buzzer for the note duration. */
    static const uint8_t SILENT_NOTE = 0xFF;

    webots::Speaker* m_speaker;                      /**< The robot simulated speaker. */
    char             m_pathToWavFile[MAX_PATH_SIZE]; /**< Absolute path to wav file. */
    const char*      m_melodySequence;               /**< Melody sequence */
    uint8_t          m_octave;                       /**< The current octave */
    uint32_t         m_wholeNoteDuration;            /**< The duration in ms of a whole note. */
    uint32_t         m_noteType;                     /**< 4 for quarter, etc. */
    uint8_t          m_volume;                       /**< The note volume. */
    bool             m_staccato;                     /**< true if playing staccato. */
    uint8_t          m_staccatoRestDuration; /**< Duration of a staccato rest or zero if it is time to play a note. */
    uint32_t         m_timestamp;            /**< Timestamp in ms to handle sound timings. */
    uint16_t         m_duration;             /**< Frequency/Note duration in ms how long it shall be played. */
    bool             m_finished;             /**< Is false while playing. */

    /* Default constructor not allowed. */
    Buzzer();

    /**
     * Determine the frequency for the specified note, then play that note
     * for the desired duration (in ms). This is done without using floats
     * and without having to loop. volume controls buzzer volume, with 15 being
     * loudest and 0 being quietest.
     *
     * Note: frequency * duration / 1000 must be less than 0xFFFF (65535).  This
     *      means that you can't use a max duration of 65535 ms for frequencies
     *      greater than 1 kHz.  For example, the max duration you can use for a
     *      frequency of 10 kHz is 6553 ms.  If you use a duration longer than this,
     *      you will cause an integer overflow that produces unexpected behavior.
     *
     * @param[in] note      The note to play.
     * @param[in] duration  Duration in ms.
     * @param[in] volume    The volume [0-15].
     */
    void playNote(uint8_t note, uint16_t duration, uint8_t volume);

    /**
     * Handle next note in the melody sequence.
     */
    void nextNote();

    /**
     * Interprete the next characters for one note in the melody sequence.
     * 
     * @param[out]      note        The note which to play.
     * @param[out]      duration    The note duration in ms how long it shall be played.
     * @param[in,out]   volume      The note volume, which shall be taken over for future notes.
     * 
     * @return If melody sequence continues, it will return true otherwise false.
     */
    bool interprete(uint8_t& note, uint16_t& duration, uint8_t& volume);

    /**
     * Get current character from melody lower-case.
     * Any space will automatically increment the melody sequence.
     *
     * @return Current character
     */
    char getCurrentCharacter();

    /**
     * Get the numerical value at the melody sequence begin and
     * incremenet the melody sequence after the number.
     *
     * @return Number
     */
    uint16_t getNumber();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BUZZER_H */
/** @} */
