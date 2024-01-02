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
 * @brief  Display realization
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup HALSim
 *
 * @{
 */

#ifndef DISPLAY_H
#define DISPLAY_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IDisplay.h"

#include <webots/Display.hpp>
#include <string>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the roboto simulation LCD. */
class Display : public IDisplay
{
public:
    /**
     * Constructs the display adapter.
     * 
     * @param[in] display   The robot display.
     */
    Display(webots::Display* display) : 
        IDisplay(), 
        m_display(display), 
        m_height(0), 
        m_width(0), 
        m_currX(0), 
        m_currY(0),
        m_lastX(0),
        m_lastY(0)
    {
        if (nullptr != display)
        {
            m_height = display->getHeight();
            m_width = display->getWidth();
        }
    }

    /**
     * Destroys the display adapter.
     */
    ~Display()
    {
    }

    /**
     * Clear the display and set the cursor to the upper left corner.
     */
    void clear() final
    {
        if (nullptr != m_display)
        {
            m_display->setColor(BLACK);
            m_display->fillRectangle(0, 0, m_width, m_height);
        }

        gotoXY(0, 0);
    }

    /**
     * Set the cursor to the given position.
     *
     * @param[in] xCoord x-coordinate, 0 is the most left position.
     * @param[in] yCoord y-coordinate, 0 is the most upper position.
     */
    void gotoXY(uint8_t xCoord, uint8_t yCoord) final
    {
        /* The one gets added to have a better central allignment of the text. */
        m_currY = static_cast<uint8_t>((m_height / (DISPLAYLINES + 1)) * (1 + yCoord));
        m_currX = static_cast<uint8_t>((m_width / DISPLAYCHARS) * xCoord);

        m_lastX = xCoord;
        m_lastY = yCoord;
    }

    /**
     * Print the string to the display at the current cursor position.
     *
     * @param[in] str   String
     *
     * @return Printed number of characters
     */
    size_t print(const char str[]) final
    {
        std::string text;
        text = str;

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);

        return text.length();
    }

    /**
     * Print the unsigned 8-bit value to the display at the current cursor position.
     *
     * @param[in] value Value
     *
     * @return Printed number of characters
     */
    size_t print(uint8_t value) final
    {
        std::string text;
        text = std::to_string(value);

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);

        return text.length();
    }

    /**
     * Print the unsigned 16-bit value to the display at the current cursor position.
     *
     * @param[in] value Value
     *
     * @return Printed number of characters
     */
    size_t print(uint16_t value) final
    {
        std::string text;
        text = std::to_string(value);

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);

        return text.length();
    }

    /**
     * Print the unsigned 32-bit value to the display at the current cursor position.
     *
     * @param[in] value Value
     *
     * @return Printed number of characters
     */
    size_t print(uint32_t value) final
    {
        std::string text;
        text = std::to_string(value);

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);

        return text.length();
    }

    /**
     * Print the signed 8-bit value to the display at the current cursor position.
     *
     * @param[in] value Value
     *
     * @return Printed number of characters
     */
    size_t print(int8_t value) final
    {
        std::string text;
        text = std::to_string(value);

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);

        return text.length();
    }

    /**
     * Print the signed 16-bit value to the display at the current cursor position.
     *
     * @param[in] value Value
     *
     * @return Printed number of characters
     */
    size_t print(int16_t value) final
    {
        std::string text;
        text = std::to_string(value);

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);

        return text.length();
    }

    /**
     * Print the signed 32-bit value to the display at the current cursor position.
     *
     * @param[in] value Value
     *
     * @return Printed number of characters
     */
    size_t print(int32_t value) final
    {
        std::string text;
        text = std::to_string(value);

        if (nullptr != m_display)
        {
            m_display->setColor(WHITE);
            m_display->drawText(text, m_currX, m_currY);
        }

        gotoXY((m_lastX + text.length()), m_lastY);
        
        return text.length();
    }

private:

    webots::Display*        m_display;          /**< The robot simulated display. */
    uint8_t                 m_height;           /**< The display height. */
    uint8_t                 m_width;            /**< The display width. */
    uint8_t                 m_currX;            /**< Internal variable for the current pixel position on the simulated display. */
    uint8_t                 m_currY;            /**< Internal variable for the current pixel position on the simulated display. */
    uint8_t                 m_lastX;            /**< The last x-value passed to the goto function. */
    uint8_t                 m_lastY;            /**< The last y-value passed to the goto function. */
    static const uint32_t   WHITE = 0xFFFFFF;   /**< The color white for the display text. */
    static const uint32_t   BLACK = 0x000000;   /**< The color black for the display background. */
    static const uint8_t    DISPLAYLINES = 2;   /**< The amount of lines the display has. */
    static const uint8_t    DISPLAYCHARS = 8;   /**< The characters per line the display has. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DISPLAY_H */
/** @} */
