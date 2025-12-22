// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef RAWTOACES_LOG_H
#define RAWTOACES_LOG_H

#include <iostream>
#include <string>

namespace rta
{

/// Log levels for rawtoaces output
/// Each level includes all messages from lower levels
enum class LogLevel
{
    Silent = 0,  ///< No output except errors
    Error = 1,   ///< Error messages only
    Warning = 2, ///< Warnings and errors
    Info = 3,    ///< Basic progress information (default for -v)
    Debug = 4,   ///< Detailed configuration and processing info (-v -v)
    Trace = 5    ///< Very detailed output including matrices (-v -v -v)
};

/// Simple logging utility for consistent output formatting
class Log
{
  public:
    /// Get the singleton instance
    static Log &instance()
    {
        static Log log;
        return log;
    }

    /// Set the current verbosity level (0-5)
    void set_verbosity( int level )
    {
        if ( level <= 0 )
            _level = LogLevel::Silent;
        else if ( level == 1 )
            _level = LogLevel::Info;
        else if ( level == 2 )
            _level = LogLevel::Debug;
        else
            _level = LogLevel::Trace;
    }

    /// Get the current verbosity level as integer
    int get_verbosity() const { return static_cast<int>( _level ); }

    /// Check if a given level should be logged
    bool should_log( LogLevel level ) const { return level <= _level; }

    /// Log an error message (always shown unless Silent)
    template <typename... Args> void error( Args &&...args )
    {
        if ( _level >= LogLevel::Error )
        {
            std::cerr << "[ERROR] ";
            ( std::cerr << ... << args ) << std::endl;
        }
    }

    /// Log a warning message
    template <typename... Args> void warning( Args &&...args )
    {
        if ( _level >= LogLevel::Warning )
        {
            std::cerr << "[WARNING] ";
            ( std::cerr << ... << args ) << std::endl;
        }
    }

    /// Log an info message (basic progress, -v)
    template <typename... Args> void info( Args &&...args )
    {
        if ( _level >= LogLevel::Info )
        {
            std::cerr << "[INFO] ";
            ( std::cerr << ... << args ) << std::endl;
        }
    }

    /// Log a debug message (detailed info, -v -v)
    template <typename... Args> void debug( Args &&...args )
    {
        if ( _level >= LogLevel::Debug )
        {
            std::cerr << "[DEBUG] ";
            ( std::cerr << ... << args ) << std::endl;
        }
    }

    /// Log a trace message (very detailed, -v -v -v)
    template <typename... Args> void trace( Args &&...args )
    {
        if ( _level >= LogLevel::Trace )
        {
            std::cerr << "[TRACE] ";
            ( std::cerr << ... << args ) << std::endl;
        }
    }

    /// Log a message without prefix (for continuation or special formatting)
    template <typename... Args> void raw( LogLevel level, Args &&...args )
    {
        if ( _level >= level )
        {
            ( std::cerr << ... << args );
        }
    }

    /// Log a newline
    void newline( LogLevel level = LogLevel::Info )
    {
        if ( _level >= level )
        {
            std::cerr << std::endl;
        }
    }

private:
    Log() : _level( LogLevel::Warning ) {} // Default: show warnings and errors
    LogLevel _level;
};

/// Convenience macro for getting the logger
#define RTA_LOG rta::Log::instance()

} // namespace rta

#endif // RAWTOACES_LOG_H
