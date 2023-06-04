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
 * @brief  Logging
 * @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef LOGGING_H
#define LOGGING_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

#ifndef LOG_FATAL_ENABLE
#define LOG_FATAL_ENABLE (1)
#endif /* LOG_FATAL_ENABLE */

#ifndef LOG_ERROR_ENABLE
#define LOG_ERROR_ENABLE (1)
#endif /* LOG_ERROR_ENABLE */

#ifndef LOG_WARNING_ENABLE
#define LOG_WARNING_ENABLE (1)
#endif /* LOG_WARNING_ENABLE */

#ifndef LOG_INFO_ENABLE
#define LOG_INFO_ENABLE (1)
#endif /* LOG_INFO_ENABLE */

#ifndef LOG_DEBUG_ENABLE
#define LOG_DEBUG_ENABLE (0)
#endif /* LOG_DEBUG_ENABLE */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#if (0 == LOG_FATAL_ENABLE)

#define LOG_FATAL(_filename, _msg)
#define LOG_FATAL_VAL(_filename, _msg, _val)
#define LOG_FATAL_HEAD(_filename)
#define LOG_FATAL_MSG(_msg)
#define LOG_FATAL_TAIL()

#else /* (0 == LOG_FATAL_ENABLE) */

/** Log fatal error message. */
#define LOG_FATAL(_filename, _msg) Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_FATAL, (_msg))

/** Log fatal error message with additional value. */
#define LOG_FATAL_VAL(_filename, _msg, _val)                                                                           \
    Logging::print<_val>((_filename), __LINE__, Logging::LOG_LEVEL_FATAL, (_msg), (_val))

/** Log fatal error header. */
#define LOG_FATAL_HEAD(_filename) Logging::printHead((_filename), __LINE__, Logging::LOG_LEVEL_FATAL)

/** Log fatal error message, without line feed. */
#define LOG_FATAL_MSG(_msg)       Logging::printMsg(_msg)

/** Log fatal error tail. */
#define LOG_FATAL_TAIL()          Logging::printTail()

#endif /* (0 == LOG_FATAL_ENABLE) */

#if (0 == LOG_ERROR_ENABLE)

#define LOG_ERROR(_filename, _msg)
#define LOG_ERROR_VAL(_filename, _msg, _val)
#define LOG_ERROR_HEAD(_filename)
#define LOG_ERROR_MSG(_msg)
#define LOG_ERROR_TAIL()

#else /* (0 == LOG_ERROR_ENABLE) */

/** Log error message. */
#define LOG_ERROR(_filename, _msg) Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_ERROR, (_msg))

/** Log error message with additional value. */
#define LOG_ERROR_VAL(_filename, _msg, _val)                                                                           \
    Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_ERROR, (_msg), (_val))

/** Log error header. */
#define LOG_ERROR_HEAD(_filename) Logging::printHead((_filename), __LINE__, Logging::LOG_LEVEL_ERROR)

/** Log error error message, without line feed. */
#define LOG_ERROR_MSG(_msg)       Logging::printMsg(_msg)

/** Log error tail. */
#define LOG_ERROR_TAIL()          Logging::printTail()

#endif /* (0 == LOG_ERROR_ENABLE) */

#if (0 == LOG_WARNING_ENABLE)

#define LOG_WARNING(_filename, _msg)
#define LOG_WARNING_VAL(_filename, _msg, _val)
#define LOG_WARNING_HEAD(_filename)
#define LOG_WARNING_MSG(_msg)
#define LOG_WARNING_TAIL()

#else /* (0 == LOG_WARNING_ENABLE) */

/** Log warning message. */
#define LOG_WARNING(_filename, _msg) Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_WARNING, (_msg))

/** Log warning message with additional value. */
#define LOG_WARNING_VAL(_filename, _msg, _val)                                                                         \
    Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_WARNING, (_msg), (_val))

/** Log warning header. */
#define LOG_WARNING_HEAD(_filename) Logging::printHead((_filename), __LINE__, Logging::LOG_LEVEL_WARNING)

/** Log warning message, without line feed. */
#define LOG_WARNING_MSG(_msg)       Logging::printMsg(_msg)

/** Log warning tail. */
#define LOG_WARNING_TAIL()          Logging::printTail()

#endif /* (0 == LOG_WARNING_ENABLE) */

#if (0 == LOG_INFO_ENABLE)

