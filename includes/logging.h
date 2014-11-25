/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Logging.h
 * Header file for the logging
 * Copyright (C) 2005-2009 Simon Newton
 */
/**
 * @defgroup logging Logging
 * @brief The YDLE logging system.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 * #include <logging.h>
 *
 * // Call this once
 * ydle::InitLogging(ydle::YDLE_LOG_WARN, ydle::YDLE_LOG_STDERR);
 *
 * YDLE_FATAL << "Null pointer!";
 * YDLE_WARN << "Could not connect to server: " << ip_address;
 * YDLE_INFO << "Reading configs from " << config_dir;
 * YDLE_DEBUG << "Counter was " << counter;
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * @addtogroup logging
 * @{
 *
 * @file Logging.h
 * @brief Header file for YDLE Logging
 */

#ifndef INCLUDE_YDLE_LOGGING_H_
#define INCLUDE_YDLE_LOGGING_H_

#ifdef WIN32
#include <windows.h>  // for HANDLE
#endif

#include <ostream>
#include <string>
#include "ydle-log.h"


namespace ydle {

using std::string;

/**
 * @class LogDestination
 * @brief The base class for log destinations.
 */
class LogDestination {
 public:
	LogDestination(log_level l);
    /**
     * @brief Destructor
     */
    virtual ~LogDestination() {}

    /**
     * @brief An abstract function for writing to your log destination
     * @note You must over load this if you want to create a new log
     * destination
     */
    virtual void Write(log_level level, const string &log_line) = 0;
    void setLevel(log_level level);
protected:
    ydle::log_level _level;
};

/**
 * @brief A LogDestination that writes to stderr
 */
class StdErrorLogDestination: public LogDestination {
 public:
	StdErrorLogDestination(log_level level);
    /**
     * @brief Writes a messages out to stderr.
     */
    void Write(log_level level, const string &log_line);
};

/**
 * @brief A LogDestination that writes to syslog
 */
class SyslogDestination: public LogDestination {
 public:
	SyslogDestination(log_level l);
    /**
     * @brief Initialize the SyslogDestination
     */
    bool Init();

    /**
     * @brief Write a line to the system logger.
     * @note This is syslog on *nix or the event log on windows.
     */
    void Write(log_level level, const string &log_line);
};

/**@}*/


/**
 * @addtogroup logging
 * @{
 */

/**
 * @brief Set the logging level.
 * @param level the new log_level to use.
 */
void SetLogLevel(log_level level);



/**
 * @brief Increment the log level by one. The log level wraps to YDLE_LOG_NONE.
 */
void IncrementLogLevel();

/**
 * @brief Initialize the YDLE logging system from flags.
 * @pre ParseFlags() must have been called before calling this.
 * @returns true if logging was initialized sucessfully, false otherwise.
 */
bool InitLoggingFromFlags();

/**
 * @brief Initialize the YDLE logging system
 * @param level the level to log at
 * @param output the destintion for the logs
 * @returns true if logging was initialized sucessfully, false otherwise.
 */
bool InitLogging(log_level level, log_output output);

/**
 * @brief Initialize the YDLE logging system using the specified LogDestination.
 * @param level the level to log at
 * @param destination the LogDestination to use.
 * @returns true if logging was initialized sucessfully, false otherwise.
 */
void InitLogging(log_level level, LogDestination *destination);

/***/
}  // namespace ydle
/**@}*/
#endif  // INCLUDE_YDLE_LOGGING_H_
