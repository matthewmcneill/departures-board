# OTA Architecture & Deployment Strategy

## 1. Introduction
The Departures Board implements an Over-The-Air (OTA) architecture designed to minimize user friction while safely managing firmware distribution. The goal is to provide a "zero-touch" appliance experience where the device silently detects, downloads, and flashes firmware over its 2.4GHz WiFi connection, without requiring the user to physically connect via USB or manually initiate updates.

## 2. OTA Lifecycle and Partitioning
The ESP32 manages OTA updates safely by avoiding destructive, in-place overwriting. It utilizes dual application partitions (`App 0` and `App 1`) alongside a bootloader that directs traffic.

**The "Ping-Pong" Flash Mechanism:**
1. **Active Execution:** The device boots and executes out of its primary active partition (e.g., `App 0`).
2. **Background Download:** When an update is triggered, the `HTTPUpdateGitHub` engine routes the incoming binary stream directly into the *inactive* partition (e.g., `App 1`), while the device continues to run out of `App 0`.
3. **Validation & Handoff:** Once the 1.5MB binary is fully written to `App 1`, the firmware runs verification checksums. 
4. **Boot Swap:** If successful, the firmware calls `esp_ota_set_boot_partition()`. This flags a single bit instructing the hardware bootloader to boot from `App 1` on the next reboot.
5. **Reboot:** `ESP.restart()` is called. The device boots into `App 1` running the new firmware. The old firmware remains completely untouched in `App 0` until the next update overwrites it.

### Automatic and Manual Rollbacks
Because the "old" firmware is physically preserved in the inactive partition, the architecture natively supports instant rollbacks:
- **Panic Protection:** If a bug in the new firmware causes an immediate crash or endless reboot loop within the first 5 minutes, the hardware watchdog forcibly resets the bootloader flag, rolling back to the previous partition automatically.
- **Manual Rollback:** Through the Web Portal API (`/api/ota/rollback`), users can explicitly flip the boot flag back to the older partition, restoring functionality within seconds without needing a computer.

## 3. GitHub Integration
Releases are hosted purely on GitHub Releases. 
- The system checks the `api.github.com/repos/gadec-uk/departures-board/releases/latest` endpoint using a highly memory-efficient `JsonStreamingParser` attached to the `githubClient`.
- Instead of using a bloated 10KB+ JSON buffer which would trigger Out-of-Memory kernel panics, the system streams the JSON byte-by-byte looking for the `tag_name` and the `firmware.bin` asset URL.
- The `otaUpdater` isolates checking to a specified "Quiet Hour" (e.g., 3:00 AM) to ensure updates do not commandeer the screen during active viewing hours.

## 4. Security Philosophy: Usability vs. Strict Cryptography
The architecture currently prioritizes operational simplicity, accepting minor Man-In-The-Middle (MITM) vulnerabilities as a calculated tradeoff for a tokenless, maintenance-free open-source product.

### The "Insecure" Transport Decision
When pulling the binary payload from GitHub, the firmware invokes `client->setInsecure();`. This instructs the ESP32 to encrypt the traffic via HTTPS, but deliberately bypass checking GitHub's `DigiCert` Root CA certificate identity.

**Why bypass certificate checking?**
Root CA Certificates expire (usually every 10 to 20 years). If a Root CA is hardcoded into the firmware, and it eventually expires or is revoked, the ESP32 will instantly reject all future connections to GitHub. The device would be trapped in a permanent catch-22: *It cannot verify the connection to download a new Certificate, because the old Certificate has expired.* Fixing it would require every user to dismantle their hardware and manually re-flash over USB. By bypassing transport identity, we guarantee the appliance will outlive certificate lifecycles indefinitely.

### Mitigation via Software-Level Signing (Future Proofing)
Because `setInsecure()` leaves the device vulnerable to a malicious local network entity spoofing the GitHub DNS to serve modified firmware patches, the system employs **Application-Level Cryptographic Verification**:
1. OpenSSL is used during the Continuous Integration (CI) build scripts (`scripts/deploy_release.py`) to generate an RSA/SHA-256 `.sig` signature for the compiled binary via an AES-encrypted private key. 
2. The `.sig` file is downloaded to the ESP32 RAM *before* the main binary is streamed.
3. The OTA framework hashes the downloaded binary stream using `mbedtls` and verifies it against the downloaded `.sig` via an embedded Public Key.
4. If a local hacker spoofs the DNS, they do not possess the AES-encrypted private key, and cannot forge a valid `.sig` file. The flashing aborts, isolating the malicious firmware to the inactive partition and restarting safely into the active zone. 

This hybrid architecture preserves the zero-maintenance resilience of the hardware while entirely neutralizing the payload tampering threats inherent in `setInsecure()`.
