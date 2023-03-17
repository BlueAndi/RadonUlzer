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

/**
 * @author  Write here the name of the author.
 * @brief   This module contains the program entry point for the tests.
 */

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <unity.h>
#include <SimpleTimer.h>
#include <PIDController.h>
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

static void testSimpleTimer();
static void testPIDController();
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

    RUN_TEST(testSimpleTimer);
    RUN_TEST(testPIDController);
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
 * Test the SimpleTimer class.
 */
static void testSimpleTimer()
{
    const uint32_t WAIT_TIME = 100;
    const uint32_t DELTA_MIN = 1;
    SimpleTimer    testTimer;

    /* If timer is not running, it shall not signal timeout. */
    TEST_ASSERT_FALSE(testTimer.isTimeout());

    /* Start timer with 0. It must signal timeout immediately. */
    testTimer.start(0);
    TEST_ASSERT_TRUE(testTimer.isTimeout());

    /* Stop timer. It must not signal timeout. */
    testTimer.stop();
    TEST_ASSERT_FALSE(testTimer.isTimeout());

    /* Start timer with 100 ms. It must not signal timeout. */
    testTimer.start(WAIT_TIME);
    TEST_ASSERT_FALSE(testTimer.isTimeout());

    /* Test timeout signalling. */
    delay(WAIT_TIME + DELTA_MIN);
    TEST_ASSERT_TRUE(testTimer.isTimeout());

    /* Restart timer. It must not signal timeout. */
    testTimer.restart();
    TEST_ASSERT_FALSE(testTimer.isTimeout());

    /* Test timeout signalling after a restart. */
    delay(WAIT_TIME + DELTA_MIN);
    TEST_ASSERT_TRUE(testTimer.isTimeout());

    /* Verify timer duration till now. */
    TEST_ASSERT_GREATER_OR_EQUAL(WAIT_TIME + DELTA_MIN, testTimer.getCurrentDuration());
}

/**
 * Test the PICController class.
 */
static void testPIDController()
{
    PIDController<int16_t> pidCtrl;
    uint8_t                index            = 0;
    int16_t                output           = 0;
    const uint32_t         TEST_SAMPLE_TIME = 0; /* Every PID calculate() call shall be processed */

    /* Sample time for all tests shall be set once. */
    pidCtrl.setSampleTime(TEST_SAMPLE_TIME);

    /* Kp = 1, Ki = 0, Kd = 0 */
    pidCtrl.setPFactor(1, 1);
    pidCtrl.setIFactor(0, 1);
    pidCtrl.setDFactor(0, 1);
    pidCtrl.clear();

    /* Output must follow error */
    output = 0;
    for (index = 0; index < 10; ++index)
    {
        output = 0 - index;
        TEST_ASSERT_EQUAL_INT16(output, pidCtrl.calculate(0, index));
    }

    /* Kp = 0, Ki = 1, Kd = 0 */
    pidCtrl.setPFactor(0, 1);
    pidCtrl.setIFactor(1, 1);
    pidCtrl.setDFactor(0, 1);
    pidCtrl.clear();

    /* Output must increase per error */
    output = 0;
    for (index = 0; index < 10; ++index)
    {
        output += (0 - index);
        TEST_ASSERT_EQUAL_INT16(output, pidCtrl.calculate(0, index));
    }

    /* Kp = 0, Ki = 0, Kd = 1 */
    pidCtrl.setPFactor(0, 1);
    pidCtrl.setIFactor(0, 1);
    pidCtrl.setDFactor(1, 1);
    pidCtrl.clear();

    /* Output must be equal to error deviation from previous error */
    output = 0;
    for (index = 1; index < 10; ++index)
    {
        output = -1;
        TEST_ASSERT_EQUAL_INT16(output, pidCtrl.calculate(0, index));
    }
}

/**
 * Test the utilities.
 */
static void testUtil()
{
    uint32_t    testVectorUInt[]     = {0, UINT32_MAX / 2, UINT32_MAX};
    const char* expectedResultUInt[] = {"0", "2147483647", "4294967295"};
    int32_t     testVectorInt[]      = {INT32_MIN, INT32_MIN / 2, 0, INT32_MAX / 2, INT32_MAX};
    const char* expectedResultInt[]  = {"-2147483647", "-1073741824", "0", "1073741823", "2147483647"};
    uint8_t     idx                  = 0;

    /* Standard use case */
    while (sizeof(testVectorUInt) / sizeof(testVectorUInt[0]) > idx)
    {
        char result[12];

        Util::uintToStr(result, sizeof(result), testVectorUInt[idx]);

        TEST_ASSERT_EQUAL_STRING(expectedResultUInt[idx], result);

        ++idx;
    }

    /* With limited result buffer. */
    while (sizeof(testVectorUInt) / sizeof(testVectorUInt[0]) > idx)
    {
        char result[4];

        Util::uintToStr(result, sizeof(result), testVectorUInt[idx]);

        TEST_ASSERT_EQUAL_STRING_ARRAY(expectedResultUInt[idx], result, sizeof(result) - 1);

        ++idx;
    }

    /* Standard use case */
    while (sizeof(testVectorInt) / sizeof(testVectorInt[0]) > idx)
    {
        char result[12];

        Util::intToStr(result, sizeof(result), testVectorInt[idx]);

        TEST_ASSERT_EQUAL_STRING(expectedResultInt[idx], result);

        ++idx;
    }

    /* With limited result buffer. */
    while (sizeof(testVectorInt) / sizeof(testVectorInt[0]) > idx)
    {
        char result[4];

        Util::intToStr(result, sizeof(result), testVectorInt[idx]);

        TEST_ASSERT_EQUAL_STRING_ARRAY(expectedResultInt[idx], result, sizeof(result) - 1);

        ++idx;
    }
}