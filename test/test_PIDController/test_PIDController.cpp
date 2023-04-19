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
 * @author  Andreas Merkle <web@blue-andi.de>
 * @brief   This module contains the PID Controller tests.
 */

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <PIDController.h>

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

static void testPIDController();

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

    RUN_TEST(testPIDController);

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
