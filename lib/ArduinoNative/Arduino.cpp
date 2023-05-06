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

#else

#include <Board.h>
#include <webots/Robot.hpp>
#include <Keyboard.h>
#include "Terminal.h"
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

/******************************************************************************
 * Prototypes
 *****************************************************************************/

extern void setup();
extern void loop();
int handleCommandLineArguments(int argc, char** argv);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

#ifndef UNIT_TEST

/** SocketServer Stream. */
static SocketServer gSocketStream;

/** Terminal/Console Stream. */
static Terminal gTerminalStream;

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

/**
 * Default Port used for Socket Communications.
 */
static const uint16_t SOCKET_SERVER_DEFAULT_PORT = 65432U;

/**
 * Maximum Number of Socket Connections.
 */
static const uint8_t SOCKET_SERVER_MAX_CONNECTIONS = 1U;

/**
 * Chosen Port Number for Socket Communications.
 */
static uint16_t  socketServerPortNumber = SOCKET_SERVER_DEFAULT_PORT;

#endif

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
    int       status                 = 0;
    Keyboard& keyboard               = Board::getInstance().getKeyboard();

    status = handleCommandLineArguments(argc, argv);

    if (0 == status)
    {
        if (false == gSocketStream.init(socketServerPortNumber, SOCKET_SERVER_MAX_CONNECTIONS))
        {
            printf("Error initializing SocketServer.\n");
            status = -1;
        }
        else
        {
            printf("SocketServer ready on port %d\n", socketServerPortNumber);

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

#endif

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Handle the Arguments passed to the programm.
 * @param[in] argc Program Argument Count
 * @param[in] argv Program Argument Vector
 * @returns 0 if handling was succesful. Otherwise, -1
 */
int handleCommandLineArguments(int argc, char** argv)
{
    int status = 0;
    const char *availableOptions = "p:n:h";

    for (;;)
    {
        switch (getopt(argc, argv, availableOptions))
        {
        case 'p': /* Port */
        {
            /* Parse Port Number */
            char* p;                                   /* End Pointer*/
            errno            = 0;                      /* Reset Error Register */
            long parsedValue = strtol(optarg, &p, 10); /* Long value parsed from string. */

            if ((*p == '\0') &&                        /* Make sure the string is completely read. */
                (errno == 0) &&                        /* No Errors were produced. */
                (UINT16_MAX >= parsedValue) &&         /* No overflow of uint16_t to allow direct casting. */
                (0U <= parsedValue))                   /* No negative values. */
            {
                socketServerPortNumber = parsedValue;
            }
            else
            {
                printf("Error parsing Port Argument.\n");
                status = -1;
            }
            continue;
        }
        case 'n': /* Name */
            printf("Instance has been named \"%s\"\n", optarg);
            continue;

        case '?':                                                  /* Unknown */
        case 'h':                                                  /* Help */
        default:                                                   /* Default */
            printf("Usage: %s <option(s)>\nOptions:\n", argv[0]);
            printf("\t-h\t\t\tShow this help message.\n");         /* Help */
            printf("\t-p <PORT NUMBER>\tSet SocketServer Port\n"); /* Port */
            printf("\t-n <NAME>\t\tSet instace name");             /* Name */
            status = -1;
            break;

        case -1: /* No more Arguments to parse. */
            break;
        }

        break;
    }

    return status;
}