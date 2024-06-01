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
 *  @brief  Print class definition
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

#ifndef PRINT_H
#define PRINT_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Class for definition of Print
 */
class Print
{
public:
    /** Destruct Print */
    virtual ~Print()
    {
    }

    /**
     * Print argument.
     * @param[in] str Argument to print.
     */
    virtual void print(const char str[]) = 0;

    /**
     * Print argument.
     * @param[in] value Argument to print.
     */
    virtual void print(uint8_t value) = 0;

    /**
     * Print argument.
     * @param[in] value Argument to print.
     */
    virtual void print(uint16_t value) = 0;

    /**
     * Print argument.
     * @param[in] value Argument to print.
     */
    virtual void print(uint32_t value) = 0;

    /**
     * Print argument.
     * @param[in] value Argument to print.
     */
    virtual void print(int8_t value) = 0;

    /**
     * Print argument.
     * @param[in] value Argument to print.
     */
    virtual void print(int16_t value) = 0;

    /**
     * Print argument.
     * @param[in] value Argument to print.
     */
    virtual void print(int32_t value) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] str Argument to print.
     */
    virtual void println(const char str[]) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(uint8_t value) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(uint16_t value) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(uint32_t value) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(int8_t value) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(int16_t value) = 0;

    /**
     * Print argument.
     * Appends Carriage Return at the end of the argument.
     * @param[in] value Argument to print.
     */
    virtual void println(int32_t value) = 0;

    /**
     * Write bytes.
     * @param[in] buffer Byte Array to send.
     * @param[in] length Length of Buffer.
     * @returns Number of bytes written
     */
    virtual size_t write(const uint8_t* buffer, size_t length) = 0;

protected:
    /** Construct Print */
    Print()
    {
    }
};

#endif /* PRINT_H_ */
/** @} */
