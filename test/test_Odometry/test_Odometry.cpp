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

static void testOdometry();

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

    RUN_TEST(testOdometry);

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
 * Test the odometry unit.
 */
static void testOdometry()
{
    IEncodersTest& encodersTest = Board::getInstance().getEncodersTest();
    Odometry&      odometry     = Odometry::getInstance();
    int32_t        posX         = 0;
    int32_t        posY         = 0;
    uint8_t        idx          = 0;
    /* Use multiple of RobotConstants::ENCODER_STEPS_PER_MM to avoid errors caused by divisions. */
    int16_t       testVectorSteps[] = {8, 16, 32, 64, 128, 64, 32, 16, 8, 0, -8, -16, -32, -64, -128};
    uint32_t      mileageSteps      = 0;
    int32_t       circumference     = RobotConstants::WHEEL_BASE * FP_2PI() / 2; /* [mm] */
    int32_t       angleDeg          = 0;                                         /* [deg] */
    int32_t       angleMRad         = 0;                                         /* [mrad] */
    int32_t       epsilonMRad       = FP_2PI() / 360;                            /* [mrad] */
    int32_t       epsilonPos        = 1;                                         /* [mm] */
    const int16_t ODO_CALC_PERIOD =
        static_cast<uint16_t>(10U * RobotConstants::ENCODER_STEPS_PER_MM); /* equal to STEPS_THRESHOLD in [steps] */

    while (2 > idx)
    {
        /* Verify initial values */
        TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
        TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
        odometry.getPosition(posX, posY);
        TEST_ASSERT_EQUAL_INT32(0, posX);
        TEST_ASSERT_EQUAL_INT32(0, posY);
        TEST_ASSERT_TRUE(odometry.isStandStill());

        /* Process once and initial values should be still the same. */
        odometry.process();

        ++idx;
    }

    /* Simulate driving straight forward and backward. */
    idx = 0;
    while ((sizeof(testVectorSteps) / sizeof(testVectorSteps[0])) > idx)
    {
        encodersTest.setCountsLeft(testVectorSteps[idx]);
        encodersTest.setCountsRight(testVectorSteps[idx]);
        odometry.process();

        if (0 == idx)
        {
            mileageSteps = abs(testVectorSteps[idx]);
        }
        else
        {
            mileageSteps += abs(testVectorSteps[idx] - testVectorSteps[idx - 1]);
        }

        TEST_ASSERT_EQUAL_UINT32((mileageSteps / ODO_CALC_PERIOD) / RobotConstants::ENCODER_STEPS_PER_MM,
                                 odometry.getMileageCenter() / ODO_CALC_PERIOD);
        TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
        odometry.getPosition(posX, posY);
        TEST_ASSERT_INT32_WITHIN(epsilonPos, 0, posX);
        TEST_ASSERT_INT32_WITHIN(epsilonPos,
                                 static_cast<int32_t>(testVectorSteps[idx] / ODO_CALC_PERIOD) /
                                     static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM),
                                 posY / ODO_CALC_PERIOD);
        TEST_ASSERT_FALSE(odometry.isStandStill());

        ++idx;
    }

    /* Verify wrap around in forward direction. Drive 20 mm forward. */
    encodersTest.setCountsLeft(INT16_MAX - static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MAX - static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    odometry.setOrientation(FP_PI() / 2);
    odometry.clearPosition();
    odometry.clearMileage();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    encodersTest.setCountsLeft(INT16_MIN + static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MIN + static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(20, odometry.getMileageCenter());
    TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(20, posY);
    TEST_ASSERT_FALSE(odometry.isStandStill());

    /* Verify wrap around in backward direction. Drive 20 mm backward. */
    encodersTest.setCountsLeft(INT16_MIN + static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MIN + static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    odometry.setOrientation(FP_PI() / 2);
    odometry.clearPosition();
    odometry.clearMileage();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    encodersTest.setCountsLeft(INT16_MAX - static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MAX - static_cast<int16_t>(10 * RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(20, odometry.getMileageCenter());
    TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(-20, posY);
    TEST_ASSERT_FALSE(odometry.isStandStill());

    /* Turn left 45° on the same position. */
    encodersTest.setCountsLeft(0);
    encodersTest.setCountsRight(0);
    odometry.process();
    odometry.setOrientation(FP_PI() / 2);
    odometry.clearPosition();
    odometry.clearMileage();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    angleDeg  = 45;
    angleMRad = (FP_PI() / 2) + ((angleDeg * FP_2PI()) / 360); /* mrad */
    angleMRad %= FP_2PI();
    encodersTest.setCountsLeft(-circumference * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM) * angleDeg /
                               360 / 1000);
    encodersTest.setCountsRight(circumference * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM) * angleDeg /
                                360 / 1000);
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    TEST_ASSERT_INT16_WITHIN(epsilonMRad, angleMRad, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(0, posY);
    TEST_ASSERT_FALSE(odometry.isStandStill());

    /* Turn right 90° on the same position. */
    angleDeg  = -90;
    angleMRad = (FP_PI() / 2) + ((angleDeg * FP_2PI()) / 360); /* mrad */
    angleMRad %= FP_2PI();
    encodersTest.setCountsLeft(-circumference * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM) * angleDeg /
                               360 / 1000);
    encodersTest.setCountsRight(circumference * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM) * angleDeg /
                                360 / 1000);
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    TEST_ASSERT_INT16_WITHIN(epsilonMRad, angleMRad, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(0, posY);
    TEST_ASSERT_FALSE(odometry.isStandStill());

    /* Turn left 360 + 90° on the same position. */
    angleDeg  = 450;
    angleMRad = (FP_PI() / 2) + ((angleDeg * FP_2PI()) / 360); /* mrad */
    angleMRad %= FP_2PI();
    encodersTest.setCountsLeft(-circumference * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM) * angleDeg /
                               360 / 1000);
    encodersTest.setCountsRight(circumference * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM) * angleDeg /
                                360 / 1000);
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    TEST_ASSERT_INT16_WITHIN(epsilonMRad, angleMRad, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(0, posY);
    TEST_ASSERT_FALSE(odometry.isStandStill());
}
