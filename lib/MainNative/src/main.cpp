/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Main entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <Arduino.h>
#include <Board.h>
#include <RobotDeviceNames.h>
#include <Keyboard.h>
#include <Logging.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This type defines the possible program arguments. */
typedef struct
{
    const char* robotName;              /**< Robot name */
    bool        verbose;                /**< Show verbose information */
    bool        isZumoComSystemEnabled; /**< Is the ZumoComSystem enabled? */
    const char* serialRxChannel;        /**< Serial Rx channel */
    const char* serialTxChannel;        /**< Serial Tx channel */
    const char* cwd;                    /**< Current working directory */
    const char* settingsPath;           /**< Path to the settings file */

} PrgArguments;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static int           handleCommandLineArguments(PrgArguments& prgArguments, int argc, char** argv);
static void          showPrgArguments(const PrgArguments& prgArgs);
static unsigned long getSystemTick();
static void          systemDelay(unsigned long ms);

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Supported long program arguments. */
static const struct option LONG_OPTIONS[] = {{"help", no_argument, nullptr, 0},
                                             {"serialRxCh", required_argument, nullptr, 0},
                                             {"serialTxCh", required_argument, nullptr, 0},
                                             {"cwd", required_argument, nullptr, 0},
                                             {"settingsPath", required_argument, nullptr, 0},
                                             {nullptr, no_argument, nullptr, 0}}; /* Marks the end. */

/** Program argument default value of the robot name. */
static const char* PRG_ARG_ROBOT_NAME_DEFAULT = "";

/** Program argument default value of the verbose flag. */
static bool PRG_ARG_VERBOSE_DEFAULT = false;

/** Program argument default value of the flag whether the ZumoComSystem is enabled or not. */
static bool PRG_ARG_IS_ZUMO_COM_SYSTEM_ENABLED_DEFAULT = false;

/** Program argument default value of the serial rx channel. */
static const char PRG_ARG_SERIAL_RX_CH_DEFAULT[] = "1";

/** Program argument default value of the serial tx channel. */
static const char PRG_ARG_SERIAL_TX_CH_DEFAULT[] = "2";

/** Program argument default value of the current working directory. */
static const char* PRG_ARG_CWD_DEFAULT = ".";

/** Program argument default value of the settings path. */
static const char* PRG_ARG_SETTINGS_PATH_DEFAULT = nullptr;

/**
 * The maximum duration a simulated time step can have.
 * Everything above would cause missbehaviour in the application.
 */
static const int MAX_TIME_STEP = 10;

/**
 * Simulation time handler, used by Arduino functions.
 */
static SimTime* gSimTime = nullptr;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Main program entry point.
 *
 * @param[in] argc  Number of arguments
 * @param[in] argv  Array of arguments
 *
 * @return Status
 */
