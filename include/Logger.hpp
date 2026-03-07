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
 * - info: Info
 * - warn: Warn
 * - error: Error
 * - debug: Debug
 * - printRedacted: Print redacted
 * - redact: Redact
 */
#pragma once

#include <Arduino.h>
#include <vector>

class Logger {
public:
  // Add a sensitive string to the redaction list
  static void registerSecret(const String& secret);

  // Core logging functions
  static void info(const String& message);
  static void warn(const String& message);
  static void error(const String& message);
/**
 * @brief Debug
 * @param message
 */
  static void debug(const String& message);

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
