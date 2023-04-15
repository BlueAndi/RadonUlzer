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

class Serial_ : public Stream
{
public:
    Serial_();

    ~Serial_();

    void begin(unsigned long baudrate);

    void end();

    void print(const char str[]);

    void print(uint8_t value);

    void print(uint16_t value);

    void print(uint32_t value);

    void print(int8_t value);

    void print(int16_t value);

    void print(int32_t value);

    void println(const char str[]);

    void println(uint8_t value);

    void println(uint16_t value);

    void println(uint32_t value);

    void println(int8_t value);

    void println(int16_t value);

    void println(int32_t value);

    size_t write(const uint8_t* buffer, size_t length) final;

    int available() final;

    size_t readBytes(uint8_t* buffer, size_t length) final;
};

extern Serial_ Serial;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SERIAL_H */