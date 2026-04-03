/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/deviceCrypto/deviceCrypto.hpp
 * Description: Generic cryptographic utility module providing mbedtls AES-256-CBC 
 *              encryption and decryption bounded to a persistent hardware key.
 *
 * Exported Functions/Classes:
 * - DeviceCrypto: [Class] Service provider for AES string encryption.
 *   - begin(): Initializes the AES master key from NVS, generating one if missing.
 *   - encryptString(): Encrypts a plaintext String to a Base64-encoded ciphertext String.
 *   - decryptString(): Decrypts a Base64-encoded ciphertext String to plaintext.
 */

#ifndef DEVICE_CRYPTO_HPP
#define DEVICE_CRYPTO_HPP

#include <Arduino.h>
#include <memory>

/**
 * @brief Infrastructure wrapper class that manages hardware-bound AES-256 keys
 *        and securely encrypts and decrypts application strings (e.g. WiFi, API tokens).
 */
class DeviceCrypto {
private:
    uint8_t aesKey[32];     ///< 256-bit AES Master Key permanently held in RAM
    bool keyLoaded = false; ///< Flag indicating if the AES key is active in memory

    /**
     * @brief Padds a payload Buffer to adhere to the 16-byte CBC block size requirement.
     * @param buffer Raw pointer to the array holding the payload.
     * @param originalLen Native string length.
     * @param paddedLen Modulo 16 target padding length.
     */
    void applyPKCS7Padding(uint8_t* buffer, size_t originalLen, size_t paddedLen);
    
    /**
     * @brief Removes padding post-decryption.
     * @param buffer Raw pointer to the padded decrypt stream.
     * @param paddedLen Stream total length.
     * @return Decoded valid string span.
     */
    size_t removePKCS7Padding(const uint8_t* buffer, size_t paddedLen);

public:
    DeviceCrypto() = default;
    ~DeviceCrypto() = default;

    /**
     * @brief Mounts the NVS partition to extract the master encryption key.
     *        If no key is configured, uses esp_random() to generate and save one.
     */
    void begin();

    /**
     * @brief Encrypts plaintext into a base64 encoded payload using AES-256-CBC.
     * @param plaintext The raw string you wish to secure.
     * @param inputLen Length of the plaintext.
     * @param outLen Pointer to receive the exported Base64 output size.
     * @return Managed RAII pointer containing the Base64 ciphertext, or nullptr on failure.
     */
    std::unique_ptr<char[]> encrypt(const char* plaintext, size_t inputLen, size_t* outLen);

    /**
     * @brief Decrypts a base64 encoded ciphertext payload back into plaintext.
     * @param ciphertext The base64-encoded incoming char buffer.
     * @param inputLen Length of the base64 ciphertext.
     * @param outLen Pointer to receive the verified plaintext string size.
     * @return Managed RAII pointer containing the plaintext string, or nullptr on failure.
     */
    std::unique_ptr<char[]> decrypt(const char* ciphertext, size_t inputLen, size_t* outLen);
};

#endif
