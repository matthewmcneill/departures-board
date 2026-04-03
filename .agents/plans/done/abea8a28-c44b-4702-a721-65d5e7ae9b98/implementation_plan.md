# Embedded String Optimization & Logging Fix

This plan addresses two critical architectural improvements related to embedded best practices: resolving macro-compiled logging and eradicating `Arduino String` references to mitigate heap fragmentation.

## 1. Architectural Assessment of `Arduino String`
**Finding**: The current `deviceCrypto` module passes and returns `Arduino String` payloads by-value during cryptography sequences (`encryptString`/`decryptString`). Furthermore, it relies on implicit `.length()` and string concatenations (`+`). 
**Impact**: Because the ESP32’s SRAM (320KB max) relies on contiguous heap blocks, repeated allocation and reallocation behind the scenes by `String` instances causes severe heap fragmentation. If a process requires an extra byte, the `String` object may completely duplicate its buffer, abandon the old one, and punch holes in the runtime memory map. Because these cryptographic functions are invoked asynchronously by web handlers and dynamically during boot cycles, `String` operations here threaten long-term stability.

**Best Practice Resolution**:
We must transition all cryptographic methods to **Caller-allocated bounded buffers** or **singular contiguous `malloc`/`free` operations**, removing `String` from `lib/deviceCrypto` completely.

## User Review Required
> [!IMPORTANT]
> Your suggestion to use **RAII** is excellent. Since the rest of the project actively isolates heavy transients onto the heap using `std::unique_ptr<T>`, we can perfectly apply `std::unique_ptr<char[]>` array definitions for our AES block returns. This ensures a clean, single-pass memory block is extracted from the heap and automatically destroyed when dropped by the caller, without risking an 8KB stack explosion on the web threads. Does the updated approach below look good?

## Proposed Changes

---

### lib/deviceCrypto
#### [MODIFY] `deviceCrypto.hpp`
- Replace `String` returns with RAII Managed Buffers:
`std::unique_ptr<char[]> encrypt(const char* plaintext, size_t inputLen, size_t* outLen);`
`std::unique_ptr<char[]> decrypt(const char* ciphertext, size_t inputLen, size_t* outLen);`

#### [MODIFY] `deviceCrypto.cpp`
- **Compiler Level Logging**: Fully switch from explicit `Logger::_info` overrides to macro-respectful syntax: `LOG_INFOf("CRYPTO", "Decoding payload: %u bytes", inputLen);` to execute Point 1 seamlessly.
- **Remove String Manipulation**: Refactor the methods internally to process streams entirely using the `uint8_t*` output structs, eliminating the `result = String(buf)` assignments. We will execute `mbedtls_base64_encode` straight into the bounded caller arrays.

---

### modules/wifiManager
#### [MODIFY] `wifiManager.cpp`
- Modify `loadWiFiConfig()` to replace `File.readString()` with bounded `File.readBytes()` or stream to strings temporarily, pushing the stream to `cryptoEngine->decrypt()`.
- The returned `std::unique_ptr<char[]>` is passed directly to `deserializeJson` perfectly cleanly, and RAII drops the payload instantly when exiting scopes.
- Adjust `saveWiFiConfig()` similarly. Serialize JSON to `measureJson`, dump to temporary managed buffer, encrypt to a RAII buffer, and feed back.

---

### modules/configManager
#### [MODIFY] `configManager.cpp`
- Identical RAII pointer interactions replacing Arduino String interactions when saving and loading `/apikeys.bin`. Memory limit bound will be strictly guarded by `measureJson` and `std::nothrow` initializations.

## Open Questions
- None pending. If approved, we will execute the implementation exactly as described.

## Verification Plan
### Automated Tests
1. Compilation check (`pio run`)
2. Memory profiling via log verification explicitly watching for heap fragmentation block leaks.

### Manual Verification
1. Flash the device.
2. Ensure both the legacy migration format works, and subsequent device reloads natively deserialize the `.bin` using only raw C-Buffers.
3. Call the web API to save the config, confirming `configManager` processes the string safely.
