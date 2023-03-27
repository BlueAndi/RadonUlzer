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
#include <Arduino.h>
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
        m_lastSyncCommand(0U),
        m_lastSyncResponse(0U),
        m_pendingSuscribeChannel(),
        m_stream(stream)
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
     */
    void process()
    {
        // Periodic Heartbeat
        heartbeat();

        // Process RX data
        processRxData();
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channelNumber Channel to send frame to.
     * @param[in] payload Byte buffer to be sent.
     * @param[in] payloadSize Amount of bytes to send.
     */
    void sendData(uint8_t channelNumber, const uint8_t* payload, uint8_t payloadSize)
    {
        if ((CONTROL_CHANNEL_NUMBER != channelNumber) && (nullptr != payload))
        {
            send(channelNumber, payload, payloadSize);
        }
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channelName Channel to send frame to.
     * @param[in] payload Byte buffer to be sent.
     * @param[in] payloadSize Amount of bytes to send.
     */
    void sendData(const char* channelName, const uint8_t* payload, uint8_t payloadSize)
    {
        if (nullptr != channelName)
        {
            sendData(getChannelNumber(channelName), payload, payloadSize);
        }
    }

    /**
     * Get Number of a channel by its name.
     * @param[in] channelName Name of Channel
     * @returns Number of the Channel, or 0 if not channel with the name is present.
     */
    uint8_t getChannelNumber(const char* channelName)
    {
        uint8_t idx = tMaxChannels;

        if (nullptr != channelName)
        {
            for (idx = 0U; idx < tMaxChannels; idx++)
            {
                if (0U == strncmp(channelName, m_dataChannels[idx].m_name, CHANNEL_NAME_MAX_LEN))
                {
                    break;
                }
            }
        }

        return (idx == tMaxChannels) ? 0U : (idx + 1U);
    }

    /**
     * Creates a new channel on the server.
     * @param[in] channelName Name of the channel.
     * It will not be checked if the name already exists.
     * @param[in] dlc Length of the payload of this channel.
     * @param[in] cb Callback for data incoming to this channel.
     * @returns The channel number if succesfully created, or 0 if not able to create new channel.
     */
    uint8_t createChannel(const char* channelName, uint8_t dlc, ChannelCallback cb)
    {
        // Using strnlen in case the name is not null-terminated.
        uint8_t nameLength = strnlen(channelName, CHANNEL_NAME_MAX_LEN);
        uint8_t idx = tMaxChannels;

        if ((nullptr != channelName) && (MAX_DATA_LEN >= dlc) && (nullptr != cb))
        {
            for (idx = 0U; idx < tMaxChannels; idx++)
            {
                if (nullptr == m_dataChannels[idx].m_callback)
                {
                    memcpy(m_dataChannels[idx].m_name, channelName, nameLength);
                    m_dataChannels[idx].m_dlc      = dlc;
                    m_dataChannels[idx].m_callback = cb;
                    break;
                }
            }
        }

        return (idx == tMaxChannels) ? 0U : (idx + 1U);
    }

    /**
     * Suscribe to a Channel to receive the incoming data.
     * @param[in] channelName Name of the Channel to suscribe to.
     * @param[in] callback Callback to return the incoming data.
     */
    void subscribeToChannel(const char* channelName, ChannelCallback callback)
    {
        if ((nullptr != channelName) && (nullptr != callback))
        {
            // Suscribe to channel.
            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH];
            buf[0U] = COMMANDS::SCRB;
            memcpy(&buf[1U], channelName, CHANNEL_NAME_MAX_LEN);
            send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf));

            // Save Name and Callback for channel creation after response.
            memcpy(m_pendingSuscribeChannel.m_name, channelName, CHANNEL_NAME_MAX_LEN);
            m_pendingSuscribeChannel.m_callback = callback;
        }
    }

