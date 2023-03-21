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

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Class for the YAP Server.
 * @tparam maxChannels Maximum number of channels
 */
template<uint8_t maxChannels>
class YAPServer
{
public:
    /**
     * Construct the YAP Server.
     */
    YAPServer() : m_isSynced(false), m_lastSyncCommand(0U), m_lastSyncResponse(0U), m_pendingSuscribeChannel()
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
     * @param[in] channel Channel to send frame to.
     * @param[in] data Byte buffer to be sent.
     * @param[in] length Amount of bytes to send.
     */
    void sendData(uint8_t channel, const uint8_t* data, uint8_t length)
    {
        if (CONTROL_CHANNEL_NUMBER != channel)
        {
            send(channel, data, length);
        }
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
        uint8_t itr = maxChannels;

        if ((MAX_DATA_LEN >= dlc) && (CHANNEL_NAME_MAX_LEN >= strnlen(channelName, CHANNEL_NAME_MAX_LEN)))
        {
            for (itr = 0; itr < maxChannels; itr++)
            {
                if (nullptr == m_dataChannels[itr].m_callback)
                {
                    m_dataChannels[itr].m_name     = channelName;
                    m_dataChannels[itr].m_dlc      = dlc;
                    m_dataChannels[itr].m_callback = cb;
                    break;
                }
            }
        }

        return (itr == maxChannels) ? 0U : (itr + 1);
    }

    /**
     * Suscribe to a Channel to receive the incoming data.
     * @param[in] channelName Name of the Channel to suscribe to.
     * @param[in] callback Callback to return the incoming data.
     */
    void subscribeToChannel(const char* channelName, ChannelCallback callback)
    {
        bool    isSuccess  = false;
        uint8_t nameLength = strnlen(channelName, CHANNEL_NAME_MAX_LEN);

        if (CHANNEL_NAME_MAX_LEN >= nameLength)
        {
            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH];
            buf[0] = COMMANDS::SCRB;

            for (uint8_t i = 0; i < nameLength; i++)
            {
                buf[i + 1] = channelName[i];
            }

            m_pendingSuscribeChannel.m_name     = channelName;
            m_pendingSuscribeChannel.m_callback = callback;
            send(CONTROL_CHANNEL_NUMBER, buf, CONTROL_CHANNEL_PAYLOAD_LENGTH);
        }
    }

    /**
     * Debug Function. Print all channel info.
     */
    void printChannels()
    {
        for (uint8_t i = 0; i < maxChannels; i++)
        {
            if (nullptr == m_dataChannels[i].m_callback)
            {
                Serial.print("Channel ");
                Serial.print(i + 1);
                Serial.println(": Nullptr");
            }
            else
            {
                Serial.print("Channel ");
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(m_dataChannels[i].m_name);
                Serial.print(" --- DLC: ");
                Serial.println(m_dataChannels[i].m_dlc);
            }
        }
        Serial.println("--------------------------");
    }

    void printFrame(const Frame& frame, uint8_t payloadLength)
    {
        Serial.println("=== Begin Frame ===");
        Serial.print("Channel:");
        Serial.println(frame.fields.header.headerFields.m_channel);
        Serial.print("Valid Checksum:");
        Serial.println(isFrameValid(frame));
        Serial.print("Data:");

        for (uint8_t i = 0; i < payloadLength; i++)
        {
            Serial.print(frame.fields.payload.m_data[i]);
            Serial.print(" ");
        }

        Serial.println("");
        Serial.println("=== End Frame ===");
    }

