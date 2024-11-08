// Copyright (c) 2014-2019 The Bitcoin Core developers
// Copyright (c) 2024 SimonJRiddix & NeuralLead
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// First Edition with Italian Comments
// NeuralLead Hash + Quantum Computing Hash

#include <iomanip>
#include <iostream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__)
    #include <windows.h>
    //#define WINAPI true
    #define PATH_SEP "\\"
#else
    #include <dirent.h> // for *Nix directory access
    #include <unistd.h>
    #define PATH_SEP "/"
#endif

#include <sys/stat.h>
#include <sys/types.h>

bool file_exists(const char* file)
{
    if (file == NULL) { return false; }
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__)
        #if defined(WIN_API)
            // if you want the WinAPI, versus CRT
            if (strnlen(file, MAX_PATH+1) > MAX_PATH) {
                // ... throw error here or ...
                return false;
            }
            DWORD res = GetFileAttributesA(file);
            return (res != INVALID_FILE_ATTRIBUTES &&
                !(res & FILE_ATTRIBUTE_DIRECTORY));
        #else
            // Use Win CRT
            struct _stat fi;
            if (_stat(file, &fi) == 0) {
                #if defined(S_ISSOCK)
                    // sockets come back as a 'file' on some systems
                    // so make sure it's not a socket or directory
                    // (in other words, make sure it's an actual file)
                    return !(S_ISDIR(fi.st_mode)) &&
                        !(S_ISSOCK(fi.st_mode));
                #else
                    return !(S_ISDIR(fi.st_mode));
                #endif
            }
            return false;
        #endif
    #else
        struct stat fi;
        if (stat(file, &fi) == 0) {
            #if defined(S_ISSOCK)
                return !(S_ISDIR(fi.st_mode)) &&
                    !(S_ISSOCK(fi.st_mode));
            #else
                return !(S_ISDIR(fi.st_mode));
            #endif
        }
        return false;
    #endif
}

bool dir_exists(const char* folder)
{
    if (folder == NULL) { return false; }
    #if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__)
        #if defined(WIN_API)
            // if you want the WinAPI, versus CRT
            if (strnlen(folder, MAX_PATH+1) > MAX_PATH) {
                // ... throw error here or ...
                return false;
            }
            DWORD res = GetFileAttributesA(folder);
            return (res != INVALID_FILE_ATTRIBUTES &&
                (res & FILE_ATTRIBUTE_DIRECTORY));
        #else
            struct _stat fi;
            if (_stat(folder, &fi) == 0) {
                return (S_ISDIR(fi.st_mode));
            }
            return false;    
        #endif
    #else
        struct stat fi;
        if (stat(folder, &fi) == 0) {
            return (S_ISDIR(fi.st_mode));
        }
        return false;
    #endif
}

#include <crypto/neuralleadqhash_interface.h>
#include <crypto/common.h>

#include <assert.h>
#include <string.h>
#include <map>

#include <compat/cpuid.h>
#include <util/strencodings.h>

#include <crypto/cryptoconf.h>
#include <crypto/sha512.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace
{
    namespace sha256
    {
        void inline Initialize(uint32_t* s)
        {
            s[0] = 0x6a09e667ul;
            s[1] = 0xbb67ae85ul;
            s[2] = 0x3c6ef372ul;
            s[3] = 0xa54ff53aul;
            s[4] = 0x5be0cd19ul;
            s[5] = 0x9b05688cul;
            s[6] = 0x1f83d9abul;
            s[7] = 0x510e527ful;
        }

        void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks)
        {
            uint8_t* aiai = (uint8_t*) malloc(64);
            if(aiai==nullptr)
                std::runtime_error("memory error");
            memcpy(aiai, chunk, 64);

            uint8_t* oioi = (uint8_t*) malloc(32);
            if(oioi==nullptr)
                std::runtime_error("memory error");
            
            uint8_t* hhh = (uint8_t*) malloc(4);
            if(hhh==nullptr)
                std::runtime_error("memory error");

            while (blocks--)
            {
                for(int u = 0, a = 0; a < 64; u++, a+=4)
                {
                    WriteBE32(hhh, s[u%8]);
                    
                    aiai[a]   += hhh[0];
                    aiai[a+1] += hhh[1];
                    aiai[a+2] += hhh[2];
                    aiai[a+3] += hhh[3];
                }
                
                DirectComputeHash(aiai, 64, oioi);

                s[0] += ReadBE32(oioi);
                s[1] += ReadBE32(oioi + 4);
                s[2] += ReadBE32(oioi + 8);
                s[3] += ReadBE32(oioi + 12);
                s[4] += ReadBE32(oioi + 16);
                s[5] += ReadBE32(oioi + 20);
                s[6] += ReadBE32(oioi + 24);
                s[7] += ReadBE32(oioi + 28);
                chunk += 64;
            }

            free(aiai);
            free(oioi);
            free(hhh);
        }

        void TransformD64(unsigned char* out, const unsigned char* in)
        {
            uint32_t s[8];
            Initialize(s);
            Transform(s, in, 2);

            // Output
            WriteBE32(out + 0,  s[0] + 0x6a09e667ul);
            WriteBE32(out + 4,  s[1] + 0xbb67ae85ul);
            WriteBE32(out + 8,  s[2] + 0x3c6ef372ul);
            WriteBE32(out + 12, s[3] + 0xa54ff53aul);
            WriteBE32(out + 16, s[4] + 0x510e527ful);
            WriteBE32(out + 20, s[5] + 0x9b05688cul);
            WriteBE32(out + 24, s[6] + 0x1f83d9abul);
            WriteBE32(out + 28, s[7] + 0x5be0cd19ul);
        }

    } // namespace sha256

