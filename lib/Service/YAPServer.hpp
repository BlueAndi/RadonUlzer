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
 * @brief  Yet Another Protocol (YAP) Server
 * @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef YAP_SERVER_H
#define YAP_SERVER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <YAPCommon.hpp>
#include <Stream.h>
#include <string.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Class for the YAP Server.
 * @tparam tMaxChannels Maximum number of channels
 */
template<uint8_t tMaxChannels>
class YAPServer
{
public:
    /**
     * Construct the YAP Server.
     */
    YAPServer(Stream& stream) :
        m_isSynced(false),
        m_isSubscriberReady(false),
        m_lastSyncCommand(0U),
        m_lastSyncResponse(0U),
        m_stream(stream),
        m_receiveFrame(),
        m_receivedBytes(0U),
        m_rxAttempts(0U),
        m_numberOfTxChannels(0U),
        m_numberOfRxChannels(0U),
        m_numberOfPendingChannels(0U)
    {
    }

    /**
     * Destroy the YAP Server.
     */
    ~YAPServer()
    {
    }

    /**
     * Manage the Server functions.
     * Call this function cyclic.
     * @param[in] currentTimestamp Time in milliseconds.
     */
    void process(const uint32_t currentTimestamp)
    {
        /* Periodic Heartbeat */
        heartbeat(currentTimestamp);

        /* Process RX data */
        processRxData();

        /* Manage Pending Subscriptions. */
        managePendingSubscriptions();
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channelNumber Channel to send frame to.
     * @param[in] payload Byte buffer to be sent.
     * @param[in] payloadSize Amount of bytes to send.
     * @returns If payload succesfully sent, returns true. Otherwise, false.
     */
    bool sendData(uint8_t channelNumber, const uint8_t* payload, uint8_t payloadSize) const
    {
        bool isSent = false;

        if ((CONTROL_CHANNEL_NUMBER != channelNumber) &&
            (nullptr != payload) &&
            (true == m_isSubscriberReady) &&
            (true == m_isSynced))
        {
            isSent = send(channelNumber, payload, payloadSize);
        }

        return isSent;
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channelName Channel to send frame to.
     * @param[in] payload Byte buffer to be sent.
     * @param[in] payloadSize Amount of bytes to send.
     * @returns If payload succesfully sent, returns true. Otherwise, false.
     */
    bool sendData(const char* channelName, const uint8_t* payload, uint8_t payloadSize) const
    {
        bool isSent = false;

        if (nullptr != channelName)
        {
            isSent = sendData(getTxChannelNumber(channelName), payload, payloadSize);
        }

        return isSent;
    }

    /**
     * Get Number of a TX channel by its name.
     * @param[in] channelName Name of Channel
     * @returns Number of the Channel, or 0 if not channel with the name is present.
     */
    uint8_t getTxChannelNumber(const char* channelName) const
    {
        uint8_t idx = tMaxChannels;

        if (nullptr != channelName)
        {
            for (idx = 0U; idx < tMaxChannels; idx++)
            {
                if (0U == strncmp(channelName, m_txChannels[idx].m_name, CHANNEL_NAME_MAX_LEN))
                {
                    break;
                }
            }
        }

        return (idx == tMaxChannels) ? 0U : (idx + 1U);
    }

    /**
     * Creates a new TX Channel on the server.
     * @param[in] channelName Name of the channel.
     * It will not be checked if the name already exists.
     * @param[in] dlc Length of the payload of this channel.
     * @returns The channel number if succesfully created, or 0 if not able to create new channel.
     */
    uint8_t createChannel(const char* channelName, uint8_t dlc)
    {
        /* Using strnlen in case the name is not null-terminated. */
        uint8_t nameLength = strnlen(channelName, CHANNEL_NAME_MAX_LEN);
        uint8_t idx        = 0U;

        if ((nullptr != channelName) && (0U != nameLength) && (MAX_DATA_LEN >= dlc) && (0U != dlc) &&
            (tMaxChannels > m_numberOfTxChannels))
        {
            /*
             * Number of TX Channels corresponds to idx in TX Channel Array
             * as these are ordered and Channels cannot be deleted.
             */
            memcpy(m_txChannels[m_numberOfTxChannels].m_name, channelName, nameLength);
            m_txChannels[m_numberOfTxChannels].m_dlc = dlc;

            /* Increase Channel Counter. */
            m_numberOfTxChannels++;

            /* Provide Channel Number. Could be summarized with operation above. */
            idx = m_numberOfTxChannels;
        }

        return idx;
    }

    /**
     * Suscribe to a Channel to receive the incoming data.
     * @param[in] channelName Name of the Channel to suscribe to.
     * @param[in] callback Callback to return the incoming data.
     */
    void subscribeToChannel(const char* channelName, ChannelCallback callback)
    {
        if ((nullptr != channelName) && (nullptr != callback) && (tMaxChannels > m_numberOfPendingChannels))
        {
            /* Save Name and Callback for channel creation after response */
            /* Using strnlen in case the name is not null-terminated. */
            uint8_t nameLength = strnlen(channelName, CHANNEL_NAME_MAX_LEN);

            /*
             * Number of Pending Channels corresponds to idx in Pending Channel Array
             * as these are ordered and Channels cannot be deleted.
             */
            memcpy(m_pendingSuscribeChannels[m_numberOfPendingChannels].m_name, channelName, nameLength);
            m_pendingSuscribeChannels[m_numberOfPendingChannels].m_callback = callback;

            /* Increase Channel Counter. */
            m_numberOfPendingChannels++;
        }
    }

    /**
     * Returns current Sync state of th YAP Server.
     */
    bool isSynced()
    {
        return m_isSynced;
    }

    /**
     * Get the number of configured TX channels.
     * @returns Number of configured Data Channels. The Command Channel is ignored here.
     */
    uint8_t getNumberOfTxChannels()
    {
        return m_numberOfTxChannels;
    }

    /**
     * Get the number of configured RX channels.
     * @returns Number of configured Data Channels. The Command Channel is ignored here.
     */
    uint8_t getNumberOfRxChannels()
    {
        return m_numberOfRxChannels;
    }

private:
    /**
     * Control Channel Command: SYNC
     * @param[in] payload Incomming Command data
     */
    void cmdSYNC(const uint8_t* payload)
    {
        uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SYNC_RSP, payload[0U], payload[1U], payload[2U],
                                                       payload[3U]};
        /* Ignore return as SYNC_RSP can fail */
        (void) send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf));
    }

    /**
     * Control Channel Command: SYNC_RSP
     * @param[in] payload Incomming Command data
     */
    void cmdSYNC_RSP(const uint8_t* payload)
    {
        uint32_t rcvTimestamp = 0;

        /* Using (payloadSize - 1U) as CMD Byte is not passed. */
        if (true == Util::byteArrayToUint32(payload, sizeof(uint32_t), rcvTimestamp))
        {
            /* Check Timestamp with m_lastSyncCommand */
            if (rcvTimestamp == m_lastSyncCommand)
            {
                m_lastSyncResponse = m_lastSyncCommand;
                m_isSynced         = true;
            }
            else
            {
                m_isSynced = false;
                m_isSubscriberReady = false;
            }
        }
    }

    /**
     * Control Channel Command: SCRB
     * @param[in] payload Incomming Command data
     */
    void cmdSCRB(const uint8_t* payload)
    {
        uint8_t channelNumber = getTxChannelNumber(reinterpret_cast<const char*>(payload));
        uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SCRB_RSP};

        if (CONTROL_CHANNEL_NUMBER != channelNumber)
        {
            buf[1U] = channelNumber;
            buf[2U] = getChannelDLC(channelNumber, true);
        }
        else
        {
            /* No channel found. Invalid Response: Channel 0 with DLC = 0 */
            buf[1U] = 0U;
            buf[2U] = 0U;
        }

        /* Name is always sent back. */
        memcpy(&buf[3U], payload, CHANNEL_NAME_MAX_LEN);

        if (false == send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf)))
        {
            /* Fall out of sync if failed to send. */
            m_isSynced = false;
            m_isSubscriberReady = false;
        }
    }

    /**
     * Control Channel Command: SCRB_RSP
     * @param[in] payload Incomming Command data
     */
    void cmdSCRB_RSP(const uint8_t* payload)
    {
        uint8_t        channelNumber = payload[0U];
        uint8_t        channelDLC    = payload[1U];
        const uint8_t* channelName   = &payload[2U];

        if ((tMaxChannels >= channelNumber) && (nullptr != channelName) && (0U < m_numberOfPendingChannels))
        {
            for (uint8_t idx = 0; idx < tMaxChannels; idx++)
            {
                /* Check if a SCRB is pending. */
                if (nullptr != m_pendingSuscribeChannels[idx].m_callback)
                {
                    /* Check if its the correct channel. */
                    if (0U == strncmp(reinterpret_cast<const char*>(channelName), m_pendingSuscribeChannels[idx].m_name,
                                      CHANNEL_NAME_MAX_LEN))
                    {
                        /* Channel is found in the Server. */
                        if ((0U != channelNumber) && (0U != channelDLC))
                        {
                            uint8_t channelArrayIndex = (channelNumber - 1U);

                            /* Channel is empty. Increase Counter*/
                            if (nullptr == m_rxChannels[channelArrayIndex].m_callback)
                            {
                                /* Increase RX Channel Counter. */
                                m_numberOfRxChannels++;
                            }

                            /* Set Channel in RxChannels Array. */
                            memcpy(m_rxChannels[channelArrayIndex].m_name, m_pendingSuscribeChannels[idx].m_name,
                                   CHANNEL_NAME_MAX_LEN);
                            m_rxChannels[channelArrayIndex].m_dlc      = channelDLC;
                            m_rxChannels[channelArrayIndex].m_callback = m_pendingSuscribeChannels[idx].m_callback;
                        }

                        /* Channel is no longer pending. */
                        memset(m_pendingSuscribeChannels[idx].m_name, 0U, CHANNEL_NAME_MAX_LEN);
                        m_pendingSuscribeChannels[idx].m_callback = nullptr;

                        /* Decrease Pending Channel Counter. */
                        m_numberOfPendingChannels--;
                        break;
                    }
                }
            }
        }
    }

    /**
     * Control Channel Command: RDY
     * @param[in] payload Incomming Command data
     */
    void cmdRDY(const uint8_t* payload)
    {
        m_isSubscriberReady = true;
    }

    /**
     * Callback for the Control Channel
     * @param[in] payload Payload of received frame.
     * @param[in] payloadSize Length of Payload
     */
    void callbackControlChannel(const uint8_t* payload, const uint8_t payloadSize)
    {
        if ((nullptr == payload) || (CONTROL_CHANNEL_PAYLOAD_LENGTH != payloadSize))
        {
            return;
        }

        uint8_t        cmdByte = payload[CONTROL_CHANNEL_COMMAND_INDEX];
        const uint8_t* cmdData = &payload[CONTROL_CHANNEL_PAYLOAD_INDEX];

        switch (cmdByte)
        {

        case COMMANDS::SYNC:
            cmdSYNC(cmdData);
            break;

        case COMMANDS::SYNC_RSP:
            cmdSYNC_RSP(cmdData);
            break;

        case COMMANDS::SCRB:
            cmdSCRB(cmdData);
            break;

        case COMMANDS::SCRB_RSP:
            cmdSCRB_RSP(cmdData);
            break;

        case COMMANDS::RDY:
            cmdRDY(cmdData);
            break;

        default:
            break;
        }
    }

    /**
     * Receive and process RX Data.
     */
    void processRxData()
    {
        uint8_t expectedBytes = 0;
        uint8_t dlc           = 0;

        /* Determine how many bytes to read. */
        if (HEADER_LEN > m_receivedBytes)
        {
            /* Header must be read. */
            expectedBytes = (HEADER_LEN - m_receivedBytes);
        }
        else
        {
            /* Header has been read. Get DLC of Rx Channel using Header. */
            dlc = getChannelDLC(m_receiveFrame.fields.header.headerFields.m_channel, false);

            /* DLC = 0 means that the channel does not exist. */
            if ((0U != dlc) && (MAX_RX_ATTEMPTS >= m_rxAttempts))
            {
                expectedBytes = (dlc - (m_receivedBytes - HEADER_LEN));
                m_rxAttempts++;
            }
        }

        /* Are we expecting to read anything? */
        if (0U != expectedBytes)
        {
            /* Read the required amount of bytes, if available. */
            if (expectedBytes <= m_stream.available())
            {
                m_receivedBytes += m_stream.readBytes(&m_receiveFrame.raw[m_receivedBytes], expectedBytes);
            }

            /* Frame has been received. */
            if ((0U != dlc) && ((HEADER_LEN + dlc) == m_receivedBytes))
            {
                if (true == isFrameValid(m_receiveFrame))
                {
                    uint8_t channelArrayIndex = (m_receiveFrame.fields.header.headerFields.m_channel - 1U);

                    /* Differenciate between Control and Data Channels. */
                    if (CONTROL_CHANNEL_NUMBER == m_receiveFrame.fields.header.headerFields.m_channel)
                    {
                        callbackControlChannel(m_receiveFrame.fields.payload.m_data, CONTROL_CHANNEL_PAYLOAD_LENGTH);
                    }
                    else if (nullptr != m_rxChannels[channelArrayIndex].m_callback)
                    {
                        /* Callback */
                        m_rxChannels[channelArrayIndex].m_callback(m_receiveFrame.fields.payload.m_data, dlc);
                    }
                }

                /* Frame received. Cleaning! */
                clearLocalRxBuffers();
            }
        }
        else
        {
            /* Invalid header. Delete Frame. */
            clearLocalRxBuffers();
        }
    }

    /**
     * Clear RX Buffer and counters.
     */
    void clearLocalRxBuffers()
    {
        memset(m_receiveFrame.raw, 0U, MAX_FRAME_LEN);
        m_receivedBytes = 0U;
        m_rxAttempts    = 0U;
    }

    /**
     * Periodic heartbeat.
     * Sends SYNC Command depending on the current Sync state.
     * @param[in] currentTimestamp Time in milliseconds.
     */
    void heartbeat(const uint32_t currentTimestamp)
    {
        uint32_t heartbeatPeriod = HEATBEAT_PERIOD_UNSYNCED;

        if (true == m_isSynced)
        {
            heartbeatPeriod = HEATBEAT_PERIOD_SYNCED;
        }

        if ((currentTimestamp - m_lastSyncCommand) >= heartbeatPeriod)
        {
            /* Timeout. */
            if (m_lastSyncCommand != m_lastSyncResponse)
            {
                m_isSynced = false;
                m_isSubscriberReady = false;
            }

            /* Send SYNC Command. */
            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SYNC};

            /* Using (sizeof(buf) - 1U) as CMD Byte is not passed. */
            Util::uint32ToByteArray(&buf[CONTROL_CHANNEL_PAYLOAD_INDEX], (sizeof(buf) - 1), currentTimestamp);

            if (true == send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf)))
            {
                m_lastSyncCommand = currentTimestamp;
            }
        }
    }

    /**
     * Subscribe to any pending Channels if synced to server.
     */
    void managePendingSubscriptions()
    {
        if ((true == m_isSynced) && (0U < m_numberOfPendingChannels))
        {
            for (uint8_t idx = 0; idx < tMaxChannels; idx++)
            {
                if (nullptr != m_pendingSuscribeChannels[idx].m_callback)
                {
                    /* Suscribe to channel. */
                    uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SCRB};
                    memcpy(&buf[CONTROL_CHANNEL_PAYLOAD_INDEX], m_pendingSuscribeChannels[idx].m_name,
                           CHANNEL_NAME_MAX_LEN);

                    if (false == send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf)))
                    {
                        /* Out-of-Sync on failed send. */
                        m_isSynced = false;
                        m_isSubscriberReady = false;
                        break;
                    }
                }
            }
        }
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channelNumber Channel to send frame to.
     * @param[in] payload Byte buffer to be sent.
     * @param[in] payloadSize Amount of bytes to send.
     * @returns If payload succesfully sent, returns true. Otherwise, false.
     */
    bool send(uint8_t channelNumber, const uint8_t* payload, uint8_t payloadSize) const
    {
        bool    frameSent  = false;
        uint8_t channelDLC = getChannelDLC(channelNumber, true);

        if ((nullptr != payload) && (channelDLC == payloadSize) &&
            (true == m_isSynced || (CONTROL_CHANNEL_NUMBER == channelNumber)))
        {
            const uint8_t frameLength  = HEADER_LEN + channelDLC;
            uint8_t       writtenBytes = 0;
            Frame         newFrame;

            newFrame.fields.header.headerFields.m_channel = channelNumber;
            memcpy(newFrame.fields.payload.m_data, payload, channelDLC);
            newFrame.fields.header.headerFields.m_checksum = checksum(newFrame);

            writtenBytes = m_stream.write(newFrame.raw, frameLength);

            if (frameLength == writtenBytes)
            {
                frameSent = true;
            }
        }

        return frameSent;
    }

    /**
     * Check if a Frame is valid using its checksum.
     * @param[in] frame Frame to be checked.
     * @returns true if the Frame's checksum is correct.
     */
    bool isFrameValid(const Frame& frame)
    {
        /* Frame is valid when both checksums are the same. */
        return (checksum(frame) == frame.fields.header.headerFields.m_checksum);
    }

    /**
     * Get the Payload Length of a channel
     * @param[in] channel Channel number to check
     * @param[in] isTxChannel Is the Channel a TX Channel? If false, will return value for an RX Channel instead.
     * @returns DLC of the channel, or 0 if channel is not found.
     */
    uint8_t getChannelDLC(uint8_t channel, bool isTxChannel) const
    {
        uint8_t channelDLC = 0U;

        if (CONTROL_CHANNEL_NUMBER == channel)
        {
            channelDLC = CONTROL_CHANNEL_PAYLOAD_LENGTH;
        }
        else if (tMaxChannels >= channel)
        {
            uint8_t channelIdx = channel - 1U;

            if (true == isTxChannel)
            {
                channelDLC = m_txChannels[channelIdx].m_dlc;
            }
            else
            {
                channelDLC = m_txChannels[channelIdx].m_dlc;
            }
        }
        else
        {
            /* Invalid channel, nothing to do. */
            ;
        }

        return channelDLC;
    }

    /**
     * Performs the checksum of a Frame.
     * @param[in] frame Frame to calculate checksum
     * @returns checksum value
     */
    uint8_t checksum(const Frame& frame) const
    {
        uint32_t sum = frame.fields.header.headerFields.m_channel;

        for (size_t idx = 0U; idx < MAX_DATA_LEN; idx++)
        {
            sum += frame.fields.payload.m_data[idx];
        }

        return (sum % UINT8_MAX);
    }

