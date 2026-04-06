# Secure Release Cycle & Digitally Signed Firmware

Moving to a digitally signed release cycle protects your users from malicious updates while allowing you to keep the repository public and avoiding the destructive nature of hardware "Secure Boot" eFuses. 

Here is an exploration of how we would structure the scripts, the keys, and the release pipeline.

## 1. Generating Password-Protected Keys
We can create a setup script (e.g., `scripts/generate_keys.sh`) that makes use of OpenSSL.

- **The Private Key:** When generating the private key, we use AES-256 encryption. OpenSSL will prompt you to type a password. You must enter this password every time you release new firmware. The encrypted key (`private_key.pem`) remains safely on your laptop. If someone steals your laptop or the file, they cannot sign malware without the password.
- **The Public Key:** The script generates a `public_key.pem` and reformats it into a C++ header file (e.g., `src/publicKey.hpp`). This is compiled directly into the C++ firmware. Because it is public, it doesn't matter if everyone on GitHub can see it—they can only use it to check signatures, not forge them.

```bash
# Concept of key generation script
echo "Enter a secure password for the new Private Key:"
openssl genpkey -algorithm RSA -out private_key.pem -aes256 -pkeyopt rsa_keygen_bits:2048

echo "Generating Public Key Header for firmware..."
openssl rsa -pubout -in private_key.pem -out public_key.pem
# ... script converts public_key.pem into a const char* inside src/publicKey.hpp
```

## 2. Automated Release & Deployment Script
Once the C++ development is done, instead of manually dragging and dropping files onto GitHub, we will create a Python or Bash deployment script (e.g., `scripts/deploy_release.py`).

**The workflow of the script:**
1. **Extract Local Version:** It parses `src/departuresBoard.hpp` to extract the current `VERSION_MAJOR` and `VERSION_MINOR` of your local codebase.
2. **Poll GitHub:** It hits the `api.github.com/repos/gadec-uk/departures-board/releases/latest` endpoint.
3. **Incremental Validation:** It compares the API tag (e.g., `v3.1`) against your local parsed version (e.g., `v3.1.2`). If your local code is equal to or older than the GitHub release, the script aborts and tells you to bump the version number.
4. **Build Firmware:** If valid, it silently executes `pio run -e esp32dev` to compile the firmware.
5. **Cryptographic Signing (The Password Prompt):** The script executes OpenSSL to create a SHA-256 signature of the compiled `.bin` file using your encrypted key.
   ```bash
   openssl dgst -sha256 -sign private_key.pem -out firmware.sig .pio/build/esp32dev/firmware.bin
   ```
   *At this exact moment, your terminal will pause and prompt for the AES password you created in step 1.*
6. **Publish to GitHub:** The script uses the GitHub CLI (`gh`) or REST API to automatically create a draft or published release, uploading both `firmware.bin` and `firmware.sig` as release assets.

## 3. C++ Architectural Impacts (What the ESP32 must do)
Because we are circumventing the ESP32's hardware-level secure boot, we must implement "Application-Level" secure boot using the `mbedtls` cryptographic library already present in the ESP32 framework.

Currently, the updater blindly downloads and commits to flash:
```cpp
Update.begin(size);
Update.writeStream(client);
Update.end();
ESP.restart(); // Device trusts the code and runs it
```

**With the new release cycle, the OTA process changes drastically:**
1. The ESP32 downloads the tiny `.sig` file into RAM.
2. The ESP32 opens the `firmware.bin` download stream.
3. Because the ESP32 lacks the RAM to hold a 1.5MB binary, it streams chunks of the binary, simultaneously doing two things:
   - Writing the chunk to the inactive OTA flash partition.
   - Passing the chunk through an `mbedtls_sha256_update()` hashing calculation.
4. When the download finishes, `Update.end()` completes writing to flash.
5. **The Verification Gate:** Before restarting, the ESP32 runs `mbedtls_pk_verify()`, feeding the final calculated SHA-256 hash and the downloaded `.sig` against the `publicKey.hpp` embedded in its current code. 
6. If the signature matches, it flags the new OTA partition as active and reboots. If the math fails (meaning the binary was intercepted/corrupted, or signed with a different private key), it erases the partition, logs a security error, and aborts the reboot.

---
Let me know if this aligns with your vision for securing the release pipeline. If so, I can pause the previous API modifications and start formally writing the Python/Bash scripts for this deployment pipeline.
