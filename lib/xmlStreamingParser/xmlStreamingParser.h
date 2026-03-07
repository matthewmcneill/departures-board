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
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - xmlStreamingParser: Class definition
 * - state_Begin: State_ begin
 * - state_StartTag: State_ start tag
 * - state_TagName: State_ tag name
 * - state_EmptyTag: State_ empty tag
 * - state_TagContents: State_ tag contents
 * - state_Attribute: State_ attribute
 * - state_EndTag: State_ end tag
 * - state_CDATA: State_ c d a t a
 * - state_Comment: State_ comment
 * - ContextBufferAddChar: context buffer add char
 * - ChangeState: change state
 * - parse: Parse
 * - setListener: Set listener
 * - reset: Reset
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
 * @brief State_ begin
 * @param character
 */
    void state_Begin(const char character);
/**
 * @brief State_ start tag
 * @param character
 */
    void state_StartTag(const char character);
/**
 * @brief State_ tag name
 * @param character
 */
    void state_TagName(const char character);
/**
 * @brief State_ empty tag
 * @param character
 */
    void state_EmptyTag(const char character);
/**
 * @brief State_ tag contents
 * @param character
 */
    void state_TagContents(const char character);
/**
 * @brief State_ attribute
 * @param character
 */
    void state_Attribute(const char character);
/**
 * @brief State_ end tag
 * @param character
 */
    void state_EndTag(const char character);
/**
 * @brief State_ c d a t a
 * @param character
 */
    void state_CDATA(const char character);
/**
 * @brief State_ comment
 * @param character
 */
    void state_Comment(const char character);
/**
 * @brief context buffer add char
 * @param character
 */
    void ContextBufferAddChar(const char character);
/**
 * @brief change state
 * @param newState
 */
    void ChangeState(int newState);

  public:
    xmlStreamingParser();
/**
 * @brief Parse
 * @param character
 */
    void parse(const char character);
/**
 * @brief Set listener
 * @param listener
 */
    void setListener(xmlListener* listener);
/**
 * @brief Reset
 */
    void reset();

};
