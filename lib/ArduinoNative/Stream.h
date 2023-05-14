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
#include "Print.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Class for definition of data Streams
 */
class Stream : public Print
{
public:
    /** Destruct Stream */
    virtual ~Stream()
    {
    }

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
    Stream() : Print()
    {
    }
};

#endif /* STREAM_H_ */
/** @} */
