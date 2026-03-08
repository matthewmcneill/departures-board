/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *
 * Module: src/Logger.cpp
 * Description: Implementation of application logging interface with automatic sensitive data redaction.
 *
 * Exported Functions/Classes:
 * - Logger::registerSecret: Add a sensitive string to the redaction list.
 * - Logger::_info: Core info logging function (internal use only, called via macros).
 * - Logger::_warn: Core warn logging function (internal use only, called via macros).
 * - Logger::_error: Core error logging function (internal use only, called via macros).
 * - Logger::_debug: Core debug logging function (internal use only, called via macros).
 */
#include "Logger.hpp"

std::vector<String> Logger::secrets;

/**
 * @brief Registers a sensitive string (e.g., API key) to be redacted from all future log output.
 * @param secret The plaintext string that should not appear in the logs.
 */
void Logger::registerSecret(const String& secret) {
  if (secret.length() > 0) {
    secrets.push_back(secret);
  }
}

/**
 * @brief Scans a log message and replaces any registered secrets with asterisks.
 * @param message The original unredacted log string.
 * @return A safe version of the string with sensitive data hidden.
 */
String Logger::redact(const String& message) {
  if (secrets.empty()) return message;

  String redactedMessage = message;
  for (const String& secret : secrets) {
    if (secret.length() > 0 && redactedMessage.indexOf(secret) >= 0) {
      redactedMessage.replace(secret, "***REDACTED***");
    }
  }
  return redactedMessage;
}

/**
 * @brief Internal function that formats, redacts, and outputs the final log message to Serial.
 * @param level A string representing the severity level (e.g. "INFO", "ERROR").
 * @param message The raw message to be logged.
 */
void Logger::printRedacted(const String& level, const String& message) {
#ifdef ENABLE_DEBUG_LOG
  String output = "[" + level + "] " + redact(message);
  Serial.println(output);
#endif
}

/**
 * @brief Logs an informational message. Called internally by the LOG_INFO macro.
 * @param message The information to log.
 */
void Logger::_info(const String& message) {
  printRedacted("INFO", message);
}

/**
 * @brief Logs a warning message. Called internally by the LOG_WARN macro.
 * @param message The warning to log.
 */
void Logger::_warn(const String& message) {
  printRedacted("WARN", message);
}

/**
 * @brief Logs an error message. Called internally by the LOG_ERROR macro.
 * @param message The error to log.
 */
void Logger::_error(const String& message) {
  printRedacted("ERROR", message);
}

/**
 * @brief Logs a debug message. Called internally by the LOG_DEBUG macro.
 * @param message The debug information to log.
 */
void Logger::_debug(const String& message) {
  printRedacted("DEBUG", message);
}
