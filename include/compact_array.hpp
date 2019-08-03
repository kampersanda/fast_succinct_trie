#pragma once

#include <fstream>
#include <vector>

#include "surf/config.hpp"

namespace fst {

class CompactArray {
  public:
    CompactArray() = default;

    CompactArray(const std::vector<uint32_t>& input, const uint32_t bits)
        : size_(static_cast<uint32_t>(input.size())),
          mask_((1U << bits) - 1),
          bits_(bits),
          chunks_(size_ * bits_ / 32 + 1) {
        for (uint32_t i = 0; i < size_; ++i) {
            const uint32_t quo = i * bits_ / 32;
            const uint32_t mod = i * bits_ % 32;
            chunks_[quo] &= ~(mask_ << mod);
            chunks_[quo] |= (input[i] & mask_) << mod;
            if (32 < mod + bits_) {
                chunks_[quo + 1] &= ~(mask_ >> (32 - mod));
                chunks_[quo + 1] |= (input[i] & mask_) >> (32 - mod);
            }
        }
    }

    ~CompactArray() = default;

    uint32_t operator[](uint32_t i) const {
        const uint32_t quo = i * bits_ / 32;
        const uint32_t mod = i * bits_ % 32;
        if (mod + bits_ <= 32) {
            return (chunks_[quo] >> mod) & mask_;
        } else {
            return ((chunks_[quo] >> mod) | (chunks_[quo + 1] << (32 - mod))) & mask_;
        }
    }

    uint32_t getSize() const {
        return size_;
    }

    uint64_t getSizeIO() const {
        return (sizeof(uint32_t) * 3) + sizeof(size_t) + sizeof(uint32_t) * chunks_.size();
    }

    uint64_t getMemoryUsage() const {
        return sizeof(uint32_t) * chunks_.size();
    }

    void save(std::ostream& os) const {
        surf::saveValue(os, size_);
        surf::saveValue(os, mask_);
        surf::saveValue(os, bits_);

        size_t n = chunks_.size();
        surf::saveValue(os, n);
        os.write(reinterpret_cast<const char*>(chunks_.data()), sizeof(uint32_t) * n);
    }

    void load(std::istream& is) {
        surf::loadValue(is, size_);
        surf::loadValue(is, mask_);
        surf::loadValue(is, bits_);

        size_t n = 0;
        surf::loadValue(is, n);
        chunks_.resize(n);
        is.read(reinterpret_cast<char*>(chunks_.data()), sizeof(uint32_t) * n);
    }

  private:
    uint32_t size_ = 0;
    uint32_t mask_ = 0;
    uint32_t bits_ = 0;
    std::vector<uint32_t> chunks_;
};
}  // namespace fst