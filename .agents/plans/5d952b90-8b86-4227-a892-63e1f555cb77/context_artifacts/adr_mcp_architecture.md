# Architecture Decision Record: ESP32 MCP Server

## Dependency Inversion
`mcpServer` will NOT inherently import `ESPAsyncWebServer.h`. Following OO rules, `webHandlerManager` maintains HTTP routing logic entirely on its own, parsing incoming JSON-RPC calls and passing the lightweight input alongside an abstract `Print&` object down to `mcpServer::processPayload()`. This isolates the network layer and allows execution via serial logs if ever migrated, while saving RAM.

## Phased Approach Shift
We shifted agent skill development (`device-mcp-client`) to Phase 2 to allow fast execution iteration. Discovery scripts will query `pio-manager` log cache to natively self-hydrate device IP without user prompt. 

## Memory Constraints
PROGMEM chunking is used for JSON schemas instead of caching in RAM.
