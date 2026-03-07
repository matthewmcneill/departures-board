/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *
 * Module: include/Logger.hpp
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - Logger: Class definition
 * - registerSecret: Register secret
 * - LOG_INFO: Info macro
 * - LOG_WARN: Warn macro
 * - LOG_ERROR: Error macro
 * - LOG_DEBUG: Debug macro
 * - printRedacted: Print redacted
 * - redact: Redact
 */
#pragma once

#include <Arduino.h>
#include <vector>

#ifdef ENABLE_DEBUG_LOG
  #define LOG_INFO(msg)  Logger::_info(msg)
  #define LOG_WARN(msg)  Logger::_warn(msg)
  #define LOG_ERROR(msg) Logger::_error(msg)
  #define LOG_DEBUG(msg) Logger::_debug(msg)
#else
  #define LOG_INFO(msg)
  #define LOG_WARN(msg)
  #define LOG_ERROR(msg)
  #define LOG_DEBUG(msg)
#endif

class Logger {
public:
  // Add a sensitive string to the redaction list
  static void registerSecret(const String& secret);

  // Core logging functions (internal use only, use macros instead)
  static void _info(const String& message);
  static void _warn(const String& message);
  static void _error(const String& message);
/**
 * @brief Debug
 * @param message
 */
  static void _debug(const String& message);

private:
  static std::vector<String> secrets;
/**
 * @brief Print redacted
 * @param level
 * @param message
 */
  static void printRedacted(const String& level, const String& message);
/**
 * @brief Redact
 * @param message
 * @return return value
 */
  static String redact(const String& message);
};
