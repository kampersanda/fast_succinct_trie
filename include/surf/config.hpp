//  Modifications copyright (C) 2019 <Shunsuke Kanda>
//
//  modifications are
//    - reformating the source,
//    - adding include headers,
//    - adding save/load functions,
//    - and as commented at each point
//
#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <string.h>

// Added by Shunsuke Kanda
#include <fstream>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

namespace surf {

using level_t = uint32_t;
using position_t = uint32_t;

using label_t = uint8_t;
static const position_t kFanout = 256;

using word_t = uint64_t;
static const unsigned kWordSize = 64;
static const word_t kMsbMask = 0x8000000000000000;
static const word_t kOneMask = 0xFFFFFFFFFFFFFFFF;

static const bool kIncludeDense = true;
static const uint32_t kSparseDenseRatio = 64;
// static const uint32_t kSparseDenseRatio = 16;
// static const label_t kTerminator = 255;
static const label_t kTerminator = 0;  // Modified by Shunsuke Kanda

static const int kHashShift = 7;

static const int kCouldBePositive = 2018;  // used in suffix comparison

// Added by Shunsuke Kanda
static const position_t kNotFound = static_cast<position_t>(-1);

enum SuffixType { kNone = 0, kHash = 1, kReal = 2, kMixed = 3 };

void align(char*& ptr) {
    ptr = (char*)(((uint64_t)ptr + 7) & ~((uint64_t)7));
}

void sizeAlign(position_t& size) {
    size = (size + 7) & ~((position_t)7);
}

void sizeAlign(uint64_t& size) {
    size = (size + 7) & ~((uint64_t)7);
}

std::string uint64ToString(const uint64_t word) {
    uint64_t endian_swapped_word = __builtin_bswap64(word);
    return std::string(reinterpret_cast<const char*>(&endian_swapped_word), 8);
}

uint64_t stringToUint64(const std::string& str_word) {
    uint64_t int_word = 0;
    memcpy(reinterpret_cast<char*>(&int_word), str_word.data(), 8);
    return __builtin_bswap64(int_word);
}

// Added by Kanda
template <typename T>
inline void saveValue(std::ostream& os, T val) {
    os.write(reinterpret_cast<const char*>(&val), sizeof(T));
}
template <typename T>
inline void saveArray(std::ostream& os, const std::unique_ptr<T[]>& ptr, size_t size) {
    os.write(reinterpret_cast<const char*>(ptr.get()), sizeof(T) * size);
}
template <typename T>
inline void loadValue(std::istream& is, T& val) {
    is.read(reinterpret_cast<char*>(&val), sizeof(T));
}
template <typename T>
inline void loadArray(std::istream& is, std::unique_ptr<T[]>& ptr, size_t size) {
    ptr = std::make_unique<T[]>(size);
    is.read(reinterpret_cast<char*>(ptr.get()), sizeof(T) * size);
}

}  // namespace surf

#endif  // CONFIG_H_
