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
 * @brief  The simulation robot board realization.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALSim
 *
 * @{
 */
#ifndef BOARD_H
#define BOARD_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IBoard.h>
#include <ButtonA.h>
#include <ButtonB.h>
#include <ButtonC.h>
#include <Buzzer.h>
#include <Display.h>
#include <Encoders.h>
#include <LineSensors.h>
#include <Motors.h>
#include <LedRed.h>
#include <LedYellow.h>
#include <LedGreen.h>
#include <Settings.h>

#include <math.h>
#include <webots/Robot.hpp>
#include <Keyboard.h>
#include <SimTime.h>

#ifdef DEBUG_ODOMETRY
#include <Sender.h>
#endif /* DEBUG_ODOMETRY */

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The concrete simulation robot board.
 */
class Board : public IBoard
{
public:
    /**
     * Get board instance.
     *
     * @return Board instance
     */
    static Board& getInstance()
    {
        static Board instance; /* idiom */

        return instance;
    }

    /**
     * Initialize the hardware.
     */
    void init() final;

    /**
     * Get button A driver.
     *
     * @return Button A driver.
     */
    IButton& getButtonA() final
    {
        return m_buttonA;
    }

    /**
     * Get button B driver.
     *
     * @return Button B driver.
     */
    IButton& getButtonB() final
    {
        return m_buttonB;
    }

    /**
     * Get button C driver.
     *
     * @return Button C driver.
     */
    IButton& getButtonC() final
    {
        return m_buttonC;
    }

    /**
     * Get buzzer driver.
     *
     * @return Buzzer driver.
     */
    IBuzzer& getBuzzer() final
    {
        return m_buzzer;
    }

    /**
     * Get LCD driver.
     *
     * @return LCD driver.
     */
    IDisplay& getDisplay() final
    {
        return m_display;
    }

    /**
     * Get encoders.
     *
     * @return Encoders driver.
     */
    IEncoders& getEncoders() final
    {
        return m_encoders;
    }

    /**
     * Get line sensors driver.
     *
     * @return Line sensor driver.
     */
    ILineSensors& getLineSensors() final
    {
        return m_lineSensors;
    }

    /**
     * Get motor driver.
     *
     * @return Motor driver.
     */
    IMotors& getMotors() final
    {
        return m_motors;
    }

    /**
     * Get red LED driver.
     *
     * @return Red LED driver.
     */
    ILed& getRedLed() final
    {
        return m_ledRed;
    }

    /**
     * Get yellow LED driver.
     *
     * @return Yellow LED driver.
     */
    ILed& getYellowLed() final
    {
        return m_ledYellow;
    }

    /**
     * Get green LED driver.
     *
     * @return Green LED driver.
     */
    ILed& getGreenLed() final
    {
        return m_ledGreen;
    }

    /**
     * Get the settings.
     *
     * @return Settings
     */
    ISettings& getSettings() final
    {
        return m_settings;
    }

#ifdef DEBUG_ODOMETRY

    /**
     * Get the sender driver, used to send data to the webots supervisor.
     *
     * @return Sender driver
     */
    ISender& getSender() final
    {
        return m_sender;
    }

#endif /* DEBUG_ODOMETRY */

    /**
     * Process actuators and sensors.
     */
    void process() final
    {
        m_buzzer.process();
    }

private:
    /** Name of the speaker in the robot simulation. */
    static const char* SPEAKER_NAME;

    /** Name of the display in the robot simulation. */
    static const char* DISPLAY_NAME;

    /** Name of the left motor in the robot simulation. */
    static const char* LEFT_MOTOR_NAME;

    /** Name of the right motor in the robot simulation. */
    static const char* RIGHT_MOTOR_NAME;

    /** Name of the infrared emitter 0 in the robot simulation. */
    static const char* EMITTER_0_NAME;

    /** Name of the infrared emitter 1 in the robot simulation. */
    static const char* EMITTER_1_NAME;

    /** Name of the infrared emitter 2 in the robot simulation. */
    static const char* EMITTER_2_NAME;

    /** Name of the infrared emitter 3 in the robot simulation. */
    static const char* EMITTER_3_NAME;

    /** Name of the infrared emitter 4 in the robot simulation. */
    static const char* EMITTER_4_NAME;

    /** Name of the position sensor of the left motor in the robot simulation. */
    static const char* POS_SENSOR_LEFT_NAME;

    /** Name of the position sensor of the right motor in the robot simulation. */
    static const char* POS_SENSOR_RIGHT_NAME;

    /** Name of the light sensor 0 in the robot simulation. */
    static const char* LIGHT_SENSOR_0_NAME;

    /** Name of the light sensor 1 in the robot simulation. */
    static const char* LIGHT_SENSOR_1_NAME;

    /** Name of the light sensor 2 in the robot simulation. */
    static const char* LIGHT_SENSOR_2_NAME;

    /** Name of the light sensor 3 in the robot simulation. */
    static const char* LIGHT_SENSOR_3_NAME;

    /** Name of the light sensor 4 in the robot simulation. */
    static const char* LIGHT_SENSOR_4_NAME;

    /** Name of the red LED in the robot simulation. */
    static const char* LED_RED_NAME;

    /** Name of the yellow LED in the robot simulation. */
    static const char* LED_YELLOW_NAME;

    /** Name of the green LED in the robot simulation. */
    static const char* LED_GREEN_NAME;

    /** Name of the front proximity sensor in the robot simulation. */
    static const char* PROXIMITY_SENSOR_FRONT_LEFT_NAME;

    /** Name of the front right proximity sensor in the robot simulation. */
    static const char* PROXIMITY_SENSOR_FRONT_RIGHT_NAME;

#ifdef DEBUG_ODOMETRY
    /** Name of the sender to the webots supervisor. */
    static const char* SENDER_NAME;
#endif /* DEBUG_ODOMETRY */

    /** Simulated roboter instance. */
    webots::Robot m_robot;

    /** Simulation time handler */
    SimTime m_simTime;

    /** Own keyboard that wraps the webots keyboard. */
    Keyboard m_keyboard;

    /** Button A driver */
    ButtonA m_buttonA;

    /** Button B driver */
    ButtonB m_buttonB;

    /** Button C driver */
    ButtonC m_buttonC;

    /** Buzzer driver */
    Buzzer m_buzzer;

    /** Display driver */
    Display m_display;

    /** Encoders driver */
    Encoders m_encoders;

    /** Line sensors driver */
    LineSensors m_lineSensors;

    /** Motors driver */
    Motors m_motors;

    /** Red LED driver */
    LedRed m_ledRed;

    /** Red LED driver */
    LedYellow m_ledYellow;

    /** Red LED driver */
    LedGreen m_ledGreen;

    /** Settings */
    Settings m_settings;

#ifdef DEBUG_ODOMETRY

    /** Sender driver for supervisor communication. */
    Sender m_sender;

#endif /* DEBUG_ODOMETRY */

    /**
     * Constructs the concrete board.
     */
    Board();

    /**
     * Destroys the concrete board.
     */
    ~Board()
    {
    }

    /**
     * Get the simulation time handler.
     *
     * @return Simulation time handler
     */
    SimTime& getSimTime()
    {
        return m_simTime;
    }

    /**
     * Get the keyboard instance of the simulation.
     *
     * @return The keyboard.
     */
    Keyboard& getKeyboard()
    {
        return m_keyboard;
    }

    /**
     * Get the simulation serial driver, which is connected within Webots.
     *
     * @return If serial driver is available, it will return a pointer to it, otherwise nullptr.
     */
    WebotsSerialDrv* getSimSerial()
    {
        /* Not supported. */
        return nullptr;
    }

    /**
     * Enable all simulation devices.
     * It is called by the main entry only.
     * Devices must be enabled before they can be used, and a simulation step must be performed before the application
     * initialization.
     */
    void enableSimulationDevices();

    /**
     * The main entry needs access to the simulation robot instance.
     * But all other application parts shall have no access, which is
     * solved by this friend.
     *
     * @param[in] argc  Number of arguments
     * @param[in] argv  Arguments
     *
     * @return Exit code
     */
    friend int main(int argc, char** argv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BOARD_H */
/** @} */
