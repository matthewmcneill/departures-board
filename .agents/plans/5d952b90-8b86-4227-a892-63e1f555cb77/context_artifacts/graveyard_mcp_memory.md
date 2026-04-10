# Graveyard: Memory Exhaustion

## Single Monolithic Allocation
Spooling arrays in RAM (like using a single large `DynamicJsonDocument` response dump for `tools/list`) guarantees a crash within the ESP32 `async_tcp` stack due to limited heap boundaries. This approach was explicitly abandoned in the planning stage. 
Do not bypass the `Print&` stream execution pipeline. Always chunk directly downstream.
