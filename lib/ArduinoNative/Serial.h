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
 * @brief  Implementation of Arduino Serial
 * @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef SERIAL_H
#define SERIAL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "Stream.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Serial driver, used by Arduino applications. */
class Serial_ : public Stream
{
public:
    /**
     * Construct Serial_.
     * @param[in] stream Output Stream.
     */
    Serial_(Stream& stream);

    /**
     * Destroy Serial_.
     */
    ~Serial_();

    /**
     * Begin Serial_ Communication.
     * @param[in] baudrate Comm. Speed in bits per second
     */
    void begin(unsigned long baudrate);

    /**
     * End the Serial_ Communication.
     */
    void end();

    /**
     * Print argument to the Output Stream.
     * @param[in] str Argument to print.
     */
    void print(const char str[]) final;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    void print(uint8_t value) final;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    void print(uint16_t value) final;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    void print(uint32_t value) final;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    void print(int8_t value) final;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    void print(int16_t value) final;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    void print(int32_t value) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] str Argument to print.
     */
    void println(const char str[]) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    void println(uint8_t value) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    void println(uint16_t value) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    void println(uint32_t value) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    void println(int8_t value) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    void println(int16_t value) final;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    void println(int32_t value) final;

    /**
     * Write bytes to stream.
     * @param[in] buffer Byte Array to send.
     * @param[in] length Length of Buffer.
     * @returns Number of bytes written
     */
    size_t write(const uint8_t* buffer, size_t length) final;

    /**
     * Check if there are available bytes in the Stream.
     * @returns Number of available bytes.
     */
    int available() const final;

    /**
     * Read bytes into a buffer.
     * @param[in] buffer Array to write bytes to.
     * @param[in] length number of bytes to be read.
     * @returns Number of bytes read from Stream.
     */
    size_t readBytes(uint8_t* buffer, size_t length) final;

private:
    /**
     * Stream for input and output of data.
     */
    Stream& m_stream;

    /* Prevent empty constructor*/
    Serial_();
};

/* Serial driver, used by Arduino applications. */
extern Serial_ Serial;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SERIAL_H */
/** @} */
