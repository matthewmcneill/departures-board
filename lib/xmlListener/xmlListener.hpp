/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * xmlListener Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/xmlListener/xmlListener.h
 * Description: Abstract interface for consuming streamed XML parsing events.
 *
 * Exported Functions/Classes:
 * - class xmlListener: Defines the callback interface for handling XML tags, text, and attributes.
 *   - startTag(): Triggered when a new opening XML tag is encountered.
 *   - endTag(): Triggered when a closing XML tag is encountered.
 *   - parameter(): Triggered when an entity or parameter is encountered.
 *   - value(): Triggered when inner text/value of an XML node is parsed.
 *   - attribute(): Triggered when an attribute of an XML tag is parsed.
 */

#pragma once

class xmlListener {
  private:

  public:

/**
     * @brief Triggered when a new opening XML tag is encountered.
     * @param tagName The name of the parsed tag.
     */
    virtual void startTag(const char *tagName) = 0;

    /**
     * @brief Triggered when a closing XML tag is encountered.
     * @param tagName The name of the parsed closing tag.
     */
    virtual void endTag(const char *tagName) = 0;

    /**
     * @brief Triggered when an entity or parameter is encountered.
     * @param param The parameter string.
     */
    virtual void parameter(const char *param) = 0;

    /**
     * @brief Triggered when inner text/value of an XML node is parsed.
     * @param value The text value contained within an XML tag.
     */
    virtual void value(const char *value) = 0;

    /**
     * @brief Triggered when an attribute of an XML tag is parsed.
     * @param attribute The attribute string (e.g. name="value").
     */
    virtual void attribute(const char *attribute) = 0;
};
