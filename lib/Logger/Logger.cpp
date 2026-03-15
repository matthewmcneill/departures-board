/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/logger/logger.cpp
 * Description: Implementation of the lightweight logging utility.
 *
 * Exported Functions/Classes:
 * - Logger: Static utility class for system-wide logging.
 *   - begin(): Initializes Serial communication.
 *   - logSplashMessage(): Prints a framed splash message.
 *   - registerSecret(): Adds a string to the redaction list.
 *   - redact(): Filters sensitive strings from logs.
 */

#include <logger.hpp>

std::vector<String> Logger::secrets; // Registry of sensitive strings to be redacted

/**
 * @brief Initializes the Serial port and waits for it to stabilize.
 * @param baud The baud rate for the Serial connection.
 */
void Logger::begin(unsigned long baud) {
  Serial.begin(baud);
  delay(1000);
}

/**
 * @brief Logs a framed splash message to the Serial console.
 * @param message The text to be framed.
 */
void Logger::logSplashMessage(const char* message) {
#ifdef ENABLE_DEBUG_LOG
  if (message == nullptr) return;
  size_t len = strlen(message);
  
  Serial.println("");
  for (size_t i = 0; i < len + 8; i++) Serial.print("#");
  Serial.println("");
  Serial.print("### ");
  Serial.print(message);
  Serial.println(" ###");
  for (size_t i = 0; i < len + 8; i++) Serial.print("#");
  Serial.println("\n");
#endif
}

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
 * @brief Internal function that formats, redacts, and outputs the final log message to Serial (String version).
 * @param icon An emoji icon representing the severity.
 * @param category The subsystem or tag.
 * @param message The raw message to be logged.
 */
void Logger::printRedacted(const String& icon, const char* category, const String& message) {
#ifdef ENABLE_DEBUG_LOG
  Serial.print(icon);
  Serial.print(" [");
  Serial.print(category);
  Serial.print("] ");
  Serial.println(redact(message));
#endif
}

/**
 * @brief Internal function that formats and outputs the final log message to Serial (Literal version).
 * Bypasses String allocation and redaction if no secrets are registered for maximum efficiency.
 * @param icon An emoji icon representing the severity.
 * @param category The subsystem or tag.
 * @param message The raw literal string to be logged.
 */
void Logger::printRedacted(const String& icon, const char* category, const char* message) {
#ifdef ENABLE_DEBUG_LOG
  Serial.print(icon);
  Serial.print(" [");
  Serial.print(category);
  Serial.print("] ");
  if (secrets.empty()) {
    Serial.println(message);
  } else {
    Serial.println(redact(String(message)));
  }
#endif
}

/**
 * @brief Logs an informational message. Called internally by the LOG_INFO macro.
 */
void Logger::_info(const char* category, const String& message) {
  printRedacted("🔘", category, message);
}

void Logger::_info(const char* category, const char* message) {
  printRedacted("🔘", category, message);
}

/**
 * @brief Logs a warning message. Called internally by the LOG_WARN macro.
 */
void Logger::_warn(const char* category, const String& message) {
  printRedacted("🟡", category, message);
}

void Logger::_warn(const char* category, const char* message) {
  printRedacted("🟡", category, message);
}

/**
 * @brief Logs an error message. Called internally by the LOG_ERROR macro.
 */
void Logger::_error(const char* category, const String& message) {
  printRedacted("🔴", category, message);
}

void Logger::_error(const char* category, const char* message) {
  printRedacted("🔴", category, message);
}

/**
 * @brief Logs a debug message. Called internally by the LOG_DEBUG macro.
 */
void Logger::_debug(const char* category, const String& message) {
  printRedacted("🔵", category, message);
}

void Logger::_debug(const char* category, const char* message) {
  printRedacted("🔵", category, message);
}
