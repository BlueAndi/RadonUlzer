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
 * @brief  Arduino native
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

#ifdef UNIT_TEST

#include <time.h>
#include "Terminal.h"

#else

#include <Board.h>
#include <webots/Robot.hpp>
#include <Keyboard.h>
#include "SocketServer.h"
#include <getopt.h>

#endif

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
    const char* socketServerPort; /**< Socket server port */
    const char* robotName;        /**< Robot name */
    bool        verbose;          /**< Show verbose information */
} PrgArguments;

#endif

/******************************************************************************
 * Prototypes
 *****************************************************************************/

extern void setup();
extern void loop();

#ifndef UNIT_TEST

static int  handleCommandLineArguments(PrgArguments& prgArguments, int argc, char** argv);
static void showPrgArguments(const PrgArguments& prgArgs);

#endif

/******************************************************************************
 * Local Variables
 *****************************************************************************/

#ifdef UNIT_TEST

/** Terminal/Console stream. */
static Terminal gTerminalStream;

/** Serial driver, used by Arduino applications. */
Serial_ Serial(gTerminalStream);

#else /* UNIT_TEST */

/** SocketServer stream. */
static SocketServer gSocketStream;

/** Serial driver, used by Arduino applications. */
Serial_ Serial(gSocketStream);

/**
 * The maximum duration a simulated time step can have.
 * Everything above would cause missbehaviour in the application.
 */
static const int MAX_TIME_STEP = 10;

/**
 * Simulation time handler, used by Arduino functions.
 */
static SimTime* gSimTime = nullptr;

/** Program argument default value of the robot name. */
static const char* PRG_ARG_ROBOT_NAME_DEFAULT = "";

/** Default port used for socket communications. */
static const char* PRG_ARG_SOCKET_SERVER_PORT_DEFAULT = "65432";

/** Program argument default value of the verbose flag. */
static bool PRG_ARG_VERBOSE_DEFAULT = false;

/**
 * Maximum number of socket connections.
 */
static const uint8_t SOCKET_SERVER_MAX_CONNECTIONS = 1U;

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

#else

extern int main(int argc, char** argv)
{
    int          status   = 0;
    Keyboard&    keyboard = Board::getInstance().getKeyboard();
    PrgArguments prgArguments;

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

        /* Initialize the socket server. */
        if (false == gSocketStream.init(prgArguments.socketServerPort, SOCKET_SERVER_MAX_CONNECTIONS))
        {
            printf("Error initializing SocketServer.\n");
            status = -1;
        }
        else
        {
            if (true == prgArguments.verbose)
            {
                printf("SocketServer ready on port %s.\n", prgArguments.socketServerPort);
            }

            /* Get simulation time handler. It will be used by millis() and delay(). */
            gSimTime = &Board::getInstance().getSimTime();
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

        setup();

        while (true == gSimTime->step())
        {
            keyboard.getPressedButtons();
            loop();
            gSocketStream.process();
        }

        status = 0;
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
    const char* availableOptions = "p:n:h";
    const char* programName      = argv[0];
    int         option           = getopt(argc, argv, availableOptions);

    /* Set default values */
    prgArguments.socketServerPort = PRG_ARG_SOCKET_SERVER_PORT_DEFAULT;
    prgArguments.robotName        = PRG_ARG_ROBOT_NAME_DEFAULT;
    prgArguments.verbose          = PRG_ARG_VERBOSE_DEFAULT;

    while ((-1 != option) && (0 == status))
    {
        switch (option)
        {
        case 'n': /* Name */
            prgArguments.robotName = optarg;
            break;

        case 'p': /* SocketServer Port */
            prgArguments.socketServerPort = optarg;
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

        option = getopt(argc, argv, availableOptions);
    }

    /* Does the user need help? */
    if (0 > status)
    {
        printf("Usage: %s <option(s)>\nOptions:\n", programName);
        printf("\t-h\t\t\tShow this help message.\n");                /* Help */
        printf("\t-n <NAME>\t\tSet robot name.");                     /* Robot Name */
        printf("\t-p <PORT NUMBER>\tSet SocketServer port.");         /* SocketServer Port */
        printf(" Default: %s\n", PRG_ARG_SOCKET_SERVER_PORT_DEFAULT); /* SocketServer port default value*/
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
    printf("Robot name             : %s\n", prgArgs.robotName);
    printf("SocketServer Port      : %s\n", prgArgs.socketServerPort);
    /* Skip verbose flag. */
}

#endif