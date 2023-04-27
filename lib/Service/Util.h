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
 * @brief  Utilities
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef UTIL_H
#define UTIL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdlib.h>

/**
 * Utilities
 */
namespace Util
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Unsigned integer to string, without preceeding zeros.
 * 
 * @param[out]  str     Destination string
 * @param[in]   size    Size of the destination string in byte
 * @param[in]   value   Value
 */
void uintToStr(char* str, size_t size, uint32_t value);

/**
 * Signed integer to string, without preceeding zeros.
 * 
 * @param[out]  str     Destination string
 * @param[in]   size    Size of the destination string in byte
 * @param[in]   value   Value
 */
void intToStr(char* str, size_t size, int32_t value);

/**
 * Signed 32-bit integer to byte array.
 * Endianness: Big Endian
 * @param[out]  buffer  Destination array
 * @param[in]   size    Size of the destination buffer in byte
 * @param[in]   value   Value
 */
void int32ToByteArray(uint8_t* buffer, size_t size, int32_t value);

/**
 * Unsigned 32-bit integer to byte array.
 * Endianness: Big Endian.
 * @param[out]  buffer  Destination array.
 * @param[in]   size    Size of the destination buffer in byte.
 * @param[in]   value   Value.
 */
void uint32ToByteArray(uint8_t* buffer, size_t size, uint32_t value);

/**
 * Big Endian Byte Array to uint32_t.
 * @param[in]  buffer   Source Array.
 * @param[in]  size     Size of source array.
 * @param[out] value    Destination integer.
 * @returns true if succesfully parsed. Otherwise, false.
 */
bool byteArrayToUint32(const uint8_t* buffer, size_t size, uint32_t& value);

} // namespace Util

#endif /* UTIL_H */
/** @} */