private:
    /**
     * Callback for the Control Channel
     * @param[in] payload Payload of received frame.
     * @param[in] payloadSize Length of Payload
     */
    void callbackControlChannel(const uint8_t* payload, const uint8_t payloadSize)
    {
        if (nullptr == payload)
        {
            return;
        }

        uint8_t cmdByte = payload[0U];

        switch (cmdByte)
        {

        case COMMANDS::SYNC:
        {
            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SYNC_RSP, payload[1U], payload[2U], payload[3U],
                                                           payload[4U]};
            send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf));
            break;
        }

        case COMMANDS::SYNC_RSP:
        {
            uint32_t rcvTimestamp = 0;

            // Using (payloadSize - 1U) as CMD Byte is not passed.
            if (Util::byteArrayToUint32(&payload[1], (payloadSize - 1U), rcvTimestamp))
            {
                // Check Timestamp with m_lastSyncCommand
                if (rcvTimestamp == m_lastSyncCommand)
                {
                    m_lastSyncResponse = m_lastSyncCommand;
                    m_isSynced         = true;
                }
            }
            break;
        }

        case COMMANDS::SCRB:
        {
            uint8_t channelNumber = getChannelNumber((const char*)&payload[1U]);

            if (CONTROL_CHANNEL_NUMBER != channelNumber)
            {
                uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SCRB_RSP, channelNumber,
                                                               getChannelDLC(channelNumber)};
                send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf));
            }

            break;
        }

        case COMMANDS::SCRB_RSP:
        {
            // Check if a SCRB is pending
            if (nullptr != m_pendingSuscribeChannel.m_callback)
            {
                uint8_t channelNumber = payload[1U];
                uint8_t channelDLC    = payload[2U];

                memcpy(m_dataChannels[channelNumber - 1U].m_name, m_pendingSuscribeChannel.m_name,
                       CHANNEL_NAME_MAX_LEN);
                m_dataChannels[channelNumber - 1U].m_dlc      = channelDLC;
                m_dataChannels[channelNumber - 1U].m_callback = m_pendingSuscribeChannel.m_callback;

                m_pendingSuscribeChannel.m_callback = nullptr;
            }

            break;
        }

        default:
        {
            break;
        }
        }
    }

    /**
     * Receive and process RX Data.
     */
    void processRxData()
    {
        // Check for received data
        if (HEADER_LEN < m_stream.available())
        {
            // Create Frame and read header
            Frame rcvFrame;
            m_stream.readBytes(rcvFrame.fields.header.rawHeader, HEADER_LEN);

            uint8_t dlc = getChannelDLC(rcvFrame.fields.header.headerFields.m_channel);

            // Read Payload
            m_stream.readBytes(rcvFrame.fields.payload.m_data, dlc);

            if (isFrameValid(rcvFrame))
            {
                // Differenciate between Control and Data Channels
                if (CONTROL_CHANNEL_NUMBER == rcvFrame.fields.header.headerFields.m_channel)
                {
                    callbackControlChannel(rcvFrame.fields.payload.m_data, CONTROL_CHANNEL_PAYLOAD_LENGTH);
                }
                else if (nullptr != m_dataChannels[rcvFrame.fields.header.headerFields.m_channel - 1U].m_callback)
                {
                    // Callback
                    m_dataChannels[rcvFrame.fields.header.headerFields.m_channel - 1U].m_callback(
                        rcvFrame.fields.payload.m_data, dlc);
                }
            }
        }
    }

    /**
     * Periodic heartbeat.
     * Sends SYNC Command depending on the current Sync state.
     */
    void heartbeat()
    {
        uint32_t heartbeatPeriod  = HEATBEAT_PERIOD_UNSYNCED;
        uint32_t currentTimestamp = millis();

        if (m_isSynced)
        {
            heartbeatPeriod = HEATBEAT_PERIOD_SYNCED;
        }

        if ((currentTimestamp - m_lastSyncCommand) >= heartbeatPeriod)
        {
            // Timeout
            if (m_lastSyncCommand != m_lastSyncResponse)
            {
                m_isSynced = false;
            }

            // Send SYNC Command
            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SYNC};

            // Using (sizeof(buf) - 1U) as CMD Byte is not passed.
            Util::uint32ToByteArray(&buf[1], (sizeof(buf) - 1), currentTimestamp);

            send(CONTROL_CHANNEL_NUMBER, buf, sizeof(buf));
            m_lastSyncCommand = currentTimestamp;
        }
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channelNumber Channel to send frame to.
     * @param[in] payload Byte buffer to be sent.
     * @param[in] payloadSize Amount of bytes to send.
     */
    void send(uint8_t channelNumber, const uint8_t* payload, uint8_t payloadSize)
    {
        uint8_t channelDLC = getChannelDLC(channelNumber);

        if ((nullptr != payload) && (channelDLC == payloadSize) &&
            (m_isSynced || (CONTROL_CHANNEL_NUMBER == channelNumber)))
        {
            const uint8_t frameLength = HEADER_LEN + channelDLC;
            Frame         newFrame;

            newFrame.fields.header.headerFields.m_channel = channelNumber;
            memcpy(newFrame.fields.payload.m_data, payload, channelDLC);
            newFrame.fields.header.headerFields.m_checksum = checksum(newFrame);

            m_stream.write(newFrame.raw, frameLength);
        }
    }

    /**
     * Check if a Frame is valid using its checksum.
     * @param[in] frame Frame to be checked.
     * @returns true if the Frame's checksum is correct.
     */
    bool isFrameValid(const Frame& frame)
    {
        // Frame is valid when both checksums are the same.
        return (checksum(frame) == frame.fields.header.headerFields.m_checksum);
    }

    /**
     * Get the Payload Length of a channel
     * @param[in] channel Channel number to check
     * @returns DLC of the channel, or 0 if channel is not found.
     */
    uint8_t getChannelDLC(uint8_t channel)
    {
        uint8_t channelDLC = 0U;

        if (CONTROL_CHANNEL_NUMBER == channel)
        {
            channelDLC = CONTROL_CHANNEL_PAYLOAD_LENGTH;
        }
        else
        {
            channelDLC = m_dataChannels[channel - 1U].m_dlc;
        }

        return channelDLC;
    }

    /**
     * Performs the checksum of a Frame.
     * @param[in] frame Frame to calculate checksum
     * @returns checksum value
     */
    uint8_t checksum(const Frame& frame)
    {
        uint32_t sum = frame.fields.header.headerFields.m_channel;

        for (size_t idx = 0U; idx < getChannelDLC(frame.fields.header.headerFields.m_channel); idx++)
        {
            sum += frame.fields.payload.m_data[idx];
        }

        return (sum % 255U);
    }

private:
    /**
     *  Array of Data Channels.
     */
    Channel m_dataChannels[tMaxChannels];

    /**
     * Current Sync state.
     */
    bool m_isSynced;

    /**
     * Last Heartbeat timestamp.
     */
    uint32_t m_lastSyncCommand;

    /**
     * Last sync response timestamp.
     */
    uint32_t m_lastSyncResponse;

    /**
     * Channel used to store the name and the Callback of a pending suscription.
     */
    Channel m_pendingSuscribeChannel;

    /**
     * Stream for input and output of data.
     */
    Stream& m_stream;

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