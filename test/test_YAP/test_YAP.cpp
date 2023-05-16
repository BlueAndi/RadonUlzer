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
 * @author  Gabryel Reyes <gabryelrdiaz@gmail.com>
 * @brief   This module contains the YAP Server tests.
 */

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <unity.h>
#include <TestStream.h>
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

static void testChannelCallback(const uint8_t* payload, uint8_t payloadSize);
static void testCmdSync();
static void testCmdSyncRsp();
static void testCmdScrb();
static void testCmdScrbRsp();
static void testChannelCreation();
static void testDataSend();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

static uint8_t       emptyOutputBuffer[MAX_FRAME_LEN];
static TestStream    gTestStream;
static const uint8_t controlChannelFrameLength = (HEADER_LEN + CONTROL_CHANNEL_PAYLOAD_LENGTH);
static const uint8_t testPayload[4U]           = {0x12, 0x34, 0x56, 0x78};
static bool          callbackCalled            = false;

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
    memset(emptyOutputBuffer, 0xA5, sizeof(emptyOutputBuffer));
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

    RUN_TEST(testCmdSync);
    RUN_TEST(testCmdSyncRsp);
    RUN_TEST(testCmdScrb);
    RUN_TEST(testCmdScrbRsp);
    RUN_TEST(testChannelCreation);
    RUN_TEST(testDataSend);

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
 * Callback for incoming data from test channel.
 * @param[in] payload Byte buffer containing incomming data.
 * @param[in] payloadSize Number of bytes received.
 */
static void testChannelCallback(const uint8_t* payload, uint8_t payloadSize)
{
    callbackCalled = true;
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testPayload, payload, payloadSize);
}

/**
 * Test SYNC Command of YAP Server.
 */
static void testCmdSync()
{
    YAPServer<2U> testYapServer(gTestStream);
    uint8_t       expectedOutputBufferVector[6U][MAX_FRAME_LEN] = {
        {0x00, 0x0F, 0xFA, 0x00, 0x00, 0x00, 0x03, 0xE8}, /* SYNC 1000ms*/
        {0x00, 0x0F, 0xE6, 0x00, 0x00, 0x00, 0x07, 0xD0}, /* SYNC 2000ms*/
        {0x00, 0x0F, 0x82, 0x00, 0x00, 0x00, 0x1B, 0x58}, /* SYNC 7000ms*/
        {0x00, 0x0F, 0x1E, 0x00, 0x00, 0x00, 0x2E, 0xE0}, /* SYNC 12000ms*/
        {0x00, 0x0F, 0xB9, 0x00, 0x00, 0x00, 0x42, 0x68}  /* SYNC 17000ms*/
    };
    uint8_t inputQueueVector[2U][MAX_FRAME_LEN] = {{0x00, 0x0F, 0xE7, 0x01, 0x00, 0x00, 0x07, 0xD0},
                                                   {0x00, 0x0F, 0x83, 0x01, 0x00, 0x00, 0x1B, 0x58}};

    /*
     * Case: Unsynced Heartbeat.
     */

    gTestStream.flushOutputBuffer();

    /* Unsynced Heartbeat at 0 milliseconds */
    testYapServer.process(0U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, controlChannelFrameLength);
    gTestStream.flushOutputBuffer();

    /* Unsynced Heartbeat after 1000 milliseconds */
    testYapServer.process(1000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0U], gTestStream.m_outputBuffer,
                                  controlChannelFrameLength);
    gTestStream.flushOutputBuffer();

    /* No Heartbeat expected */
    testYapServer.process(1500U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, controlChannelFrameLength);
    gTestStream.flushOutputBuffer();

    /* Unsynced Heartbeat after 2000 milliseconds */
    testYapServer.process(2000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[1U], gTestStream.m_outputBuffer,
                                  controlChannelFrameLength);
    gTestStream.flushOutputBuffer();

    /*
     * Case: Sync
     */

    /* Put Data in RX Queue */
    gTestStream.pushToQueue(inputQueueVector[0], controlChannelFrameLength);

    /* Two process calls required */
    testYapServer.process(2500U); /* Read Frame Header */
    testYapServer.process(2700U); /* Read Frame Payload */
    TEST_ASSERT_TRUE(testYapServer.isSynced());

    /* No output expected */
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, controlChannelFrameLength);
    gTestStream.flushInputBuffer();

    /*
     * Case: Synced Heartbeat.
     * Last Sync = 2000 ms
     * isSynced = true
     * Next Sync = 7000 ms
     */

    /* No output expected. Would be Heartbeat if Unsynced. */
    testYapServer.process(3000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, controlChannelFrameLength);

    /* Synced Heartbeat. */
    testYapServer.process(7000U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[2U], gTestStream.m_outputBuffer,
                                  controlChannelFrameLength);
    gTestStream.flushOutputBuffer();

    /**
     * Case: Maintain Sync
     */

    /* Put SYNC_RSP in RX Queue. Otherwise will fall out of Sync. */
    gTestStream.pushToQueue(inputQueueVector[1], controlChannelFrameLength);

    /* Two process calls required */
    testYapServer.process(9000U);  /* Read Frame Header */
    testYapServer.process(11000U); /* Read Frame Payload */
    TEST_ASSERT_TRUE(testYapServer.isSynced());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, controlChannelFrameLength);

    /* Synced Heartbeat */
    testYapServer.process(12000U);
    TEST_ASSERT_TRUE(testYapServer.isSynced());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[3U], gTestStream.m_outputBuffer,
                                  controlChannelFrameLength);
    gTestStream.flushInputBuffer();
    gTestStream.flushOutputBuffer();

    /**
     * Case: Fall out of Sync.
     * No data passed to RX Queue.
     */

    /* Synced Heartbeat. Fall out fo sync, as last Heartbeat was not Acknowledged. */
    testYapServer.process(17000U);
    TEST_ASSERT_FALSE(testYapServer.isSynced());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[4U], gTestStream.m_outputBuffer,
                                  controlChannelFrameLength);
    gTestStream.flushOutputBuffer();
}

