# Task List: DeviceCrypto String Eradication

- `[/]` **Phase 1: Library Refactor**
  - `[ ]` Modify `lib/deviceCrypto/deviceCrypto.hpp` to expose `std::unique_ptr<char[]>` and `size_t` pointer sizes instead of `String`.
  - `[ ]` Modify `lib/deviceCrypto/deviceCrypto.cpp` to use `std::unique_ptr` array patterns and `std::nothrow` for internal Base64 allocations.
  - `[ ]` Ensure hardware logging fully leverages `LOG_INFOf` macro-guarded parameters instead of String concatenations.

- `[x]` **Phase 2: Configuration Migrations**
  - `[x]` Update `modules/wifiManager/wifiManager.cpp` to read into character streams and execute decryption over `std::unique_ptr<char[]>`.
  - `[x]` Update `modules/configManager/configManager.cpp` to save and load `/apikeys.bin` explicitly rejecting `String`.

- `[x]` **Phase 3: Verification**
  - `[x]` Flash to hardware, reboot.
  - `[x]` Validate that Web UI configuration saving operates safely under the 8KB bounds.
  - `[x]` Verify boot decryption still maps properly to the Wifi stack.
