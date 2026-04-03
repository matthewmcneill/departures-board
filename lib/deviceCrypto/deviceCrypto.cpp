/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/deviceCrypto/deviceCrypto.cpp
 * Description: Generic cryptographic utility module providing mbedtls AES-256-CBC 
 *              encryption and decryption bounded to a persistent hardware key.
 */

#include "deviceCrypto.hpp"
#include <Preferences.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <esp_random.h>
#include <esp_log.h>
#include "../../lib/logger/logger.hpp"

/**
 * @brief Mounts the NVS partition to extract the master encryption key.
 *        If no key is configured, uses esp_random() to generate and save one.
 */
void DeviceCrypto::begin() {
    // --- Step 1: Open Secure Persistence ---
    // Access the dedicated 'secrets' namespace
    Preferences prefs;
    prefs.begin("secrets", false);
    
    // --- Step 2: Validate or Generate Hardware Root Key ---
    if (prefs.getBytesLength("aes_key") != 32) {
        // Generate new key
        for (int i = 0; i < 32; i++) {
            aesKey[i] = (uint8_t)(esp_random() % 256);
        }
        prefs.putBytes("aes_key", aesKey, 32);
        Logger::_info("CRYPTO", "Generated and firmly committed new 256-bit AES Hardware Key.");
    } else {
        prefs.getBytes("aes_key", aesKey, 32);
        Logger::_info("CRYPTO", "Mounted existing persistent AES Master Key.");
    }
    prefs.end();
    keyLoaded = true;
}

/**
 * @brief Padds a payload Buffer to adhere to the 16-byte CBC block size requirement.
 * @param buffer Raw pointer to the array holding the payload.
 * @param originalLen Native string length.
 * @param paddedLen Modulo 16 target padding length.
 */
void DeviceCrypto::applyPKCS7Padding(uint8_t* buffer, size_t originalLen, size_t paddedLen) {
    uint8_t paddingValue = paddedLen - originalLen;
    for (size_t i = originalLen; i < paddedLen; i++) {
        buffer[i] = paddingValue;
    }
}

/**
 * @brief Removes padding post-decryption.
 * @param buffer Raw pointer to the padded decrypt stream.
 * @param paddedLen Stream total length.
 * @return Decoded valid string span.
 */
size_t DeviceCrypto::removePKCS7Padding(const uint8_t* buffer, size_t paddedLen) {
    if (paddedLen == 0 || paddedLen % 16 != 0) return 0;
    uint8_t paddingValue = buffer[paddedLen - 1];
    
    // Basic verification against PKCS#7 format bounds
    if (paddingValue == 0 || paddingValue > 16) return paddedLen;
    
    // Verify all trailing pad bytes match the length
    for (size_t i = 0; i < paddingValue; i++) {
        if (buffer[paddedLen - 1 - i] != paddingValue) {
            return paddedLen; // Corrupted pad, fallback returning full blob
        }
    }
    return paddedLen - paddingValue;
}

/**
 * @brief Encrypts plaintext into a base64 encoded string using AES-256-CBC.
 * @param plaintext The raw string you wish to secure.
 * @param inputLen Length of the plaintext.
 * @param outLen Output parameter to capture the ciphertext length.
 * @return Managed unique pointer to Base64-encoded string containing IV + Ciphertext.
 */
