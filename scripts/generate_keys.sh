#!/bin/bash
# scripts/generate_keys.sh
# 
# Departures Board (c) 2025-2026 Gadec Software
# Refactored for v3.0 by Matt McNeill 2026 CB Labs

set -e

mkdir -p src

echo "Generating private_key.pem (You will be prompted for an AES-256 password)"
# Use RSA-2048 which balances hardware performance and security perfectly for the ESP32
openssl genrsa -aes256 -out private_key.pem 2048

echo "Extracting public key to src/publicKey.hpp"
openssl rsa -in private_key.pem -pubout -out public_key.pem

cat << 'EOF' > src/publicKey.hpp
/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: src/publicKey.hpp
 * Description: Auto-generated cryptographic public key for OTA payload verification
 *
 * Exported Functions/Classes:
 * - OtaPublicKey: Constant holding the RSA-2048 public key in PEM format
 */

#pragma once

const char* const OtaPublicKey =
EOF

# Format the PEM block as safe C++ string literals
sed -e 's/^/"/' -e 's/$/\\n"/' public_key.pem >> src/publicKey.hpp

echo ";" >> src/publicKey.hpp

rm public_key.pem

echo "Generation complete (src/publicKey.hpp). KEEP private_key.pem ABSOLUTELY SAFE!"