extern int main(int argc, char** argv)
{
    int              status = 0;
    PrgArguments     prgArguments;
    Board&           board     = Board::getInstance();
    Keyboard&        keyboard  = board.getKeyboard();
    WebotsSerialDrv* simSerial = board.getSimSerial();

    printf("\n*** Radon Ulzer ***\n");

    /* Remove any buffering from stout and stderr to get the printed information immediately. */
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    (void)setvbuf(stderr, NULL, _IONBF, 0);

    /* Before handling the command line arguments, set the default settings path. */
    PRG_ARG_SETTINGS_PATH_DEFAULT = board.getSettingsPath();

    /* Parse command line arguments. */
    status = handleCommandLineArguments(prgArguments, argc, argv);

    if (0 == status)
    {
        /* Show used arguments only in verbose mode. */
        if (true == prgArguments.verbose)
        {
            showPrgArguments(prgArguments);
        }

        /* Set the current working directory. */
        if (0 != strcmp(PRG_ARG_CWD_DEFAULT, prgArguments.cwd))
        {
            if (0 != chdir(prgArguments.cwd))
            {
                printf("Failed to set current working directory: %s\n", prgArguments.cwd);
                status = -1;
            }
        }

        /* It might happen that the user enables the ZumoComSystem, but the
         * choosen application doesn't support it.
         */
        if ((nullptr == simSerial) && (true == prgArguments.isZumoComSystemEnabled))
        {
            printf("Warning: The application doesn't support the ZumoComSystem.\n");

            prgArguments.isZumoComSystemEnabled = false;
        }

        /* Is ZumoComSystem enabled? */
        if ((nullptr != simSerial) && (true == prgArguments.isZumoComSystemEnabled))
        {
            /* Set serial rx/tx channels for communication with the ZumoComSystem. */
            simSerial->setRxChannel(atoi(prgArguments.serialRxChannel));
            simSerial->setTxChannel(atoi(prgArguments.serialTxChannel));

            /* Use the serial interface for ZumoComSystem communication and not for
             * logging.
             */
            Serial.setStream(*simSerial);
            Logging::disable();
        }

        /* Get simulation time handler. It will be used by millis() and delay(). */
        gSimTime = &board.getSimTime();

        if ((0 == gSimTime->getTimeStep()) || (MAX_TIME_STEP < gSimTime->getTimeStep()))
        {
            printf("Simulation time step is too high!\n");
            printf("This would cause missbehaviour in the application.\n");

            status = -1;
        }
    }

    if (0 == status)
    {
        /* Synchronization between the simulation steps and the control steps is done automatically
         * by Webots (If the synchronization field in the robot node is set to TRUE).
         * For a more detailed explanation see:
         * https://cyberbotics.com/doc/reference/robot#synchronous-versus-asynchronous-controllers
         */

        /* Enable all simulation devices. Must be done before any other access to the devices. */
        board.enableSimulationDevices();

        /* Specify the settings path. */
        board.setSettingsPath(prgArguments.settingsPath);

        /* Do one single simulation step to force that all sensors will deliver already data.
         * Otherwise e.g. the position sensor will provide NaN.
         * This must be done before setup() is called!
         *
         * Prerequisite: The sensors must be enabled!
         */
        if (false == gSimTime->step())
        {
            printf("Very first simulation step failed.\n");
            status = -1;
        }
        else
        {
            Arduino::setup(getSystemTick, systemDelay);

            while (true == gSimTime->step())
            {
                keyboard.getPressedButtons();
                Arduino::loop();
            }
        }
    }

    return status;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Handle the arguments passed to the programm.
 * If a argument is not given via command line interface, its default value will be used.
 *
 * @param[out]  prgArguments    Parsed program arguments
 * @param[in]   argc            Program argument count
 * @param[in]   argv            Program argument vector
 *
 * @returns 0 if handling was succesful. Otherwise, -1
 */
static int handleCommandLineArguments(PrgArguments& prgArguments, int argc, char** argv)
{
    int         status           = 0;
    const char* availableOptions = "n:cnvh";
    const char* programName      = argv[0];
    int         optionIndex      = 0;
    int         option           = getopt_long(argc, argv, availableOptions, LONG_OPTIONS, &optionIndex);

    /* Set default values */
    prgArguments.robotName              = PRG_ARG_ROBOT_NAME_DEFAULT;
    prgArguments.verbose                = PRG_ARG_VERBOSE_DEFAULT;
    prgArguments.isZumoComSystemEnabled = PRG_ARG_IS_ZUMO_COM_SYSTEM_ENABLED_DEFAULT;
    prgArguments.serialRxChannel        = PRG_ARG_SERIAL_RX_CH_DEFAULT;
    prgArguments.serialTxChannel        = PRG_ARG_SERIAL_TX_CH_DEFAULT;
    prgArguments.cwd                    = PRG_ARG_CWD_DEFAULT;
    prgArguments.settingsPath           = PRG_ARG_SETTINGS_PATH_DEFAULT;

    while ((-1 != option) && (0 == status))
    {
        switch (option)
        {
        case 0: /* Long option */

            if (0 == strcmp(LONG_OPTIONS[optionIndex].name, "help"))
            {
                status = -1;
            }
            else if (0 == strcmp(LONG_OPTIONS[optionIndex].name, "serialRxCh"))
            {
                prgArguments.serialRxChannel = optarg;
            }
            else if (0 == strcmp(LONG_OPTIONS[optionIndex].name, "serialTxCh"))
            {
                prgArguments.serialTxChannel = optarg;
            }
            else if (0 == strcmp(LONG_OPTIONS[optionIndex].name, "cwd"))
            {
                prgArguments.cwd = optarg;
            }
            else if (0 == strcmp(LONG_OPTIONS[optionIndex].name, "settingsPath"))
            {
                prgArguments.settingsPath = optarg;
            }
            else
            {
                status = -1;
            }
            break;

        case 'c': /* Is ZumoComSystem enabled? */
            prgArguments.isZumoComSystemEnabled = true;
            break;

        case 'n': /* Name */
            prgArguments.robotName = optarg;
            break;

        case 'v': /* Verbose */
            prgArguments.verbose = true;
            break;

        case '?': /* Unknown */
            /* fallthrough */

        case 'h': /* Help */
            /* fallthrough */

        default: /* Default */
            status = -1;
            break;
        }

        option = getopt_long(argc, argv, availableOptions, LONG_OPTIONS, &optionIndex);
    }

    /* Does the user need help? */
    if (0 > status)
    {
        printf("Usage: %s <option(s)>\nOptions:\n", programName);
        printf("\t-h\t\t\tShow this help message.\n");                                /* Help */
        printf("\t-n <NAME>\t\tSet robot name.\n");                                   /* Robot Name */
        printf("\t-c\t\t\tEnable ZumoComSystem. Default: Disabled\n");                /* Flag */
        printf("\t--serialRxCh <CHANNEL>\t\tSet serial rx channel (ZumoComSystem)."); /* Serial rx channel */
        printf(" Default: %s\n", PRG_ARG_SERIAL_RX_CH_DEFAULT); /* Serial rx channel default value */
        printf("\t--serialTxCh <CHANNEL>\t\tSet serial tx channel (ZumoComSystem)."); /* Serial txchannel */
        printf(" Default: %s\n", PRG_ARG_SERIAL_TX_CH_DEFAULT); /* Serial tx channel default value */
        printf("\t-v\t\t\tVerbose mode. Default: Disabled\n");  /* Flag */
        printf("\t--cwd <dir>\t\tSpecify working directory.");  /* Set process working directory */
        printf("\t--settingsPath <path>\t\tSpecify settings file path. Default: %s\n",
               PRG_ARG_SETTINGS_PATH_DEFAULT); /* Settings path default value */
    }

    return status;
}

/**
 * Show program arguments on the console.
 *
 * @param[in] prgArgs   Program arguments
 */
static void showPrgArguments(const PrgArguments& prgArgs)
{
    printf("Robot name       : %s\n", prgArgs.robotName);
    printf("ZumoComSystem    : %s\n", (false == prgArgs.isZumoComSystemEnabled) ? "disabled" : "enabled");
    printf("Serial rx channel: %s\n", prgArgs.serialRxChannel);
    printf("Serial tx channel: %s\n", prgArgs.serialTxChannel);
    printf("Current working directory: %s\n", prgArgs.cwd);
    printf("Settings path    : %s\n", prgArgs.settingsPath);
    /* Skip verbose flag. */
}

/**
 * Get the system tick in ms.
 *
 * @return Timestamp (system tick) in ms
 */
static unsigned long getSystemTick()
{
    unsigned long timestamp = 0U;

    if (nullptr != gSimTime)
    {
        timestamp = gSimTime->getElapsedTimeSinceReset();
    }

    return timestamp;
}

/**
 * Delay for a specific time in ms.
 *
 * @param[in] ms    Time in ms.
 */
static void systemDelay(unsigned long ms)
{
    unsigned long timestamp = millis();

    while ((millis() - timestamp) < ms)
    {
        if ((nullptr != gSimTime) && (false == gSimTime->step()))
        {
            break;
        }
    }
}
