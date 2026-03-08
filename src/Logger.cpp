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
 * @brief Register secret
 * @param secret
 */
void Logger::registerSecret(const String& secret) {
  if (secret.length() > 0) {
    secrets.push_back(secret);
  }
}

/**
 * @brief Redact
 * @param message
 * @return return value
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
 * @brief Print redacted
 * @param level
 * @param message
 */
void Logger::printRedacted(const String& level, const String& message) {
#ifdef ENABLE_DEBUG_LOG
  String output = "[" + level + "] " + redact(message);
  Serial.println(output);
#endif
}

/**
 * @brief Info
 * @param message
 */
void Logger::_info(const String& message) {
  printRedacted("INFO", message);
}

/**
 * @brief Warn
 * @param message
 */
void Logger::_warn(const String& message) {
  printRedacted("WARN", message);
}

/**
 * @brief Error
 * @param message
 */
void Logger::_error(const String& message) {
  printRedacted("ERROR", message);
}

/**
 * @brief Debug
 * @param message
 */
void Logger::_debug(const String& message) {
  printRedacted("DEBUG", message);
}
