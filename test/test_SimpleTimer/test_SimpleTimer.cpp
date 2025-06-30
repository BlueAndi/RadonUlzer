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
 * @brief   This module contains the SimpleTimer tests.
 */

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <unity.h>
#include <SimpleTimer.h>

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
