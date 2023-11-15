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
#include <RelativeEncoders.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** Test vector element */
typedef struct
{
    int16_t encoderStepsLeft;   /**< Absolute encoder steps left */
    int16_t encoderStepsRight;  /**< Absolute encoder steps right */
    bool    isTest;             /**< Test it or is it for test preparation? */
    int16_t expectedDiffLeft;   /**< Expected step difference left */
    int16_t expectedDiffRight;  /**< Expected step difference right */

} Elem;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testRelativeEncoders();

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

    RUN_TEST(testRelativeEncoders);

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
 * Test the relative encoders unit.
 */
static void testRelativeEncoders()
{
    IEncoders&          absEncoders     = Board::getInstance().getEncoders();
    IEncodersTest&      absEncodersTest = Board::getInstance().getEncodersTest();
    RelativeEncoders    relEncoders(absEncoders);
    Elem                testVector[]    =
    {
        { 0,            0,          true,   0,      0   },
        { 10,           10,         true,   10,     10  },
        { -10,          -10,        true,   -10,    -10 },
        { INT16_MAX,    INT16_MAX,  false,  0,      0   }, /* Preparation for wrap-around */
        { INT16_MIN,    INT16_MIN,  true,   1,      1   },
        { INT16_MIN,    INT16_MIN,  false,  0,      0   }, /* Preparation for wrap-around */
        { INT16_MAX,    INT16_MAX,  true,   -1,     -1  }
    };
    uint8_t             idx             = 0;

    idx = 0;
    while ((sizeof(testVector) / sizeof(testVector[0])) > idx)
    {
        absEncodersTest.setCountsLeft(testVector[idx].encoderStepsLeft);
        absEncodersTest.setCountsRight(testVector[idx].encoderStepsRight);
        
        if (false == testVector[idx].isTest)
        {
            relEncoders.clearLeft();
            relEncoders.clearRight();
        }
        else
        {
            TEST_ASSERT_EQUAL_INT16(testVector[idx].expectedDiffLeft, relEncoders.getCountsLeft());
            TEST_ASSERT_EQUAL_INT16(testVector[idx].expectedDiffRight, relEncoders.getCountsRight());
        }

        ++idx;
    }
}
