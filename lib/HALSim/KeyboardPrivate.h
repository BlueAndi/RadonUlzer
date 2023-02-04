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
 * @brief  Keyboard realization
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup HALSim
 *
 * @{
 */

#ifndef KEYBOARD_PRIVATE_H
#define KEYBOARD_PRIVATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <webots/Keyboard.hpp>
#include <webots/Robot.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class implements a custom keyboard for better functionality. */
class KeyboardPrivate
{
public:
    /**
     * Constructs the encoders adapter.
     */
    KeyboardPrivate(webots::Robot* robot, webots::Keyboard *keyboard): 
        m_oldKeys(),
        m_newKeys(),
        m_robot(robot), 
        m_keyboard(keyboard)
    {
    }

    /**
     * Destroys the encoders adapter.
     */
    ~KeyboardPrivate()
    {
    }

    /**
     * Gets the current buttons pressed on the keyboard. Needs to be called 
     * cyclically to work correctly.
     */
    void getPressedButtons()
    {
        /* Copying the new values into the old values array. */
        for (uint8_t arrayIndex = 0; arrayIndex < (sizeof(m_oldKeys) / sizeof(*m_oldKeys)); ++arrayIndex)
        {
            m_oldKeys[arrayIndex] = m_newKeys[arrayIndex];
        }
        
        /* Getting the new values. Currently the limit of the simulation is seven
           keypresses that can be detected simultaniously. */
        for (uint8_t keyIndex = 0; keyIndex < (sizeof(m_oldKeys) / sizeof(*m_oldKeys)); ++keyIndex)
        {
            m_newKeys[keyIndex] = m_keyboard->getKey();
        }
    }

    /**
     * Checks weather the button A was pressed since the last update.
     * 
     * @return Return true if button A was pressed since the last update
     */
    bool buttonAPressed()
    {
        bool buttonPressed = false;

        /* Checks if button A is existing in the new values, but not the old ones.
           If so, it's newly pressed and true is returned. */
        if ((!arrayContains(m_oldKeys, sizeof(m_oldKeys), KEY_CODE_A_UPPER_CASE, KEY_CODE_A_LOWER_CASE)) && 
            (arrayContains(m_newKeys, sizeof(m_newKeys), KEY_CODE_A_UPPER_CASE, KEY_CODE_A_LOWER_CASE)))
        {
            buttonPressed = true;
        }

        return buttonPressed;
    }

    /**
     * Checks weather the button A was released since the last update.
     * 
     * @return Return true if button A was released since the last update
     */
    bool buttonAReleased()
    {
        bool buttonReleased = false;

        /* Checks if button A is existing in the new values, but not the old ones.
           If so, it's newly released and true is returned. */
        if ((!arrayContains(m_newKeys, sizeof(m_newKeys), KEY_CODE_A_UPPER_CASE, KEY_CODE_A_LOWER_CASE)) && 
            (arrayContains(m_oldKeys, sizeof(m_oldKeys), KEY_CODE_A_UPPER_CASE, KEY_CODE_A_LOWER_CASE)))
        {
            buttonReleased = true;
        }

        return buttonReleased;
    }

    /**
     * Checks weather the button B was pressed since the last update.
     * 
     * @return Return true if button B was pressed since the last update
     */
    bool buttonBPressed()
    {
        bool buttonPressed = false;

        /* Checks if button B is existing in the new values, but not the old ones.
           If so, it's newly pressed and true is returned. */
        if ((!arrayContains(m_oldKeys, sizeof(m_oldKeys), KEY_CODE_B_UPPER_CASE, KEY_CODE_B_LOWER_CASE)) && 
            (arrayContains(m_newKeys, sizeof(m_newKeys), KEY_CODE_B_UPPER_CASE, KEY_CODE_B_LOWER_CASE)))
        {
            buttonPressed = true;
        }

        return buttonPressed;
    }

    /**
     * Checks weather the button B was released since the last update.
     * 
     * @return Return true if button B was released since the last update
     */
    bool buttonBReleased()
    {
        bool buttonReleased = false;

        /* Checks if button B is existing in the new values, but not the old ones.
           If so, it's newly released and true is returned. */
        if ((!arrayContains(m_newKeys, sizeof(m_newKeys), KEY_CODE_B_UPPER_CASE, KEY_CODE_B_LOWER_CASE)) && 
            (arrayContains(m_oldKeys, sizeof(m_oldKeys), KEY_CODE_B_UPPER_CASE, KEY_CODE_B_LOWER_CASE)))
        {
            buttonReleased = true;
        }

        return buttonReleased;
    }

    /**
     * Checks weather the button C was pressed since the last update.
     * 
     * @return Return true if button C was pressed since the last update
     */
    bool buttonCPressed()
    {
        bool buttonPressed = false;

        /* Checks if button C is existing in the new values, but not the old ones.
           If so, it's newly pressed and true is returned. */
        if ((!arrayContains(m_oldKeys, sizeof(m_oldKeys), KEY_CODE_A_UPPER_CASE, KEY_CODE_C_LOWER_CASE)) && 
            (arrayContains(m_newKeys, sizeof(m_newKeys), KEY_CODE_A_UPPER_CASE, KEY_CODE_C_LOWER_CASE)))
        {
            buttonPressed = true;
        }

        return buttonPressed;
    }

    /**
     * Checks weather the button C was released since the last update.
     * 
     * @return Return true if button C was released since the last update
     */
    bool buttonCReleased()
    {
        bool buttonReleased = false;

        /* Checks if button C is existing in the new values, but not the old ones.
           If so, it's newly released and true is returned. */
        if ((!arrayContains(m_newKeys, sizeof(m_newKeys), KEY_CODE_C_UPPER_CASE, KEY_CODE_C_LOWER_CASE)) && 
            (arrayContains(m_oldKeys, sizeof(m_oldKeys), KEY_CODE_C_UPPER_CASE, KEY_CODE_C_LOWER_CASE)))
        {
            buttonReleased = true;
        }

        return buttonReleased;
    }

    /**
     * Waits until Button A gets released.
     * Needs to call the robots step() method and getPressedButtons()
     * to update the keypresses correctly and avoid getting stuck 
     * in the while loop.
     */
    void waitForReleaseA()
    {
        while (!buttonAReleased())
        {
            m_robot->step(int(m_robot->getBasicTimeStep()));
            getPressedButtons();
        }
    }

    /**
     * Waits until Button B gets released.
     * Needs to call the robots step() method and getPressedButtons()
     * to update the keypresses correctly and avoid getting stuck 
     * in the while loop.
     */
    void waitForReleaseB()
    {
        while (!buttonBReleased())
        {
            m_robot->step(int(m_robot->getBasicTimeStep()));
            getPressedButtons();
        }
    }

    /**
     * Waits until Button C gets released.
     * Needs to call the robots step() method and getPressedButtons()
     * to update the keypresses correctly and avoid getting stuck 
     * in the while loop.
     */
    void waitForReleaseC()
    {
        while (!buttonCReleased())
        {
            m_robot->step(int(m_robot->getBasicTimeStep()));
            getPressedButtons();
        }
    }

private:

    bool arrayContains(uint16_t array[], uint16_t arraySize, uint8_t elemLowerCase, uint8_t elemUppercase)
    {
        bool elementFound = false;
        for (uint8_t arrayIndex = 0; arrayIndex < (arraySize / sizeof(*array)); ++arrayIndex)
        {
            if((array[arrayIndex] == elemLowerCase) || (array[arrayIndex] == elemUppercase))
            {
                elementFound = true;;
            }
        }
        return elementFound;
    } 


    /** The key code of the lower case a character, which simulates the button. */
    static const char KEY_CODE_A_LOWER_CASE = 'a';

    /** The key code of the upper case A character, which simulates the button. */
    static const char KEY_CODE_A_UPPER_CASE = 'A';

    /** The key code of the lower case b character, which simulates the button. */
    static const char KEY_CODE_B_LOWER_CASE = 'b';

    /** The key code of the upper case B character, which simulates the button. */
    static const char KEY_CODE_B_UPPER_CASE = 'B';

    /** The key code of the lower case c character, which simulates the button. */
    static const char KEY_CODE_C_LOWER_CASE = 'c';

    /** The key code of the upper case C character, which simulates the button. */
    static const char KEY_CODE_C_UPPER_CASE = 'C';

    /** The maximum number of keys pressed simultaniously, that the simulation can process. */
    static const uint8_t MAX_KEY_NUMBER = 7;

    /** The keys presses during the last update. */
    uint16_t m_oldKeys[MAX_KEY_NUMBER];

    /** The keys pressed during this update. */
    uint16_t m_newKeys[MAX_KEY_NUMBER];

    webots::Keyboard* m_keyboard; /**< Robot keyboard */

    webots::Robot* m_robot; /**< Robot */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* KEYBOARD_PRIVATE_H */