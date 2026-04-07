# **Design Specification: Device-Side ESP32 MCP Server**

## **1\. Context & Objective**

The objective is to implement a lightweight Model Context Protocol (MCP) server directly on an ESP32 firmware running within a PlatformIO environment.

This server will allow an AI agent (running in Google Antigravity) to dynamically call hardware functions (e.g., read sensors, actuate pins, query state) over the local WiFi network using standard JSON-RPC 2.0 requests, bypassing the need for serial monitor polling or continuous firmware recompilation.

**Baseline Assets (Already available in firmware):**

* Active WiFi connection layer.  
* ESPAsyncWebServer library (handles HTTP).  
* JSON Parser (assumed ArduinoJson).  
* Current firmware footprint: \~950KB.  
* **Hard Constraint:** The standard OTA update partition MUST be preserved.

## **2\. Critical Constraints & Memory Architecture**

Because the firmware is already \~950KB and we must retain the default OTA-enabled partition table (which typically limits the application size to 1.2MB), we have a strict remaining budget of **\~250KB** for the MCP implementation. The agent MUST adhere to extreme memory management practices.

### **A. Flash Memory (The 250KB Budget)**

* **Directive:** Do NOT alter board\_build.partitions. The OTA partition remains intact.  
* **Directive:** All tool schemas, names, descriptions, and error messages MUST be stored strictly in Flash memory. They must never be instantiated as standard RAM-backed String objects.  
* **Implementation:** Use the PROGMEM attribute and the F() macro for all static JSON schema strings.

### **B. RAM / Heap (The Asynchronous Trap)**

ESPAsyncWebServer executes HTTP callbacks inside the async\_tcp task, which has a very limited stack size. Building massive JSON responses (like the tools/list schema dump) in a single RAM buffer will cause a crash.

* **Directive:** NEVER allocate a single large DynamicJsonDocument to hold the entire MCP response.  
* **Directive:** Utilize ArduinoJson's streaming capabilities combined with ESPAsyncWebServer's chunked response streams.  
* **Directive (Spooling from PROGMEM):** When sending large schemas (e.g., during tools/list), the server must spool these strings directly from PROGMEM into the AsyncResponseStream without copying them into intermediate RAM buffers.

**Code Pattern for Streaming JSON Response:**

AsyncResponseStream \*response \= request-\>beginResponseStream("application/json");  
// Serialize directly to the network buffer to keep RAM usage flat  
serializeJson(doc, \*response);   
request-\>send(response);

## **3\. Transport Layer Implementation (HTTP)**

While standard MCP often uses stdio, Network-based MCP relies on HTTP POST requests and Server-Sent Events (SSE). To keep it lightweight, we will implement a stateless HTTP POST endpoint for standard JSON-RPC calls.

### **Endpoint: POST /mcp**

The ESPAsyncWebServer will expose a single POST endpoint that accepts standard JSON-RPC 2.0 payloads.

**Expected Incoming Payload Example:**

{  
  "jsonrpc": "2.0",  
  "id": 1,  
  "method": "tools/call",  
  "params": {  
    "name": "toggle\_gpio",  
    "arguments": { "pin": 4, "state": true }  
  }  
}

## **4\. MCP Core Method Handlers**

The agent must implement handlers for the two primary MCP methods required for tool execution:

### **Method 1: tools/list**

This method returns the registry of available hardware tools.

* **Logic:** Iterate through a registered list of C++ structs/classes that define the tools.  
* **Memory Warning:** Stream this output iteratively. Read the JSON schemas directly from PROGMEM byte-by-byte into the network chunked response stream. Do not aggregate the schemas in RAM.

### **Method 2: tools/call**

This method maps the AI's requested tool name to the physical C++ hardware function.

* **Logic:** 1\. Parse the incoming arguments JSON object.  
  2\. Validate types (e.g., ensure pin is an integer).  
  3\. Execute the hardware function (e.g., digitalWrite(pin, state)).  
  4\. Return a standard MCP tool execution result array containing the text response.

**Expected Outgoing Response Example:**

{  
  "jsonrpc": "2.0",  
  "id": 1,  
  "result": {  
    "content": \[  
      {  
        "type": "text",  
        "text": "GPIO 4 successfully set to HIGH."  
      }  
    \]  
  }  
}

## **5\. Extensibility Architecture**

To keep the main.cpp clean, the agent should implement a modular tool registration system optimized for Flash.

* Create a base class or struct (e.g., McpTool) containing:  
  * const char\* name  
  * const char\* description\_progmem (Must be read via pgm\_read\_byte or cast to \_\_FlashStringHelper\*)  
  * const char\* json\_schema\_progmem (Must be read via pgm\_read\_byte or cast to \_\_FlashStringHelper\*)  
  * std::function\<String(JsonObject)\> execute\_callback  
* Maintain a std::vector\<McpTool\> or simple array of registered tools.  
* When a new hardware feature is added (e.g., an I2C sensor), the developer just pushes a new McpTool into the registry, ensuring the schema strings are wrapped in PROGMEM.

## **6\. Execution Steps for the AI Developer**

1. **Architecture Setup:** Create the McpTool registration structure. Ensure robust utility functions exist for streaming PROGMEM strings directly to an ESPAsyncWebServer response.  
2. **Route Implementation:** Implement the POST /mcp route handler using ESPAsyncWebServer.  
3. **Streaming Logic:** Implement the JSON-RPC parsing logic for tools/list and tools/call. Use ArduinoJson to stream the structural JSON, while manually spooling the heavy schema blocks from PROGMEM into the AsyncResponseStream.  
4. **Integration Test:** Create a "Hello World" tool (e.g., get\_device\_stats returning free heap and uptime) to test the end-to-end network execution and verify that the firmware compiles under the 1.2MB partition limit.