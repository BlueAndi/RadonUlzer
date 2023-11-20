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
 * @brief  Moving average
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef MOVAVG_H
#define MOVAVG_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class implements a moving average algorithm.
 * It is designed for fix point integers.
 *
 * @tparam T    The data type of the moving average result and input values.
 * @tparam length The number of values, which are considered in the moving average calculation.
 */
template<typename T, uint8_t length>
class MovAvg
{
public:
    /**
     * Constructs the moving average calculator.
     * The default result will be 0.
     */
    MovAvg() : m_values(), m_wrIdx(0), m_written(0), m_sum(0)
    {
        clear();
    }

    /**
     * Destroys the moving average calculator.
     */
    ~MovAvg()
    {
    }

    /**
     * Clears the internal list of values.
     */
    void clear()
    {
        uint8_t idx = 0;

        for (idx = 0; idx < length; ++idx)
        {
            m_values[idx] = 0;
        }

        m_wrIdx   = 0;
        m_written = 0;
        m_sum     = 0;
    }

    /**
     * Write a value to the moving average calculator and returns the new
     * result.
     *
     * @param[in] value New value, which shall be considered.
     *
     * @return Moving average result
     */
    T write(T value)
    {
        T oldValue = m_values[m_wrIdx];

        m_values[m_wrIdx] = value;

        ++m_wrIdx;
        if (length <= m_wrIdx)
        {
            m_wrIdx = 0;
        }

        m_sum -= oldValue;
        m_sum += value;

        if (length > m_written)
        {
            ++m_written;
        }

        return m_sum / m_written;
    }

    /**
     * Get current moving average result.
     *
     * @return Moving average result
     */
    T getResult() const
    {
        T result = 0;

        if (0 < m_written)
        {
            result = m_sum / m_written;
        }

        return result;
    }

private:
    T       m_values[length]; /**< List of values, used for moving average calculation. */
    uint8_t m_wrIdx;          /**< Write index to list of values */
    uint8_t m_written;        /**< The number of written values to the list of values, till length is reached. */
    T       m_sum;            /**< Sum of all values */

    /* Not allowed. */
    MovAvg(const MovAvg& avg);            /**< Copy construction of an instance. */
    MovAvg& operator=(const MovAvg& avg); /**< Assignment of an instance. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MOVAVG_H */
/** @} */
