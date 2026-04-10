/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/mcpServer/mcpServer.hpp
 * Description: Core router for the onboard Model Context Protocol (MCP) server. Dispatches incoming JSON-RPC methods and tools safely via PROGMEM schemas.
 *
 * Exported Functions/Classes:
 * - mcpServer::init(): Initializes the MCP server tool registry.
 * - mcpServer::processPayload(): Central entry point for executing an MCP JSON-RPC call over a Print stream.
 */

#pragma once

#include <ArduinoJson.h>
#include <Print.h>

namespace mcpServer {

    /**
     * @brief Initializes the MCP tool registry and schema mapping
     */
    void init();

    /**
     * @brief Processes an incoming JSON-RPC payload, routing to standard MCP listing or execution
     * @param requestDoc The parsed JSON payload of the request
     * @param outputStream The abstract Arduino Print stream to write the JSON response to incrementally
     */
    void processPayload(const JsonDocument& requestDoc, Print& outputStream);

} // namespace mcpServer