std::unique_ptr<char[]> DeviceCrypto::encrypt(const char* plaintext, size_t inputLen, size_t* outLen) {
    if (!keyLoaded || inputLen == 0) {
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }

    LOG_INFOf("CRYPTO", "Executing AES-256-CBC Encryption. Plaintext payload size: %u bytes", inputLen);

    // --- Step 1: Allocate Encrypted Payload Buffer ---
    size_t paddedLen = inputLen + (16 - (inputLen % 16));
    
    // Allocate buffer for IV (16 bytes) + Padded Ciphertext
    size_t totalLen = 16 + paddedLen;
    uint8_t* buffer = (uint8_t*)malloc(totalLen);
    if (!buffer) {
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }
    
    // --- Step 2: Generate Initialization Vector ---
    for (int i = 0; i < 16; i++) {
        buffer[i] = (uint8_t)(esp_random() % 256);
    }
    
    // Move payload forward past the IV block and append PKCS7 padding
    memcpy(buffer + 16, plaintext, inputLen);
    applyPKCS7Padding(buffer + 16, inputLen, paddedLen);
    
    // --- Step 3: Core Cryptographic Transformation ---
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, aesKey, 256);
    
    uint8_t processingIv[16];
    memcpy(processingIv, buffer, 16);
    
    // In-place encryption
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLen, processingIv, buffer + 16, buffer + 16);
    mbedtls_aes_free(&aes);

    // --- Step 4: Base64 Final Rendering ---
    size_t b64Len = 0;
    mbedtls_base64_encode(nullptr, 0, &b64Len, buffer, totalLen); // Query lengths
    
    std::unique_ptr<char[]> result;
    if (b64Len > 0) {
        char* b64Buffer = new (std::nothrow) char[b64Len + 1];
        if (b64Buffer) {
            if (mbedtls_base64_encode((unsigned char*)b64Buffer, b64Len, &b64Len, buffer, totalLen) == 0) {
                b64Buffer[b64Len] = '\0';
                result.reset(b64Buffer);
                if (outLen) *outLen = b64Len;
                LOG_INFOf("CRYPTO", "Encryption completed securely. Export Base64 size: %u bytes", b64Len);
            } else {
                delete[] b64Buffer;
                if (outLen) *outLen = 0;
            }
        } else {
            if (outLen) *outLen = 0;
        }
    } else {
        if (outLen) *outLen = 0;
    }
    
    free(buffer);
    return result;
}

/**
 * @brief Decrypts a base64 encoded ciphertext string back into plaintext.
 * @param ciphertext The base64-encoded char buffer.
 * @param inputLen Length of the base64 ciphertext.
 * @param outLen Output parameter to capture the exact plaintext length.
 * @return Managed unique pointer to processed plaintext, or nullptr if decryption failed.
 */
std::unique_ptr<char[]> DeviceCrypto::decrypt(const char* ciphertext, size_t inputLen, size_t* outLen) {
    if (!keyLoaded || inputLen == 0) {
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }
    
    LOG_INFOf("CRYPTO", "Decoding inbound Base64 Ciphertext. Payload size: %u bytes", inputLen);

    // --- Step 1: Decode Incoming Base64 ---
    size_t decOutLen = 0;
    mbedtls_base64_decode(nullptr, 0, &decOutLen, (const unsigned char*)ciphertext, inputLen);
    
    if (decOutLen <= 16) {
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }

    uint8_t* buffer = (uint8_t*)malloc(decOutLen);
    if (!buffer) {
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }
    
    if (mbedtls_base64_decode(buffer, decOutLen, &decOutLen, (const unsigned char*)ciphertext, inputLen) != 0) {
        free(buffer);
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }
    
    // --- Step 2: Extract Vectors and Decrypt ---
    size_t ciphertextLen = decOutLen - 16;
    if (ciphertextLen % 16 != 0) {
        free(buffer);
        if (outLen) *outLen = 0;
        return std::unique_ptr<char[]>();
    }

    uint8_t iv[16];
    memcpy(iv, buffer, 16);
    
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, aesKey, 256);
    
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertextLen, iv, buffer + 16, buffer + 16);
    mbedtls_aes_free(&aes);

    // --- Step 3: Extract Plaintext Stream ---
    size_t unpaddedLen = removePKCS7Padding(buffer + 16, ciphertextLen);
    
    std::unique_ptr<char[]> result;
    if (unpaddedLen > 0 && unpaddedLen <= ciphertextLen) {
        char* plaintextBuf = new (std::nothrow) char[unpaddedLen + 1];
        if (plaintextBuf) {
            memcpy(plaintextBuf, buffer + 16, unpaddedLen);
            plaintextBuf[unpaddedLen] = '\0';
            result.reset(plaintextBuf);
            if (outLen) *outLen = unpaddedLen;
            LOG_INFOf("CRYPTO", "Decryption verified. Extracted plaintext configuration length: %u bytes", unpaddedLen);
        } else {
            if (outLen) *outLen = 0;
        }
    } else {
        LOG_ERROR("CRYPTO", "Padding validation failed post-decryption. Key mismatch or stream corrupted!");
        if (outLen) *outLen = 0;
    }
    
    free(buffer);
    return result;
}
