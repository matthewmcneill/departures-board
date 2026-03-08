/*
 * XML Streaming Parser Library
 *  - based on the structure of samxl embedded XML parser by Zorxx Software at https://github.com/zorxx/saxml
 *
 * MIT License
 *
 * Copyright (c) 2025-2026 Gadec Software
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 * Module: lib/xmlStreamingParser/xmlStreamingParser.h
 * Description: A state-machine-based XML parser designed for low-memory streaming applications over HTTP.
 *
 * Exported Functions/Classes:
 * - class xmlStreamingParser: Emits XML tags and data events to an attached xmlListener.
 *   - xmlStreamingParser(): Constructor.
 *   - parse(): Feeds a single byte into the streaming parser's state machine.
 *   - setListener(): Attaches an interface implementation to handle XML parsing events.
 *   - reset(): Reinitializes the parser state back to zero to begin a new document.
 */
#pragma once

#include <Arduino.h>
#include <xmlListener.h>

#define XML_BUFFER_MAX_LENGTH 450

#define STATE_NULL 0
#define STATE_BEGIN 1
#define STATE_STARTTAG 2
#define STATE_TAGNAME 3
#define STATE_TAGCONTENTS 4
#define STATE_ENDTAG 5
#define STATE_ATTRIBUTE 6
#define STATE_CDATA 7
#define STATE_COMMENT 8

class xmlStreamingParser {
  private:

    int state;
    int nextState;
    xmlListener* myListener;

    char buffer[XML_BUFFER_MAX_LENGTH];
    //char currentTagName[128]; // Used for endTag with self-closing tags. We don't need it, save some RAM.

    bool bInitialize;   // True for the first call into a state
    bool inAttrQuote = false; // true if we're inside a quoted attribute string
    uint32_t length;
    char cdataMatch[10];
    uint8_t cdataIndex;

/**
 * @brief Handles characters while waiting for an opening tag '<'.
 * @param character Next byte from stream.
 */
    void state_Begin(const char character);

/**
 * @brief Triggered after reading '<', determines if it's a start tag, end tag, or CDATA/Comment.
 * @param character Next byte from stream.
 */
    void state_StartTag(const char character);

/**
 * @brief Accumulates the tag name and detects self-closing tags or attributes.
 * @param character Next byte from stream.
 */
    void state_TagName(const char character);

/**
 * @brief Handled empty tags (currently unused directly, handled by TagName/Attribute).
 * @param character Next byte from stream.
 */
    void state_EmptyTag(const char character);

/**
 * @brief Reads the inner text content of an XML node until a new '<' is found.
 * @param character Next byte from stream.
 */
    void state_TagContents(const char character);

/**
 * @brief Parses attributes defined within a tag.
 * @param character Next byte from stream.
 */
    void state_Attribute(const char character);

/**
 * @brief Handled the closing of a tag '/>' or '>'.
 * @param character Next byte from stream.
 */
    void state_EndTag(const char character);

/**
 * @brief Processes characters within a CDATA block.
 * @param character Next byte from stream.
 */
    void state_CDATA(const char character);

/**
 * @brief Processes characters within an XML comment block.
 * @param character Next byte from stream.
 */
    void state_Comment(const char character);

/**
 * @brief Accumulates characters into the internal tag/text buffer.
 * @param character The character to add.
 */
    void ContextBufferAddChar(const char character);

/**
 * @brief Transitions the parser's internal state machine to a new state.
 * @param newState The target state definition.
 */
    void ChangeState(int newState);

  public:
    xmlStreamingParser();
/**
 * @brief Feeds a single byte into the streaming parser's state machine.
 * @param character Next byte from stream.
 */
    void parse(const char character);

/**
 * @brief Attaches an interface implementation to handle XML parsing events.
 * @param listener Pointer to the xmlListener implementation.
 */
    void setListener(xmlListener* listener);

/**
 * @brief Reinitializes the parser state back to zero to begin a new document.
 */
    void reset();

};
