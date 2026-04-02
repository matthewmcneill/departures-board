/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *
 * Module: lib/logger/logger.hpp
 * Description: Lightweight logging utility for ESP32 with color-coded levels and secret redaction.
 *
 * Exported Functions/Classes:
 * - Logger: Static utility class for system-wide logging.
 *   - begin(): Initializes Serial communication.
 *   - logSplashMessage(): Prints a framed splash message.
 *   - registerSecret(): Adds a string to the redaction list.
 *   - _info(), _warn(), _error(), _debug(): Internal logging handlers.
 *
 * Macros:
 * - LOG_BEGIN(baud): Macro-guarded Serial initialization.
 * - LOG_REGISTER_SECRET(secret): Macro-guarded secret registration.
 * - LOG_ERROR(sub, msg): Macro-guarded error logging.
 * - LOG_WARN(sub, msg): Macro-guarded warning logging.
 * - LOG_INFO(sub, msg): Macro-guarded info logging.
 * - LOG_DEBUG(sub, msg): Macro-guarded debug logging.
 * - LOG_VERBOSE(sub, msg): Macro-guarded verbose (Level 5) logging.
 * - LOG_SPLASH(msg): Macro-guarded splash message logging.
 */
#pragma once

#include <Arduino.h>
#include <vector>

#ifndef CORE_DEBUG_LEVEL
#define CORE_DEBUG_LEVEL 0
#endif

#if CORE_DEBUG_LEVEL >= 1
  #define LOG_ERROR(sub, msg)           Logger::_error(sub, msg)
  #define LOG_BEGIN(baud)               Logger::begin(baud)
  #define LOG_REGISTER_SECRET(secret)    Logger::registerSecret(secret)
#else
  #define LOG_ERROR(sub, msg)
  #define LOG_BEGIN(baud)
  #define LOG_REGISTER_SECRET(secret)
#endif

#if CORE_DEBUG_LEVEL >= 2
  #define LOG_WARN(sub, msg)  Logger::_warn(sub, msg)
#else
  #define LOG_WARN(sub, msg)
#endif

#if CORE_DEBUG_LEVEL >= 3
  #define LOG_INFO(sub, msg)  Logger::_info(sub, msg)
  #define LOG_SPLASH(msg)     Logger::logSplashMessage(msg)
#else
  #define LOG_INFO(sub, msg)
  #define LOG_SPLASH(msg)
#endif

#if CORE_DEBUG_LEVEL >= 4
  #define LOG_DEBUG(sub, msg) Logger::_debug(sub, msg)
#else
  #define LOG_DEBUG(sub, msg)
#endif

#if CORE_DEBUG_LEVEL >= 5
  #define LOG_VERBOSE(sub, msg) Logger::_verbose(sub, msg)
#else
  #define LOG_VERBOSE(sub, msg)
#endif

/**
 * @brief Static utility class for system-wide logging with redaction support.
 */
class Logger {
public:
  /**
   * @brief Initializes the Serial port and waits for it to stabilize.
   * @param baud The baud rate for the Serial connection (Default: 115200).
   */
  static void begin(unsigned long baud = 115200);

  /**
   * @brief Logs a framed splash message to the Serial console.
   * @param message The text to be framed.
   */
  static void logSplashMessage(const char* message);

  /**
   * @brief Registers a sensitive string (e.g., API key) to be redacted from all future log output.
   * @param secret The plaintext string that should not appear in the logs.
   */
  static void registerSecret(const String& secret);

  /**
   * @brief Logs an informational message. Called internally by the LOG_INFO macro.
   * @param category Subsystem or tag.
   * @param message Message content.
   */
  static void _info(const char* category, const String& message);
  static void _info(const char* category, const char* message);

  /**
   * @brief Logs a warning message. Called internally by the LOG_WARN macro.
   * @param category Subsystem or tag.
   * @param message Message content.
   */
  static void _warn(const char* category, const String& message);
  static void _warn(const char* category, const char* message);

  /**
   * @brief Logs an error message. Called internally by the LOG_ERROR macro.
   * @param category Subsystem or tag.
   * @param message Message content.
   */
  static void _error(const char* category, const String& message);
  static void _error(const char* category, const char* message);

  /**
   * @brief Logs a debug message. Called internally by the LOG_DEBUG macro.
   * @param category Subsystem or tag.
   * @param message Message content.
   */
  static void _debug(const char* category, const String& message);
  static void _debug(const char* category, const char* message);

  /**
   * @brief Logs a verbose message. Called internally by the LOG_VERBOSE macro.
   * @param category Subsystem or tag.
   * @param message Message content.
   */
  static void _verbose(const char* category, const String& message);
  static void _verbose(const char* category, const char* message);

private:
  static std::vector<String> secrets; ///< Registry of sensitive strings
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
