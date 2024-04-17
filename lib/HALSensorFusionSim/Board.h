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
#include <Buzzer.h>
#include <Encoders.h>
#include <LineSensors.h>
#include <Motors.h>
#include <LedYellow.h>
#include <IMU.h>
#include <Settings.h>

#include <math.h>
#include <webots/Robot.hpp>
#include <Keyboard.h>
#include <SimTime.h>
#include <WebotsSerialDrv.h>

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
     * Get buzzer driver.
     *
     * @return Buzzer driver.
     */
    IBuzzer& getBuzzer() final
    {
        return m_buzzer;
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
     * Get yellow LED driver.
     *
     * @return Yellow LED driver.
     */
    ILed& getYellowLed() final
    {
        return m_ledYellow;
    }

    /**
     * Get IMU (Inertial Measurement Unit) driver.
     *
     * @return IMU driver
     */
    IIMU& getIMU() final
    {
        return m_imu;
    }

    /**
     * Get settings instance.
     *
     * @return Settings
     */
    ISettings& getSettings() final
    {
        return m_settings;
    }

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

    /** Name of the yellow LED in the robot simulation. */
    static const char* LED_YELLOW_NAME;

    /** Name of the accelerometer in the robot simulation. */
    static const char* ACCELEROMETER_NAME;

    /** Name of the gyro in the robot simulation. */
    static const char* GYRO_NAME;

    /** Name of the Magnetometer in the robot simulation. */
    static const char* MAGNETOMETER_NAME;

    /** Name of the serial emitter in the RadonUlzer simulation. */
    static const char* EMITTER_NAME_SERIAL;

    /** Name of the serial receiver in the RadonUlzer simulation. */
    static const char* RECEIVER_NAME_SERIAL;

    /** Simulated roboter instance. */
    webots::Robot m_robot;

    /** Simulation time handler */
    SimTime m_simTime;

    /** Own keyboard that wraps the webots keyboard. */
    Keyboard m_keyboard;

    /** Button A driver */
    ButtonA m_buttonA;

    /** Buzzer driver */
    Buzzer m_buzzer;

    /** Encoders driver */
    Encoders m_encoders;

    /** Line sensors driver */
    LineSensors m_lineSensors;

    /** Motors driver */
    Motors m_motors;

    /** Yellow LED driver */
    LedYellow m_ledYellow;

    /** IMU driver */
    IMU m_imu;

    /** Simulation serial driver */
    WebotsSerialDrv m_serialDrv;

    /** Settings */
    Settings m_settings;

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
        return &m_serialDrv;
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
