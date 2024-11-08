// Copyright (c) 2014-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CRYPTO_SHA256_H
#define BITCOIN_CRYPTO_SHA256_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

/** A hasher class for SHA-256. */
class NeuralLeadQHash_iface
{
private:
    uint32_t s[8];
    unsigned char buf[64];
    uint64_t bytes;

    bool _UseGPU = false;

    uint8_t* aiai = nullptr;
    uint8_t* oioi = nullptr;
    uint8_t* hhh = nullptr;

public:
    static const size_t OUTPUT_SIZE = 32;

    NeuralLeadQHash_iface(bool UseGPU = false);
    NeuralLeadQHash_iface& Write(const unsigned char* data, size_t len);
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
    NeuralLeadQHash_iface& Reset();
};

/** Autodetect the best available SHA256 implementation.
 *  Returns the name of the implementation.
 */
std::string SHA256AutoDetect();

/** Compute multiple double-SHA256's of 64-byte blobs.
 *  output:  pointer to a blocks*32 byte output buffer
 *  input:   pointer to a blocks*64 byte input buffer
 *  blocks:  the number of hashes to compute.
 */
void SHA256D64(unsigned char* output, const unsigned char* input, size_t blocks);

#endif // BITCOIN_CRYPTO_SHA256_H