private:
    /**
     * Array of tx Data Channels.
     * Server publishes to these channels.
     */
    Channel m_txChannels[tMaxChannels];

    /**
     * Array of rx Data Channels.
     * Server subscribes to these channels.
     */
    Channel m_rxChannels[tMaxChannels];

    /**
     * Array of pending rx Data Channels.
     * Server subscribes to these channels but no response has been received.
     */
    Channel m_pendingSuscribeChannels[tMaxChannels];

    /**
     * Current Sync state.
     */
    bool m_isSynced;

    /**
     * State of Client.
     */
    bool m_isSubscriberReady;

    /**
     * Last Heartbeat timestamp.
     */
    uint32_t m_lastSyncCommand;

    /**
     * Last sync response timestamp.
     */
    uint32_t m_lastSyncResponse;

    /**
     * Stream for input and output of data.
     */
    Stream& m_stream;

    /**
     * Frame buffer for received Bytes.
     * Allows direct access to Frame Fields once bytes are in the raw buffer.
     */
    Frame m_receiveFrame;

    /**
     * Number of bytes that have been correctly received.
     */
    uint8_t m_receivedBytes;

    /**
     * Number of attempts performed at receiving a Frame.
     */
    uint8_t m_rxAttempts;

    /**
     * Number of TX Channels configured.
     */
    uint8_t m_numberOfTxChannels;

    /**
     * Number of RX Channels configured.
     */
    uint8_t m_numberOfRxChannels;

    /**
     * Number of Pending Channels to subscribe.
     */
    uint8_t m_numberOfPendingChannels;

private:
    /* Not allowed. */
    YAPServer();
    YAPServer(const YAPServer& avg);
    YAPServer& operator=(const YAPServer& avg);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* YAP_SERVER_H */
/** @} */
