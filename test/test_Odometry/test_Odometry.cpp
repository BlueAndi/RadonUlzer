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
#include <Odometry.h>
#include <RobotConstants.h>

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

static void  testOrientation();
static void  testPosition();
static float calcStepsToAngle(float angle, bool turnInPlace);
static float calcStepsToDistance(float distance);
static void  testTurnInPlace(float angle);
static void  testTurn(float angle);
static void  testDriveDistance(float distance);

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

    RUN_TEST(testOrientation);
    RUN_TEST(testPosition);

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

static void testOrientation()
{
    testTurnInPlace(0);
    testTurnInPlace(M_PI / 2.0f);
    testTurnInPlace(M_PI);
    testTurnInPlace(M_PI * 3.0f / 2.0f);
    testTurnInPlace(M_PI * 2.0f);
    testTurnInPlace(-M_PI / 2.0f);
    testTurnInPlace(-M_PI);
    testTurnInPlace(-M_PI * 3.0f / 2.0f);
    testTurnInPlace(-M_PI * 2.0f);

    testTurn(0);
    testTurn(M_PI / 2.0f);
    testTurn(M_PI);
    testTurn(M_PI * 3.0f / 2.0f);
    testTurn(M_PI * 2.0f);
    testTurn(-M_PI / 2.0f);
    testTurn(-M_PI);
    testTurn(-M_PI * 3.0f / 2.0f);
    testTurn(-M_PI * 2.0f);
}

static void testPosition()
{
    testDriveDistance(0);
    testDriveDistance(10);
    testDriveDistance(200);
    testDriveDistance(1000);
    testDriveDistance(static_cast<float>(INT16_MAX / 2) * 1000.0f /
                      static_cast<float>(RobotConstants::ENCODER_STEPS_PER_M));
    testDriveDistance(-10);
    testDriveDistance(-200);
    testDriveDistance(-1000);
    testDriveDistance(static_cast<float>(INT16_MIN / 2) * 1000.0f /
                      static_cast<float>(RobotConstants::ENCODER_STEPS_PER_M));
}

static float calcStepsToAngle(float angle, bool turnInPlace)
{
    float wheelBase     = static_cast<float>(RobotConstants::WHEEL_BASE);                    /* [mm] */
    float diameter      = (false == turnInPlace) ? 2.0f * wheelBase : wheelBase;             /* [mm] */
    float stepsPerMM    = static_cast<float>(RobotConstants::ENCODER_STEPS_PER_M) / 1000.0f; /* [steps/mm] */
    float circumference = M_PI * diameter;                                                   /* [mm] */

    return circumference * stepsPerMM * angle / (2 * M_PI);
}

static float calcStepsToDistance(float distance)
{
    float stepsPerMM = static_cast<float>(RobotConstants::ENCODER_STEPS_PER_M) / 1000.0f; /* [steps/mm] */

    return stepsPerMM * distance;
}

static void testTurnInPlace(float angle)
{
    IEncodersTest& encodersTest = Board::getInstance().getEncodersTest();
    Odometry&      odometry     = Odometry::getInstance();
    float          stepsToAngle = calcStepsToAngle(angle, true);
    int16_t        stepsLeft    = -static_cast<int16_t>(stepsToAngle);
    int16_t        stepsRight   = static_cast<int16_t>(stepsToAngle);
    float          angleEpsilon = M_PI * 1.0f / 180.0f; /* 1° epsilon in rad */
    int32_t        posX         = 0;
    int32_t        posY         = 0;

    TEST_ASSERT(static_cast<float>(INT16_MAX) >= stepsToAngle);
    TEST_ASSERT(static_cast<float>(INT16_MIN) <= stepsToAngle);

    encodersTest.setCountsLeft(0);
    encodersTest.setCountsRight(0);
    odometry.clearPosition();
    odometry.clearMileage();
    odometry.setOrientation(0);
    TEST_ASSERT_EQUAL_INT32(0, odometry.getOrientation());

    encodersTest.setCountsLeft(stepsLeft);
    encodersTest.setCountsRight(stepsRight);
    odometry.process();

    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(0, posY);

    if (0.001 > fabs(angle))
    {
        TEST_ASSERT_TRUE(odometry.isStandStill());
    }
    else
    {
        TEST_ASSERT_FALSE(odometry.isStandStill());
    }

    TEST_ASSERT_INT32_WITHIN(static_cast<int32_t>(angleEpsilon * 1000.0f), static_cast<int32_t>(angle * 1000.0f),
                             odometry.getOrientation());
}

static void testTurn(float angle)
{
    IEncodersTest& encodersTest = Board::getInstance().getEncodersTest();
    Odometry&      odometry     = Odometry::getInstance();
    float          stepsToAngle = calcStepsToAngle(angle, false);
    int16_t        stepsLeft    = (0 <= angle) ? 0 : static_cast<int16_t>(stepsToAngle);
    int16_t        stepsRight   = (0 <= angle) ? static_cast<int16_t>(stepsToAngle) : 0;
    float          angleEpsilon = M_PI * 1.0f / 180.0f; /* 1° epsilon in rad */

    TEST_ASSERT(static_cast<float>(INT16_MAX) >= stepsToAngle);
    TEST_ASSERT(static_cast<float>(INT16_MIN) <= stepsToAngle);

    encodersTest.setCountsLeft(0);
    encodersTest.setCountsRight(0);
    odometry.clearPosition();
    odometry.clearMileage();
    odometry.setOrientation(0);
    TEST_ASSERT_EQUAL_INT32(0, odometry.getOrientation());

    encodersTest.setCountsLeft(stepsLeft);
    encodersTest.setCountsRight(stepsRight);
    odometry.process();

    /* Standstill detection has debouncing. */
    delay(10U);
    odometry.process();

    if (0.001 > fabs(angle))
    {
        TEST_ASSERT_TRUE(odometry.isStandStill());
    }
    else
    {
        TEST_ASSERT_FALSE(odometry.isStandStill());
    }

    TEST_ASSERT_INT32_WITHIN(static_cast<int32_t>(angleEpsilon * 1000.0f), static_cast<int32_t>(fabs(angle) * 1000.0f),
                             odometry.getOrientation());
}

static void testDriveDistance(float distance)
{
    IEncodersTest& encodersTest    = Board::getInstance().getEncodersTest();
    Odometry&      odometry        = Odometry::getInstance();
    int32_t        posX            = 0;
    int32_t        posY            = 0;
    float          stepsToDistance = calcStepsToDistance(distance);

    encodersTest.setCountsLeft(0);
    encodersTest.setCountsRight(0);
    odometry.clearPosition();
    odometry.clearMileage();
    odometry.setOrientation(0);
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, odometry.getOrientation());
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(0, posY);

    encodersTest.setCountsLeft(stepsToDistance);
    encodersTest.setCountsRight(stepsToDistance);
    odometry.process();

    /* Standstill detection has debouncing. */
    delay(10U);
    odometry.process();

    if (0.001f > fabs(distance))
    {
        TEST_ASSERT_TRUE(odometry.isStandStill());
    }
    else
    {
        TEST_ASSERT_FALSE(odometry.isStandStill());
    }

    if (0.0F <= distance)
    {
        distance += 0.5F;
    }
    else
    {
        distance -= 0.5F;
    }

    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(fabs(distance)), odometry.getMileageCenter());

    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(static_cast<int32_t>(distance), posX);
    TEST_ASSERT_EQUAL_INT32(0, posY);
}