/**
 * Test SYNC_RSP Command of YAP Server.
 */
static void testCmdSyncRsp()
{
    YAPServer<2U> testYapServer(gTestStream);
    uint8_t       testTime                                           = 0U;
    uint8_t       numberOfCases                                      = 3U;
    uint8_t expectedOutputBufferVector[numberOfCases][MAX_FRAME_LEN] = {{0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00},
                                                                        {0x00, 0x16, 0x01, 0x12, 0x34, 0x56, 0x78},
                                                                        {0x00, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0xFF}};
    uint8_t inputQueueVector[numberOfCases][MAX_FRAME_LEN]           = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                                        {0x00, 0x15, 0x00, 0x12, 0x34, 0x56, 0x78},
                                                                        {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF}};

    /* Ignore SYNC */
    testYapServer.process(testTime++);
    gTestStream.flushOutputBuffer();

    /* Test for SYNC_RSP */
    for (uint8_t testCase = 0; testCase < numberOfCases; testCase++)
    {
        gTestStream.pushToQueue(inputQueueVector[testCase], controlChannelFrameLength);
        testYapServer.process(testTime++);
        testYapServer.process(testTime++);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[testCase], gTestStream.m_outputBuffer,
                                      controlChannelFrameLength);
        gTestStream.flushInputBuffer();
        gTestStream.flushOutputBuffer();
    }
}

/**
 * Test SCRB Command of YAP Server.
 */
static void testCmdScrb()
{
    YAPServer<2U> testYapServer(gTestStream);
    uint8_t       testTime                                                 = 0U;
    uint8_t       numberOfCases                                            = 2U;
    uint8_t       expectedOutputBufferVector[numberOfCases][MAX_FRAME_LEN] = {
        {0x00, 0x44, 0x03, 0x00, 0x00, 'T', 'E', 'S', 'T'}, {0x00, 0x4D, 0x03, 0x01, 0x08, 'T', 'E', 'S', 'T'}};
    uint8_t inputQueueVector[numberOfCases][MAX_FRAME_LEN] = {
        {0x00, 0x43, 0x02, 'T', 'E', 'S', 'T', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

    /* Ignore SYNC */
    testYapServer.process(testTime++);
    gTestStream.flushOutputBuffer();

    /*
     * Case: Suscribe to Unknown Channel
     */
    gTestStream.pushToQueue(inputQueueVector[0], controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0], gTestStream.m_outputBuffer, controlChannelFrameLength);
    gTestStream.flushOutputBuffer();
    gTestStream.flushInputBuffer();

    /*
     * Case: Subscribe to Known Channel
     */
    TEST_ASSERT_EQUAL_UINT8(1U, testYapServer.createChannel("TEST", 8U));

    gTestStream.pushToQueue(inputQueueVector[0], controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[1], gTestStream.m_outputBuffer, controlChannelFrameLength);
    gTestStream.flushOutputBuffer();
    gTestStream.flushInputBuffer();

    /*
     * Case: Subscribe to a Duplicate Channel
     */
    TEST_ASSERT_EQUAL_UINT8(2U, testYapServer.createChannel("TEST", 8U));

    gTestStream.pushToQueue(inputQueueVector[0], controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[1], gTestStream.m_outputBuffer, controlChannelFrameLength);
    gTestStream.flushOutputBuffer();
    gTestStream.flushInputBuffer();
}

/**
 * Test SCRB_RSP Command of YAP Server.
 */
static void testCmdScrbRsp()
{
    YAPServer<2U> testYapServer(gTestStream);
    uint8_t       testTime                                                 = 0U;
    uint8_t       numberOfCases                                            = 3U;
    uint8_t       expectedOutputBufferVector[numberOfCases][MAX_FRAME_LEN] = {
        {0x00, 0x43, 0x02, 'T', 'E', 'S', 'T', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    uint8_t inputQueueVector[numberOfCases][MAX_FRAME_LEN] = {{0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00},
                                                              {0x00, 0x44, 0x03, 0x00, 0x00, 'T', 'E', 'S', 'T'},
                                                              {0x00, 0x4D, 0x03, 0x01, 0x08, 'T', 'E', 'S', 'T'}};

    /* Sync */
    gTestStream.pushToQueue(inputQueueVector[0U], controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_TRUE(testYapServer.isSynced());

    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.getNumberOfRxChannels());

    /*
     * Case: Suscribe to Unknown Channel
     */
    gTestStream.pushToQueue(inputQueueVector[1U], controlChannelFrameLength);

    testYapServer.subscribeToChannel("TEST", testChannelCallback);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0], gTestStream.m_outputBuffer, controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.getNumberOfRxChannels());
    gTestStream.flushInputBuffer();
    gTestStream.flushOutputBuffer();

    /*
     * Case: Suscribe to Known Channel
     */
    gTestStream.pushToQueue(inputQueueVector[2U], controlChannelFrameLength);

    testYapServer.subscribeToChannel("TEST", testChannelCallback);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0], gTestStream.m_outputBuffer, controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8(1U, testYapServer.getNumberOfRxChannels());
    gTestStream.flushInputBuffer();
    gTestStream.flushOutputBuffer();

    /*
     * Case: Suscribe again to Known Channel
     */
    gTestStream.pushToQueue(inputQueueVector[2U], controlChannelFrameLength);

    testYapServer.subscribeToChannel("TEST", testChannelCallback);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0], gTestStream.m_outputBuffer, controlChannelFrameLength);
    testYapServer.process(testTime++);
    testYapServer.process(testTime++);
    TEST_ASSERT_EQUAL_UINT8(1U, testYapServer.getNumberOfRxChannels());
    gTestStream.flushInputBuffer();
    gTestStream.flushOutputBuffer();
}

