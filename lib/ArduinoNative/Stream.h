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
 *  @brief  Data Stream class definition
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

#ifndef STREAM_H
#define STREAM_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Class for definition of data Streams
 */
class Stream
{
public:
    /** Destruct Stream */
    virtual ~Stream()
    {
    }

    /**
     * Print argument to the Output Stream.
     * @param[in] str Argument to print.
     */
    virtual void print(const char str[]) = 0;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    virtual void print(uint8_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    virtual void print(uint16_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    virtual void print(uint32_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    virtual void print(int8_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    virtual void print(int16_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * @param[in] value Argument to print.
     */
    virtual void print(int32_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] str Argument to print.
     */
    virtual void println(const char str[]) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(uint8_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(uint16_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(uint32_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(int8_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(int16_t value) = 0;

    /**
     * Print argument to the Output Stream.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(int32_t value) = 0;

    /**
     * Write bytes to stream.
     * @param[in] buffer Byte Array to send.
     * @param[in] length Length of Buffer.
     * @returns Number of bytes written
     */
    virtual size_t write(const uint8_t* buffer, size_t length) = 0;

    /**
     * Check if there are available bytes in the Stream.
     * @returns Number of available bytes.
     */
    virtual int available() const = 0;

    /**
     * Read bytes into a buffer.
     * @param[in] buffer Array to write bytes to.
     * @param[in] length number of bytes to be read.
     * @returns Number of bytes read from Stream.
     */
    virtual size_t readBytes(uint8_t* buffer, size_t length) = 0;

protected:
    /** Construct Stream */
    Stream()
    {
    }
};

#endif /* STREAM_H_ */