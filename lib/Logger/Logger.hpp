/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *
 * Module: lib/logger/logger.hpp
 * Description: Lightweight logging utility for ESP32 with color-coded levels.
 *
 * Exported Functions/Classes:
 * - LOG_ERROR(msg): Macro to log an error message.
 * - LOG_DEBUG(msg): Macro to log a debug message.
 */
#pragma once

#include <Arduino.h>
#include <vector>

#ifdef ENABLE_DEBUG_LOG
  #define LOG_INFO(sub, msg)  Logger::_info(sub, msg)
  #define LOG_WARN(sub, msg)  Logger::_warn(sub, msg)
  #define LOG_ERROR(sub, msg) Logger::_error(sub, msg)
  #define LOG_DEBUG(sub, msg) Logger::_debug(sub, msg)
  #define LOG_SPLASH(msg)     Logger::logSplashMessage(msg)
#else
  #define LOG_INFO(sub, msg)
  #define LOG_WARN(sub, msg)
  #define LOG_ERROR(sub, msg)
  #define LOG_DEBUG(sub, msg)
  #define LOG_SPLASH(msg)
#endif

class Logger {
public:
  // Initialize Serial
  static void begin(unsigned long baud = 115200);

  // Log framed splash message (only active if ENABLE_DEBUG_LOG)
  static void logSplashMessage(const char* message);

  // Add a sensitive string to the redaction list
  static void registerSecret(const String& secret);

  // Core logging functions (internal use only, use macros instead)
  static void _info(const char* category, const String& message);
  static void _info(const char* category, const char* message);
  static void _warn(const char* category, const String& message);
  static void _warn(const char* category, const char* message);
  static void _error(const char* category, const String& message);
  static void _error(const char* category, const char* message);
  static void _debug(const char* category, const String& message);
  static void _debug(const char* category, const char* message);

private:
  static std::vector<String> secrets;
/**
 * @brief Print redacted
 * @param icon
 * @param category
 * @param message
 */
  static void printRedacted(const String& icon, const char* category, const String& message);
  static void printRedacted(const String& icon, const char* category, const char* message);
/**
 * @brief Redact
 * @param message
 * @return return value
 */
  static String redact(const String& message);
};