#define LOG_INFO(_filename, _msg)
#define LOG_INFO_VAL(_filename, _msg, _val)
#define LOG_INFO_HEAD(_filename)
#define LOG_INFO_MSG(_msg)
#define LOG_INFO_TAIL()

#else /* (0 == LOG_INFO_ENABLE) */

/** Log info message. */
#define LOG_INFO(_filename, _msg) Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_INFO, (_msg))

/** Log info message with additional value. */
#define LOG_INFO_VAL(_filename, _msg, _val)                                                                            \
    Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_INFO, (_msg), (_val))

/** Log info header. */
#define LOG_INFO_HEAD(_filename) Logging::printHead((_filename), __LINE__, Logging::LOG_LEVEL_INFO)

/** Log info message, without line feed. */
#define LOG_INFO_MSG(_msg)       Logging::printMsg(_msg)

/** Log info tail. */
#define LOG_INFO_TAIL()          Logging::printTail()

#endif /* (0 == LOG_INFO_ENABLE) */

#if (0 == LOG_DEBUG_ENABLE)

#define LOG_DEBUG(_filename, _msg)
#define LOG_DEBUG_VAL(_filename, _msg, _val) (void)(_filename)
#define LOG_DEBUG_HEAD(_filename)
#define LOG_DEBUG_MSG(_msg)
#define LOG_DEBUG_TAIL()

#else /* (0 == LOG_DEBUG_ENABLE) */

/** Log debug message. */
#define LOG_DEBUG(_filename, _msg) Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_DEBUG, (_msg))

/** Log debug message with additional value. */
#define LOG_DEBUG_VAL(_filename, _msg, _val)                                                                           \
    Logging::print((_filename), __LINE__, Logging::LOG_LEVEL_DEBUG, (_msg), (_val))

/** Log debug header. */
#define LOG_DEBUG_HEAD(_filename) Logging::printHead((_filename), __LINE__, Logging::LOG_LEVEL_DEBUG)

/** Log debug message, without line feed. */
#define LOG_DEBUG_MSG(_msg)       Logging::printMsg(_msg)

/** Log debug tail. */
#define LOG_DEBUG_TAIL()          Logging::printTail()

#endif /* (0 == LOG_DEBUG_ENABLE) */

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Logging class for log messages depending on the previously set log level.
 */
namespace Logging
{
    /**
     * Enumeration to distinguish between different levels of severity.
     */
    enum LogLevel
    {
        LOG_LEVEL_FATAL =
            0, /**< Any error that is forcing a shutdown of service or application, because there is no way out. */
        LOG_LEVEL_ERROR,   /**< Any error that is fatal for the operating, but not for the service or application. */
        LOG_LEVEL_WARNING, /**< Anything that shows the user to pay attention, but can be automatically be recovered. */
        LOG_LEVEL_INFO,    /**< General useful information for the user. */
        LOG_LEVEL_DEBUG,   /**< A diagnostic message helpful for the developer. */
    };

    /**
     * Print log message header.
     * Use printMsg() to log the message itself.
     *
     * @param[in] filename      The name of the file, where the log message is located.
     * @param[in] lineNumber    The line number in the file, where the log message is located.
     * @param[in] level         The severity level.
     */
    void printHead(const char* filename, int lineNumber, LogLevel level);

    /**
     * Print message without line feed.
     * It can be used several times to concatenate a message without additional buffering.
     *
     * @param[in] message The message itself.
     */
    void printMsg(const char* message);

    /**
     * Print tail of log message.
     */
    void printTail();

    /**
     * Print log message.
     *
     * @param[in] filename      The name of the file, where the log message is located.
     * @param[in] lineNumber    The line number in the file, where the log message is located.
     * @param[in] level         The severity level.
     * @param[in] message       The message itself.
     */
    void print(const char* filename, int lineNumber, LogLevel level, const char* message);

    /**
     * Print log message.
     *
     * @param[in] filename      The name of the file, where the log message is located.
     * @param[in] lineNumber    The line number in the file, where the log message is located.
     * @param[in] level         The severity level.
     * @param[in] message       The message itself.
     * @param[in] value         The value to print after the message.
     */
    template<typename T>
    void print(const char* filename, int lineNumber, LogLevel level, const char* message, T value)
    {
        printHead(filename, lineNumber, level);
        Serial.print(message);
        Serial.println(value);
    }

}; // namespace Logging

    /******************************************************************************
     * Functions
     *****************************************************************************/

#endif /* __LOGGING_H__ */
/** @} */