/**
 * Test Channel Creation on a YAP Server.
 */
static void testChannelCreation()
{
    const uint8_t          maxChannels = 5U;
    YAPServer<maxChannels> testYapServer(gTestStream);

    /* No Channels Configured on Start */
    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.getNumberOfTxChannels());

    /*
     * Case: Try to configure invalid channels.
     */

    /* Channel Name is empty */
    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.createChannel("", 1U));
    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.getNumberOfTxChannels());

    /* DLC = 0U */
    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.createChannel("TEST", 0U));
    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.getNumberOfTxChannels());

    /*
     * Case: Configure maximum valid channels.
     */
    for (uint8_t channelNumber = 0; channelNumber < maxChannels; channelNumber++)
    {
        TEST_ASSERT_EQUAL_UINT8(channelNumber, testYapServer.getNumberOfTxChannels());
        TEST_ASSERT_EQUAL_UINT8((channelNumber + 1U), testYapServer.createChannel("TEST", 1U));
        TEST_ASSERT_EQUAL_UINT8((channelNumber + 1U), testYapServer.getNumberOfTxChannels());
    }

    /*
     * Case: Try to configure more than the maximum number of channels.
     */

    TEST_ASSERT_EQUAL_UINT8(0U, testYapServer.createChannel("TEST", 1U));
    TEST_ASSERT_EQUAL_UINT8(maxChannels, testYapServer.getNumberOfTxChannels());
}

/**
 * Test data send on YAP Server.
 */
static void testDataSend()
{
    YAPServer<1U> testYapServer(gTestStream);
    uint8_t       expectedOutputBufferVector[1U][MAX_FRAME_LEN] = {{0x01, 0x16, 0x12, 0x34, 0x56, 0x78}};
    uint8_t       inputQueueVector[1U][MAX_FRAME_LEN]           = {{0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00}};

    /* Flush Stream */
    gTestStream.flushInputBuffer();
    gTestStream.flushOutputBuffer();

    /*
     * Case: Send on non-existent channel while unsynced.
     */
    testYapServer.sendData("TEST", testPayload, sizeof(testPayload));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, sizeof(testPayload));

    /* Create a Channel */
    TEST_ASSERT_EQUAL_UINT8(1U, testYapServer.createChannel("TEST", sizeof(testPayload)));

    /*
     * Case: Send on existent channel while unsynced.
     */
    testYapServer.sendData("TEST", testPayload, sizeof(testPayload));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, sizeof(testPayload));

    /* Sync */
    gTestStream.pushToQueue(inputQueueVector[0U], controlChannelFrameLength);
    testYapServer.process(0U);
    testYapServer.process(1U);
    TEST_ASSERT_TRUE(testYapServer.isSynced());

    /*
     * Case: Send on non-existent channel.
     */
    testYapServer.sendData("HELLO", testPayload, sizeof(testPayload));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(emptyOutputBuffer, gTestStream.m_outputBuffer, sizeof(testPayload));

    /*
     * Case: Send on existent channel.
     */
    testYapServer.sendData("TEST", testPayload, sizeof(testPayload));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedOutputBufferVector[0U], gTestStream.m_outputBuffer, sizeof(testPayload));
}