private:
    /**
     * Callback for the Control Channel
     * @param[in] rcvData Payload of received frame.
     */
    void callbackControlChannel(const uint8_t* rcvData, const uint8_t payloadLength)
    {
        uint8_t cmdByte = rcvData[0];

        switch (cmdByte)
        {

        case COMMANDS::SYNC:
        {
            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SYNC_RSP, rcvData[1], rcvData[2], rcvData[3],
                                                           rcvData[4]};
            send(CONTROL_CHANNEL_NUMBER, buf, CONTROL_CHANNEL_PAYLOAD_LENGTH);
            break;
        }

        case COMMANDS::SYNC_RSP:
        {
            uint32_t rcvTimestamp = ((uint32_t)rcvData[1] << 24) | ((uint32_t)rcvData[2] << 16) |
                                    ((uint32_t)rcvData[3] << 8) | ((uint32_t)rcvData[4]);

            // Check Timestamp with m_lastSyncCommand
            if (rcvTimestamp == m_lastSyncCommand)
            {
                m_lastSyncResponse = m_lastSyncCommand;
                m_isSynced         = true;
            }

            break;
        }

        case COMMANDS::SCRB:
        {
            char channelName[CHANNEL_NAME_MAX_LEN + 1];
            memcpy(channelName, &rcvData[1], CHANNEL_NAME_MAX_LEN);
            channelName[CHANNEL_NAME_MAX_LEN] = '\0';

            uint8_t itr;

            for (itr = 0; itr < maxChannels; itr++)
            {
                if (0U == strncmp(channelName, m_dataChannels[itr].m_name, CHANNEL_NAME_MAX_LEN))
                {
                    // Channel name found. Send SRCB_RSP
                    uint8_t channelNumber                       = itr + 1;
                    uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SCRB_RSP, channelNumber,
                                                                   m_dataChannels[itr].m_dlc};
                    send(CONTROL_CHANNEL_NUMBER, buf, CONTROL_CHANNEL_PAYLOAD_LENGTH);
                    break;
                }
            }

            break;
        }

        case COMMANDS::SCRB_RSP:
        {
            // Check if a SCRB is pending
            if (nullptr != m_pendingSuscribeChannel.m_callback)
            {
                uint8_t channelNumber = rcvData[1];
                uint8_t channelDLC    = rcvData[2];

                m_dataChannels[channelNumber - 1].m_name     = m_pendingSuscribeChannel.m_name;
                m_dataChannels[channelNumber - 1].m_dlc      = channelDLC;
                m_dataChannels[channelNumber - 1].m_callback = m_pendingSuscribeChannel.m_callback;

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
        if (HEADER_LEN < Serial.available())
        {
            // Create Frame and read header
            Frame rcvFrame;
            Serial.readBytes(rcvFrame.fields.header.rawHeader, HEADER_LEN);

            // Differenciate between Control and Data Channels
            if (CONTROL_CHANNEL_NUMBER == rcvFrame.fields.header.headerFields.m_channel)
            {
                // Read Payload
                Serial.readBytes(rcvFrame.fields.payload.m_data, CONTROL_CHANNEL_PAYLOAD_LENGTH);

                if (isFrameValid(rcvFrame))
                {
                    callbackControlChannel(rcvFrame.fields.payload.m_data, CONTROL_CHANNEL_PAYLOAD_LENGTH);
                }
            }
            // Determine which callback to call, if any.
            else if (nullptr != m_dataChannels[rcvFrame.fields.header.headerFields.m_channel - 1].m_callback)
            {
                uint8_t payloadLength = getChannelDLC(rcvFrame.fields.header.headerFields.m_channel);

                // Read Payload
                Serial.readBytes(rcvFrame.fields.payload.m_data, payloadLength);

                if (isFrameValid(rcvFrame))
                {
                    // Callback
                    {
                        m_dataChannels[rcvFrame.fields.header.headerFields.m_channel - 1].m_callback(
                            rcvFrame.fields.payload.m_data, payloadLength);
                    }
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
            uint16_t hiBytes  = ((currentTimestamp & 0xFFFF0000) >> 16U);
            uint16_t lowBytes = (currentTimestamp & 0x0000FFFF);

            uint8_t hiMSB  = ((hiBytes & 0xFF00) >> 8U);
            uint8_t hiLSB  = (hiBytes & 0x00FF);
            uint8_t lowMSB = ((lowBytes & 0xFF00) >> 8U);
            uint8_t lowLSB = (lowBytes & 0x00FF);

            uint8_t buf[CONTROL_CHANNEL_PAYLOAD_LENGTH] = {COMMANDS::SYNC, hiMSB, hiLSB, lowMSB, lowLSB};

            send(CONTROL_CHANNEL_NUMBER, buf, CONTROL_CHANNEL_PAYLOAD_LENGTH);
            m_lastSyncCommand = currentTimestamp;
        }
    }

    /**
     * Send a frame with the selected bytes.
     * @param[in] channel Channel to send frame to.
     * @param[in] data Byte buffer to be sent.
     * @param[in] length Amount of bytes to send.
     */
    void send(uint8_t channel, const uint8_t* data, uint8_t payloadLength)
    {
        uint8_t channelDLC = getChannelDLC(channel);

        if ((channelDLC >= payloadLength) && (m_isSynced || (CONTROL_CHANNEL_NUMBER == channel)))
        {
            const uint8_t frameLength = HEADER_LEN + channelDLC;
            Frame         newFrame;
            newFrame.fields.header.headerFields.m_channel = channel;

            for (uint8_t i = 0; i < payloadLength; i++)
            {
                newFrame.fields.payload.m_data[i] = data[i];
            }

            newFrame.fields.header.headerFields.m_checksum = checksum(newFrame);

            Serial.write(newFrame.raw, frameLength);
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

    uint8_t getChannelDLC(uint8_t channel)
    {
        uint8_t channelDLC = 0U;

        if (CONTROL_CHANNEL_NUMBER == channel)
        {
            channelDLC = CONTROL_CHANNEL_PAYLOAD_LENGTH;
        }
        else
        {
            channelDLC = m_dataChannels[channel - 1].m_dlc;
        }

        return channelDLC;
    }

    uint8_t checksum(const Frame& frame)
    {
        uint32_t sum = frame.fields.header.headerFields.m_channel;

        for (size_t i = 0; i < getChannelDLC(frame.fields.header.headerFields.m_channel); i++)
        {
            sum += frame.fields.payload.m_data[i];
        }

        return (sum % 255);
    }

private:
    /**
     *  Array of Data Channels.
     */
    Channel m_dataChannels[maxChannels];

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

private:
    /** Prevent instance copying */
    YAPServer(const YAPServer& avg);
    YAPServer& operator=(const YAPServer& avg);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* YAP_SERVER_H */