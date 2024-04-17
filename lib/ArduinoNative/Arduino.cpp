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
 * @brief  Arduino native
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include "Terminal.h"

#ifdef UNIT_TEST

#include <time.h>

#else /* UNIT_TEST */

#include <Board.h>
#include <webots/Robot.hpp>
#include <Keyboard.h>
#include <getopt.h>
#include <Logging.h>

#endif /* UNIT_TEST */

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

#ifndef UNIT_TEST

/** This type defines the possible program arguments. */
typedef struct
{
    const char* robotName;              /**< Robot name */
    bool        isZumoComSystemEnabled; /**< Is the ZumoComSystem enabled? */
    const char* serialRxChannel;        /**< Serial Rx channel */
    const char* serialTxChannel;        /**< Serial Tx channel */
    bool        verbose;                /**< Show verbose information */

} PrgArguments;

#endif /* UNIT_TEST */

/******************************************************************************
 * Prototypes
 *****************************************************************************/

extern void setup();
extern void loop();

#ifndef UNIT_TEST

static int  handleCommandLineArguments(PrgArguments& prgArguments, int argc, char** argv);
static void showPrgArguments(const PrgArguments& prgArgs);

#endif /* UNIT_TEST */

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Terminal/Console stream. */
static Terminal gTerminalStream;

/** Serial driver, used by Arduino applications. */
Serial_ Serial(gTerminalStream);

#ifndef UNIT_TEST

/**
 * The maximum duration a simulated time step can have.
 * Everything above would cause missbehaviour in the application.
 */
static const int MAX_TIME_STEP = 10;

/**
 * Simulation time handler, used by Arduino functions.
 */
static SimTime* gSimTime = nullptr;

/** Supported long program arguments. */
static const struct option LONG_OPTIONS[] = {{"help", no_argument, nullptr, 0},
                                             {"serialRxCh", required_argument, nullptr, 0},
                                             {"serialTxCh", required_argument, nullptr, 0},
                                             {nullptr, no_argument, nullptr, 0}}; /* Marks the end. */

/** Program argument default value of the robot name. */
static const char* PRG_ARG_ROBOT_NAME_DEFAULT = "";

/** Program argument default value of the flag whether the ZumoComSystem is enabled or not. */
static bool PRG_ARG_IS_ZUMO_COM_SYSTEM_ENABLED_DEFAULT = false;

/** Program argument default value of the serial rx channel. */
static const char PRG_ARG_SERIAL_RX_CH_DEFAULT[] = "1";

/** Program argument default value of the serial tx channel. */
static const char PRG_ARG_SERIAL_TX_CH_DEFAULT[] = "2";

/** Program argument default value of the verbose flag. */
static bool PRG_ARG_VERBOSE_DEFAULT = false;

#endif /* UNIT_TEST */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

#ifdef UNIT_TEST

extern int main(int argc, char** argv)
{
    setup(); /* Prepare test */
    loop();  /* Run test once */

    return 0;
}

extern unsigned long millis()
{
    clock_t now = clock();

    return (now * 1000UL) / CLOCKS_PER_SEC;
}

extern void delay(unsigned long ms)
{
    unsigned long timestamp = millis();

    while ((millis() - timestamp) < ms)
    {
        ;
    }
}

#else /* UNIT_TEST */

extern int main(int argc, char** argv)
{
    int              status    = 0;
    Board&           board     = Board::getInstance();
    Keyboard&        keyboard  = board.getKeyboard();
    WebotsSerialDrv* simSerial = board.getSimSerial();
    PrgArguments     prgArguments;

    printf("\n*** Radon Ulzer ***\n");

    /* Remove any buffering from stout and stderr to get the printed information immediately. */
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    (void)setvbuf(stderr, NULL, _IONBF, 0);

    status = handleCommandLineArguments(prgArguments, argc, argv);

    if (0 == status)
    {
        /* Show used arguments only in verbose mode. */
        if (true == prgArguments.verbose)
        {
            showPrgArguments(prgArguments);
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

        if (0 == status)
        {
            /* Get simulation time handler. It will be used by millis() and delay(). */
            gSimTime = &board.getSimTime();
        }
    }

    if (0 != status)
    {
        /* Something went wrong previously and was already notified.*/
        ;
    }
    else if ((0 == gSimTime->getTimeStep()) || (MAX_TIME_STEP < gSimTime->getTimeStep()))
    {
        printf("Simulation time step is too high!\n");
        printf("This would cause missbehaviour in the application.\n");

        status = -1;
    }
    else
    {
        /**
         * Synchronization between the simulation steps and the control steps is done automatically
         * by Webots (If the synchronization field in the robot node is set to TRUE).
         * For a more detailed explanation see:
         * https://cyberbotics.com/doc/reference/robot#synchronous-versus-asynchronous-controllers
         */

        /* Enable all simulation devices. Must be done before any other access to the devices. */
        board.enableSimulationDevices();

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
            /* Call the setup function, which is the entry point of the Arduino application. */
            setup();

            while (true == gSimTime->step())
            {
                keyboard.getPressedButtons();
                loop();
            }
        }
    }

    return status;
}

extern unsigned long millis()
{
    return gSimTime->getElapsedTimeSinceReset();
}

extern void delay(unsigned long ms)
{
    unsigned long timestamp = millis();

    while ((millis() - timestamp) < ms)
    {
        if (false == gSimTime->step())
        {
            break;
        }
    }
}

/******************************************************************************
 * Local Functions
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
    prgArguments.isZumoComSystemEnabled = PRG_ARG_IS_ZUMO_COM_SYSTEM_ENABLED_DEFAULT;
    prgArguments.serialRxChannel        = PRG_ARG_SERIAL_RX_CH_DEFAULT;
    prgArguments.serialTxChannel        = PRG_ARG_SERIAL_TX_CH_DEFAULT;
    prgArguments.verbose                = PRG_ARG_VERBOSE_DEFAULT;

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
    /* Skip verbose flag. */
}

#endif /* UNIT_TEST */