typedef void (*TransformType)(uint32_t*, const unsigned char*, size_t);
typedef void (*TransformD64Type)(unsigned char*, const unsigned char*);

TransformType Transform = sha256::Transform;
TransformD64Type TransformD64 = sha256::TransformD64;

bool SelfTest()
{
    if(!dir_exists("neuralleadqhash"))
    {
        //std::__throw_runtime_error("NeuralLeadQHash brain folder can't be found");
        return false;
    }
    
    std::map<std::string, std::string> fhashes = 
    {
        {"coscienza.brain", "a6fdab5a3aac140d312450bfe0a8a9a0e50b3995f102de5e7f3dd7531340aebda72bf645773041dea31276524856c1c55bee64412727c1512ab85bb843fa973c"},
        {"neuralleadhash_InputTo.coscienza.brain", "fcb9cb53525f21bb684d597037b5d39879ab779ec029818b74974cff7be1a34ab65cdcfcaedf6b79808fd1819dd4ac3684b822a345db8a21bea3c52703503341"},
        {"neuralleadhash_s1.coscienza.brain", "39f69762b8e07a4727a29493faa2c0d60d15a6f025daeebca19b47dd218ac2e301e531ba509552f13f3e301e26f4fecbeb972f443a2b3a0c5455e02a15aff1c6"},
        {"neuralleadhash_x1.coscienza.brain", "7cc878b10cca8a38df03177c000347efc5486729ae6134f88a282e5239519c1488e2d16158e236af23637fda273ac063ee8a7d0baa58ade1d0c244c429b18512"},
        {"neuralleadhash_to.coscienza.brain", "2d8fb802c47bbcdbb511deead7afda7ffc709b7b59bbb46565611b8e8d1c334ea3c609af1b19f9c066bb8b6f6fbe041fd50dacecc1e76478530fbd4ef40195a7"},
    };

    std::string baseNLHashDir = "neuralleadqhash";
    baseNLHashDir+=PATH_SEP;

    for(auto fhash : fhashes)
    {
        std::string fToCheck = baseNLHashDir+fhash.first;
        if(!file_exists(fToCheck.c_str()))
        {
            return false;
        }

        std::ifstream brainFul(fToCheck, std::ios::binary);

        brainFul.seekg(0, std::ios::end);
        std::streamsize fileSize = brainFul.tellg();
        brainFul.seekg(0, std::ios::beg);

        std::vector<unsigned char> buffer(fileSize);

        if (!brainFul.read(reinterpret_cast<char*>(buffer.data()), fileSize))
        {
            return false;
        }

        brainFul.close();

        uint8_t brainFh[64];
        CSHA512().Write(buffer.data(), buffer.size()).Finalize(brainFh);
        
        if(HexStr(brainFh) != fhash.second)
            return false;
    }

    std::string iNputTest = "neurallead&simonjriddix4research";
    uint8_t* incTest = (uint8_t*)iNputTest.c_str();
    uint8_t* oHash = new uint8_t[32];

    DirectComputeHash(incTest, iNputTest.size(), oHash);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0'); // Imposta il formato esadecimale e riempimento con zero

    for (int bp = 0; bp < 32; bp++)
    {
        ss << std::setw(2) << static_cast<int>(oHash[bp]); // Converte ogni byte in esadecimale
    }
    std::string hex = ss.str();

    if(hex != "566c799f626fb916dcdf9009737a51c21f8cd5b3afb54571b5ec2164bfe62dbc")
        return false;

    delete[] oHash;

    return true;
}
} // namespace


