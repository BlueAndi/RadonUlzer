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

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Webots serial driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALSim
 *
 * @{
 */

#ifndef WEBOTS_SERIAL_DRV_H
#define WEBOTS_SERIAL_DRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Stream.h"
#include <webots/emitter.hpp>
#include <webots/receiver.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Webots serial driver.
 */
class WebotsSerialDrv : public Stream
{
public:
    /**
     * Construct a Webots serial driver.
     *
     * @param[in] emitter   Webots emitter used to send data over simulated serial.
     * @param[in] receiver  Webots receiver used to receive data over simulated serial.
     */
    WebotsSerialDrv(webots::Emitter* emitter, webots::Receiver* receiver);

    /**
     * Destruct the Webots serial driver.
     */
    virtual ~WebotsSerialDrv();

    /**
     * Set the serial receive channel id.
     *
     * @param[in] channelId Channel ID, shall be positive for inter-robot communication.
     */
    void setRxChannel(int32_t channelId);

    /**
     * Set the serial sender channel id.
     *
     * @param[in] channelId Channel ID, shall be positive for inter-robot communication.
     */
    void setTxChannel(int32_t channelId);

    /**
     * Print argument to the output stream.
     *
     * @param[in] str Argument to print.
     */
    void print(const char str[]) final;

    /**
     * Print argument to the output stream.
     *
     * @param[in] value Argument to print.
     */
    void print(uint8_t value) final;

    /**
     * Print argument to the output stream.
     *
     * @param[in] value Argument to print.
     */
    void print(uint16_t value) final;

    /**
     * Print argument to the output stream.
     *
     * @param[in] value Argument to print.
     */
    void print(uint32_t value) final;

    /**
     * Print argument to the output stream.
     *
     * @param[in] value Argument to print.
     */
    void print(int8_t value) final;

    /**
     * Print argument to the output stream.
     *
     * @param[in] value Argument to print.
     */
    void print(int16_t value) final;

    /**
     * Print argument to the output stream.
     *
     * @param[in] value Argument to print.
     */
    void print(int32_t value) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] str Argument to print.
     */
    void println(const char str[]) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] value Argument to print.
     */
    void println(uint8_t value) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] value Argument to print.
     */
    void println(uint16_t value) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] value Argument to print.
     */
    void println(uint32_t value) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] value Argument to print.
     */
    void println(int8_t value) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] value Argument to print.
     */
    void println(int16_t value) final;

    /**
     * Print argument to the output stream.
     * Appends carriage return at the end of the argument.
     *
     * @param[in] value Argument to print.
     */
    void println(int32_t value) final;

    /**
     * Write data buffer to serial.
     *
     * @param[in] buffer Byte buffer to send.
     * @param[in] length Number of bytes to send.
     *
     * @return Number of bytes written.
     */
    size_t write(const uint8_t* buffer, size_t length) final;

    /**
     * Check if any data has been received.
     *
     * @return Number of available bytes.
     */
    int available() const final;

    /**
     * Read data from serial.
     *
     * @param[in] buffer Array to read data in.
     * @param[in] length Number of bytes to be read.
     *
     * @return Number of bytes read from stream.
     */
    size_t readBytes(uint8_t* buffer, size_t length) final;

private:
    webots::Emitter*  m_emitter;     /**< The emitter used to send data. */
    webots::Receiver* m_receiver;    /**< The receiver used to receive data. */
    size_t            m_dataRemains; /**< Number of bytes which are remaining in head packet. */

    /* Not allowed. */
    WebotsSerialDrv();
    WebotsSerialDrv(const WebotsSerialDrv& srv);            /**< Copy construction of an instance. */
    WebotsSerialDrv& operator=(const WebotsSerialDrv& srv); /**< Assignment of an instance. */
};

#endif /* SOCKET_SERVER_H_ */
/** @} */
