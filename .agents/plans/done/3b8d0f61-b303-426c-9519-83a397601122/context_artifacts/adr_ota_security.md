# ADR: Validating Secure ESP OTA Pipelines

## Context
Deploying an internet-accessible ESP32 leaves it vulnerable to man-in-the-middle binary injections or firmware corruption caused by raw socket latency. Legacy implementations used naive MD5 file hashing that only verified size or download completion.

## Decision (mbedtls Streaming)
Instead of allocating 1.6MB to RAM (impossible) or caching it on SPIFFS only to read it again before flashing:
We implemented an inline stream using `Update.writeStream()`.
Simultaneously, chunk bytes are duplicated into `mbedtls_md_update()`.
Once the connection closes, the stream hashes the final context block.
We use an explicit detached signature `firmware.sig` fetched immediately *prior* to downloading the `firmware.bin`. If the signature doesn't pass `mbedtls_pk_verify` utilizing our hardcoded `publicKey.hpp` module, the `Update.abort()` is intentionally executed.

## Consequences
- Protects eFuses. If signature verification fails, the partition pointer map never flips. The system gracefully deletes the partial dirty flash and boots into the stable original firmware.
- Hardcoded 2048-bit AES guarantees authentic GitHub origins.
- Uses `std::unique_ptr` mapping for `MbedTLSOtaVerifier`, containing the ~8-12KB hash generation overhead safely and removing memory leaks.