std::string SHA256AutoDetect()
{
    assert(SelfTest());
    
    return "NeuralLeadHash";
}

NeuralLeadQHash_iface::NeuralLeadQHash_iface(bool UseGPU) : bytes(0)
{
    sha256::Initialize(s);
}

NeuralLeadQHash_iface& NeuralLeadQHash_iface::Write(const unsigned char* data, size_t len)
{
    const unsigned char* end = data + len;
    size_t bufsize = bytes % 64;
    if (bufsize && bufsize + len >= 64) {
        // Fill the buffer, and process it.
        memcpy(buf + bufsize, data, 64 - bufsize);
        bytes += 64 - bufsize;
        data += 64 - bufsize;
        Transform(s, buf, 1);
        bufsize = 0;
    }
    if (end - data >= 64) {
        size_t blocks = (end - data) / 64;
        Transform(s, data, blocks);
        data += 64 * blocks;
        bytes += 64 * blocks;
    }
    if (end > data) {
        // Fill the buffer with what remains.
        memcpy(buf + bufsize, data, end - data);
        bytes += end - data;
    }
    return *this;
}

void NeuralLeadQHash_iface::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    static const unsigned char pad[64] = {0x80};
    unsigned char sizedesc[8];
    WriteBE64(sizedesc, bytes << 3);
    Write(pad, 1 + ((119 - (bytes % 64)) % 64));
    Write(sizedesc, 8);
    
    WriteBE32(hash, s[0]);
    WriteBE32(hash + 4, s[1]);
    WriteBE32(hash + 8, s[2]);
    WriteBE32(hash + 12, s[3]);
    WriteBE32(hash + 16, s[4]);
    WriteBE32(hash + 20, s[5]);
    WriteBE32(hash + 24, s[6]);
    WriteBE32(hash + 28, s[7]);
}

NeuralLeadQHash_iface& NeuralLeadQHash_iface::Reset()
{
    bytes = 0;
    sha256::Initialize(s);
    return *this;
}

void SHA256D64(unsigned char* out, const unsigned char* in, size_t blocks)
{
    while (blocks) {
        sha256::TransformD64(out, in);
        out += 32;
        in += 64;
        --blocks;
    }
}

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__)

#else

