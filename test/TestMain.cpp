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
#include <SimpleTimer.h>
#include <PIDController.h>
#include <Util.h>

#include <Board.h>
#include <Odometry.h>
#include <RobotConstants.h>
#include <FPMath.h>
#include "TestStream.h"
#include <YAPServer.hpp>

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
static void testOdometry();
static void YAPTests();
static void testYAPSync();
static void testYAPSyncRsp();

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
    RUN_TEST(testOdometry);
    YAPTests();

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
    int16_t  testVectorSteps[] = {8, 16, 32, 64, 128, 64, 32, 16, 8, 0, -8, -16, -32, -64, -128};
    uint32_t mileageSteps      = 0;
    int32_t  circumference     = RobotConstants::WHEEL_BASE * FP_2PI() / 2; /* [mm] */
    int32_t  angleDeg          = 0;                                         /* [deg] */
    int32_t  angleMRad         = 0;                                         /* [mrad] */
    int32_t  epsilonMRad       = FP_2PI() / 360;                            /* [mrad] */
    int32_t  epsilonPos        = 1;                                         /* [mm] */

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

        TEST_ASSERT_EQUAL_UINT32(mileageSteps / RobotConstants::ENCODER_STEPS_PER_MM, odometry.getMileageCenter());
        TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
        odometry.getPosition(posX, posY);
        TEST_ASSERT_INT32_WITHIN(epsilonPos, 0, posX);
        TEST_ASSERT_INT32_WITHIN(epsilonPos,
                                 static_cast<int32_t>(testVectorSteps[idx]) /
                                     static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM),
                                 posY);
        TEST_ASSERT_FALSE(odometry.isStandStill());

        ++idx;
    }

    /* Verify wrap around in forward direction. Drive 2 mm forward. */
    encodersTest.setCountsLeft(INT16_MAX - static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MAX - static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    odometry.setOrientation(FP_PI() / 2);
    odometry.clearPosition();
    odometry.clearMileage();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    encodersTest.setCountsLeft(INT16_MIN + static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MIN + static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(2, odometry.getMileageCenter());
    TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(2, posY);
    TEST_ASSERT_FALSE(odometry.isStandStill());

    /* Verify wrap around in backward direction. Drive 2 mm backward. */
    encodersTest.setCountsLeft(INT16_MIN + static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MIN + static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    odometry.setOrientation(FP_PI() / 2);
    odometry.clearPosition();
    odometry.clearMileage();
    TEST_ASSERT_EQUAL_UINT32(0, odometry.getMileageCenter());
    encodersTest.setCountsLeft(INT16_MAX - static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    encodersTest.setCountsRight(INT16_MAX - static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM));
    odometry.process();
    TEST_ASSERT_EQUAL_UINT32(2, odometry.getMileageCenter());
    TEST_ASSERT_EQUAL_INT16(FP_PI() / 2, odometry.getOrientation());
    odometry.getPosition(posX, posY);
    TEST_ASSERT_EQUAL_INT32(0, posX);
    TEST_ASSERT_EQUAL_INT32(-2, posY);
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

/**
 * Test functionalities of YAP Server.
 */
static void YAPTests()
{
    RUN_TEST(testYAPSync);
    RUN_TEST(testYAPSyncRsp);
}

/**
 * Test SYNC functionality of YAP Server.
 */
static void testYAPSync()
{
    TestStream_   TestStream;
    YAPServer<2U> testYapServer(TestStream);
    uint8_t emptyOutputBuffer[MAX_FRAME_LEN] = {0x00};
    uint8_t expectedOutputBufferVector[6U][MAX_FRAME_LEN] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, /* SYNC 0ms*/
        {0x00, 0xEB, 0x00, 0x00, 0x00, 0x03, 0xE8}, /* SYNC 1000ms*/
        {0x00, 0xD7, 0x00, 0x00, 0x00, 0x07, 0xD0}, /* SYNC 2000ms*/
        {0x00, 0x73, 0x00, 0x00, 0x00, 0x1B, 0x58}, /* SYNC 7000ms*/
        {0x00, 0x0F, 0x00, 0x00, 0x00, 0x2E, 0xE0}, /* SYNC 12000ms*/
        {0x00, 0xAA, 0x00, 0x00, 0x00, 0x42, 0x68}  /* SYNC 17000ms*/
    };
    uint8_t inputReceiveBufferVector[2U][MAX_FRAME_LEN] = {
        {0x00, 0xD8, 0x01, 0x00, 0x00, 0x07, 0xD0},
        {0x00, 0x74, 0x01, 0x00, 0x00, 0x1B, 0x58}
    };

    /*
     * Case: Unsynced Heartbeat.
     */

    /* Unsynced Heartbeat at 0 milliseconds */
    testYapServer.process(0U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0U], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushOutputBuffer();

    /* Unsynced Heartbeat after 1000 milliseconds */
    testYapServer.process(1000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[1U], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushOutputBuffer();

    /* No Heartbeat expected */
    testYapServer.process(1500U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushOutputBuffer();

    /* Unsynced Heartbeat after 2000 milliseconds */
    testYapServer.process(2000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[2U], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushOutputBuffer();

    /*
     * Case: Sync
     */

    /* Put Data in RX Queue */
    TestStream.pushToQueue(inputReceiveBufferVector[0], HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);

    /* Two process calls required */
    testYapServer.process(2500U); /* Read Frame Header */
    testYapServer.process(2700U); /* Read Frame Payload */
    TEST_ASSERT_TRUE(testYapServer.isSynced());

    /* No output expected */
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushInputBuffer();

    /*
     * Case: Synced Heartbeat.
     * Last Sync = 2000 ms
     * isSynced = true
     * Next Sync = 7000 ms
     */

    /* No output expected. Would be Heartbeat if Unsynced. */
    testYapServer.process(3000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);

    /* Synced Heartbeat. */
    testYapServer.process(7000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[3U], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushOutputBuffer();

    /**
     * Case: Maintain Sync
     */

    /* Put SYNC_RSP in RX Queue. Otherwise will fall out of Sync. */
    TestStream.pushToQueue(inputReceiveBufferVector[1], HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);

    /* Two process calls required */
    testYapServer.process(9000U);  /* Read Frame Header */
    testYapServer.process(11000U); /* Read Frame Payload */
    TEST_ASSERT_TRUE(testYapServer.isSynced());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);

    /* Synced Heartbeat */
    testYapServer.process(12000U);
    TEST_ASSERT_TRUE(testYapServer.isSynced());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[4U], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushInputBuffer();
    TestStream.flushOutputBuffer();

    /**
     * Case: Fall out of Sync.
     * No data passed to RX Queue.
     */

    /* Synced Heartbeat. Fall out fo sync, as last Heartbeat was not Acknowledged. */
    testYapServer.process(17000U);
    TEST_ASSERT_FALSE(testYapServer.isSynced());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[5U], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
    TestStream.flushOutputBuffer();
}

/**
 * Test SYNC_RSP Command of YAP Server.
 */
static void testYAPSyncRsp()
{
    TestStream_   TestStream;
    YAPServer<2U> testYapServer(TestStream);
    uint8_t       testTime                                                 = 0U;
    uint8_t       numberOfCases                                            = 3U;
    uint8_t       emptyOutputBuffer[MAX_FRAME_LEN]                         = {0x00};
    uint8_t       expectedOutputBufferVector[numberOfCases][MAX_FRAME_LEN] = {
        {0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x16, 0x01, 0x12, 0x34, 0x56, 0x78},
        {0x00, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0xFF}
    };
    uint8_t       inputReceiveBufferVector[numberOfCases][MAX_FRAME_LEN]   = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x15, 0x00, 0x12, 0x34, 0x56, 0x78},
        {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF}
    };

    /* Ignore SYNC */
    testYapServer.process(testTime++);
    TestStream.flushOutputBuffer();

    /* Test for SYNC_RSP */
    for (uint8_t testCase = 0; testCase < numberOfCases; testCase++)
    {
        TestStream.pushToQueue(inputReceiveBufferVector[testCase], HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
        testYapServer.process(testTime++);
        testYapServer.process(testTime++);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[testCase], TestStream.m_outputBuffer, HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
        TestStream.flushInputBuffer();
        TestStream.flushOutputBuffer();
    }
}
        TestStream.flushOutputBuffer();
    }
}
