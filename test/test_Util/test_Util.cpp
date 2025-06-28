/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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

/**
 * @author  Andreas Merkle <web@blue-andi.de>
 * @brief   This module contains the Util tests.
 */

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <unity.h>
#include <Util.h>

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

static void testUtil();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

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

/**
 * Program setup routine, which is called once at startup.
 */
void setup()
{
#ifndef TARGET_NATIVE
    /* https://docs.platformio.org/en/latest/plus/unit-testing.html#demo */
    delay(2000);
#endif /* Not defined TARGET_NATIVE */
}

/**
 * Main entry point.
 */
void loop()
{
    UNITY_BEGIN();

    RUN_TEST(testUtil);

    UNITY_END();

#ifndef TARGET_NATIVE
    /* Don't exit on the robot to avoid a endless test loop.
     * If the test runs on the pc, it must exit.
     */
    for (;;)
    {
    }
#endif /* Not defined TARGET_NATIVE */
}

/**
 * Initialize the test setup.
 */
extern void setUp(void)
{
    /* Not used. */
}

/**
 * Clean up test setup.
 */
extern void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Test the utilities.
 */
static void testUtil()
{
    uint32_t     testVectorUInt[]         = {0, UINT32_MAX / 2, UINT32_MAX};
    const char*  expectedResultUInt[]     = {"0", "2147483647", "4294967295"};
    int32_t      testVectorInt[]          = {INT32_MIN, INT32_MIN / 2, 0, INT32_MAX / 2, INT32_MAX};
    const char*  expectedResultInt[]      = {"-2147483647", "-1073741824", "0", "1073741823", "2147483647"};
    uint8_t      idx                      = 0;
    const size_t RESULT_STD_BUFFER_SIZE   = 12;
    const size_t RESULT_LIMIT_BUFFER_SIZE = 4;

    /* Standard use case */
    while (sizeof(testVectorUInt) / sizeof(testVectorUInt[0]) > idx)
    {
        char result[RESULT_STD_BUFFER_SIZE];

        Util::uintToStr(result, sizeof(result), testVectorUInt[idx]);

        TEST_ASSERT_EQUAL_STRING(expectedResultUInt[idx], result);

        ++idx;
    }

    /* With limited result buffer. */
    while (sizeof(testVectorUInt) / sizeof(testVectorUInt[0]) > idx)
    {
        char result[RESULT_LIMIT_BUFFER_SIZE];

        Util::uintToStr(result, sizeof(result), testVectorUInt[idx]);

        TEST_ASSERT_EQUAL_STRING_ARRAY(expectedResultUInt[idx], result, sizeof(result) - 1);

        ++idx;
    }

    /* Standard use case */
    while (sizeof(testVectorInt) / sizeof(testVectorInt[0]) > idx)
    {
        char result[RESULT_STD_BUFFER_SIZE];

        Util::intToStr(result, sizeof(result), testVectorInt[idx]);

        TEST_ASSERT_EQUAL_STRING(expectedResultInt[idx], result);

        ++idx;
    }

    /* With limited result buffer. */
    while (sizeof(testVectorInt) / sizeof(testVectorInt[0]) > idx)
    {
        char result[RESULT_LIMIT_BUFFER_SIZE];

        Util::intToStr(result, sizeof(result), testVectorInt[idx]);

        TEST_ASSERT_EQUAL_STRING_ARRAY(expectedResultInt[idx], result, sizeof(result) - 1);

        ++idx;
    }
}