/*
Copyright 2024 SimonJRiddix & NeuralLead

NeuralLead Quantum Ai Hash

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Quantum & Neural Network functions

#include <cstring>
#include <omp.h>

#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <bitset>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <limits>
#include <cmath>

#include <Eigen/Dense>
#include <qpp/qpp.h>

using namespace qpp;

#define NUM_QUBITS 2  // Working qubits

// Rotation functions
#define rotl(x, b) (uint32_t)(((x) << (b)) | ((x) >> (32 - (b))))
#define rotr(x, b) (uint32_t)(((x) >> (b)) | ((x) << (32 - (b))))

// Normalization functions
#define normalize_uint32_t(t) (static_cast<double>(t) / static_cast<double>(UINT32_MAX))
#define denormalize_uint32_t(f) (static_cast<uint32_t>(f * static_cast<double>(UINT32_MAX)))

// Improved mixing function
void inline improvedMix(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d)
{
    a = rotl(a, 13) ^ b;
    b = rotr(b, 7) + c;
    c = rotl(c, 17) ^ d;
    d = rotr(d, 11) + a;
}

// Additional mixing step between blocks
void mixBetweenBlocks(uint32_t state[8])
{
    improvedMix(state[0], state[1], state[2], state[3]);
    improvedMix(state[1], state[2], state[3], state[4]);
    improvedMix(state[2], state[3], state[4], state[5]);
    improvedMix(state[3], state[4], state[5], state[6]);
    improvedMix(state[4], state[5], state[6], state[7]);
}

// Improved compression function
void compress(uint32_t state[8], const uint8_t* block, uint32_t& quantum_mix, bool useNL)
{
    thread_local const Gates& insta = Gates::get_thread_local_instance();

    thread_local auto& prng = RandomDevices::get_thread_local_instance();
    prng.get_prng().seed(accumulated_seed);

    uint32_t w[64];

    // Message expansion
    for (int i = 0; i < 16; ++i)
    {
        int pos = i * 4;
        w[i] = (block[pos] << 24) | (block[pos + 1] << 16) | (block[pos + 2] << 8) | block[pos + 3];
    }

    for (int i = 16; i < 64; ++i)
    {
        uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int round = 0; round < 4; ++round)
    {
        // Improved round function
        uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);

        uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        for (int pii = 0; pii < 16; pii++)
        {
            auto ri = pii * round;
            uint32_t temp1 = (h + S1 + ch + roundKeys[ri] + w[ri]) ^ quantum_mix;
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        
        // Additional mixing
        improvedMix(a, b, c, d);
        improvedMix(e, f, g, h);
    }

    if (useNL)  // NeuralLead Update
    {
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;

        float nn_inputs[32];
        float nn_outputs[8];

        InputsIntToNeuralLead(state, nn_inputs);

        neuralNetwork(nn_inputs, nn_outputs);

        // Mescolamento intermedio dello stato con rete neurallead prima della compressione quantistica
        for (int i = 1; i < 10; ++i)
        {
            state[i % 8] ^= static_cast<uint32_t>(nn_outputs[i % NL_OUTPUTS] * 1000.0f) ^ roundKeys[i] ^ state[(i - 1) % 8];
        }

        a = state[0], b = state[1], c = state[2], d = state[3];
        e = state[4], f = state[5], g = state[6], h = state[7];
    }

    {
        // Quantum algo

        // Improved round function
        uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);

        uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        ket qubits = 00_ket;  // Stato iniziale |00>

        // Quantum operations

        qubits = kron(insta.H, insta.H) * qubits;

        // Apply controlled rotations
        qubits = apply(qubits, insta.RZ(normalize_uint32_t(a ^ e)), { 0 });
        qubits = apply(qubits, insta.RX(normalize_uint32_t(b ^ f)), { 1 });
        qubits = apply(qubits, insta.RY(normalize_uint32_t(c ^ g)), { 0 });
        qubits = apply(qubits, insta.RZ(normalize_uint32_t(d ^ h)), { 1 });

        // Apply CNOT gates
        qubits = apply(qubits, insta.CNOT, { 0, 1 });

        // Measure all qubits
        auto [m, probs, states] = measure(qubits, insta.Id(1 << NUM_QUBITS));

        quantum_mix ^= denormalize_uint32_t(probs[0]);

        auto dx = denormalize_uint32_t(states[0].x().real());
        auto dx_i = denormalize_uint32_t(states[0].x().imag());
        quantum_mix ^= dx;
        quantum_mix ^= dx_i;

        auto dy = denormalize_uint32_t(states[0].y().real());
        auto dy_i = denormalize_uint32_t(states[0].y().imag());
        quantum_mix ^= dy;
        quantum_mix ^= dy_i;

        uint32_t temp1 = (h + S1 + ch + roundKeys[63] + w[63]) ^ quantum_mix;
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    // Additional mixing
    improvedMix(a, b, c, d);
    improvedMix(e, f, g, h);

    // Final state update
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

// Main hash function
DLL_API_NLHASH ComputeStatus DirectComputeHash(const uint8_t* data, size_t length, uint8_t*& hash_output)
{
    // Initial state
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // Pre-processing
    size_t padded_length = ((length + 9 + 63) / 64) * 64;
    std::vector<uint8_t> padded_data(padded_length, 0);
    memcpy(padded_data.data(), data, length);
    //padded_data[length -1] &= 0x80;

    uint64_t bit_len = length * 8;
    for (int i = 0; i < 8; ++i)
    {
        padded_data[padded_length - 1 - i] = static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF);
    }

    uint32_t quantum_mix = 0;

    // Process each block
    for (size_t i = 0; i < padded_length; i += 64)
    {        
        bool useNL = ((padded_data[i] + length) % 5 == 0);
        compress(state, &padded_data[i], quantum_mix, useNL);
        mixBetweenBlocks(state);  // Additional mixing between blocks
    }

    // Produce final hash
    for (int i = 0; i < 8; ++i)
    {
        int pos = i * 4;
        hash_output[pos] = (state[i] >> 24) & 0xFF;
        hash_output[pos + 1] = (state[i] >> 16) & 0xFF;
        hash_output[pos + 2] = (state[i] >> 8) & 0xFF;
        hash_output[pos + 3] = state[i] & 0xFF;
    }

    return ComputeStatus::HASH_SUCCESS;
}

// NeuralLead section
// Disabled, already compiled with NeuralLead Maker, call from static/dynamic library it externally to improve speed and accurancy
// thanks to optimizations

#define _ENABLE_NEURALLEAD_SECTION

    #if defined(_ENABLE_NEURALLEAD_SECTION)
        // in neuralleadhash.cpp
    #endif

#endif