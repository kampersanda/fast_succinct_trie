//  Modifications copyright (C) 2019 <Shunsuke Kanda>
//
//  modifications are
//    - reformating the source,
//    - adding save/load functions,
//    - commenting some functions out,
//    - changing the way of initilizing private members,
//    - removing raw pointers and using smart pointers,
//    - and as commented at each point
//
#ifndef RANK_H_
#define RANK_H_

#include "bitvector.hpp"

#include <assert.h>

#include <vector>

#include "popcount.h"

namespace surf {

class BitvectorRank : public Bitvector {
  public:
    // Modified by Shunsuke Kanda
    BitvectorRank() {}
    // BitvectorRank() : basic_block_size_(0), rank_lut_(nullptr){};

    BitvectorRank(const position_t basic_block_size, const std::vector<std::vector<word_t> >& bitvector_per_level,
                  const std::vector<position_t>& num_bits_per_level, const level_t start_level = 0,
                  const level_t end_level = 0 /* non-inclusive */)
        : Bitvector(bitvector_per_level, num_bits_per_level, start_level, end_level) {
        basic_block_size_ = basic_block_size;
        initRankLut();
    }

    ~BitvectorRank() {}

    // Counts the number of 1's in the bitvector up to position pos.
    // pos is zero-based; count is one-based.
    // E.g., for bitvector: 100101000, rank(3) = 2
    position_t rank(position_t pos) const {
        assert(pos < num_bits_);
        position_t word_per_basic_block = basic_block_size_ / kWordSize;
        position_t block_id = pos / basic_block_size_;
        position_t offset = pos & (basic_block_size_ - 1);
        // by Kanda
        return (rank_lut_[block_id] + popcountLinear(bits_.get(), block_id * word_per_basic_block, offset + 1));
        // return (rank_lut_[block_id] + popcountLinear(bits_, block_id * word_per_basic_block, offset + 1));
    }

    position_t rankLutSize() const {
        return ((num_bits_ / basic_block_size_ + 1) * sizeof(position_t));
    }

    position_t serializedSize() const {
        position_t size = sizeof(num_bits_) + sizeof(basic_block_size_) + bitsSize() + rankLutSize();
        sizeAlign(size);
        return size;
    }

    position_t size() const {
        return (sizeof(BitvectorRank) + bitsSize() + rankLutSize());
    }

    void prefetch(position_t pos) const {
        __builtin_prefetch(bits_.get() + (pos / kWordSize));
        __builtin_prefetch(rank_lut_.get() + (pos / basic_block_size_));
    }

    // Commented out by Shunsuke Kanda

    // void serialize(char*& dst) const {
    //     memcpy(dst, &num_bits_, sizeof(num_bits_));
    //     dst += sizeof(num_bits_);
    //     memcpy(dst, &basic_block_size_, sizeof(basic_block_size_));
    //     dst += sizeof(basic_block_size_);
    //     memcpy(dst, bits_, bitsSize());
    //     dst += bitsSize();
    //     memcpy(dst, rank_lut_, rankLutSize());
    //     dst += rankLutSize();
    //     align(dst);
    // }
    // static BitvectorRank* deSerialize(char*& src) {
    //     BitvectorRank* bv_rank = new BitvectorRank();
    //     memcpy(&(bv_rank->num_bits_), src, sizeof(bv_rank->num_bits_));
    //     src += sizeof(bv_rank->num_bits_);
    //     memcpy(&(bv_rank->basic_block_size_), src, sizeof(bv_rank->basic_block_size_));
    //     src += sizeof(bv_rank->basic_block_size_);
    //     bv_rank->bits_ = const_cast<word_t*>(reinterpret_cast<const word_t*>(src));
    //     src += bv_rank->bitsSize();
    //     bv_rank->rank_lut_ = const_cast<position_t*>(reinterpret_cast<const position_t*>(src));
    //     src += bv_rank->rankLutSize();
    //     align(src);
    //     return bv_rank;
    // }
    // void destroy() {
    //     delete[] bits_;
    //     delete[] rank_lut_;
    // }

  public:
    // Added by Kanda
    void save(std::ostream& os) const {
        Bitvector::save(os);
        saveValue(os, basic_block_size_);
        saveArray(os, rank_lut_, num_bits_ / basic_block_size_ + 1);
    }
    void load(std::istream& is) {
        Bitvector::load(is);
        loadValue(is, basic_block_size_);
        loadArray(is, rank_lut_, num_bits_ / basic_block_size_ + 1);
    }

  private:
    void initRankLut() {
        position_t word_per_basic_block = basic_block_size_ / kWordSize;
        position_t num_blocks = num_bits_ / basic_block_size_ + 1;

        // Modified by Shunsuke Kanda
        rank_lut_ = std::make_unique<position_t[]>(num_blocks);
        // rank_lut_ = new position_t[num_blocks];

        position_t cumu_rank = 0;
        for (position_t i = 0; i < num_blocks - 1; i++) {
            rank_lut_[i] = cumu_rank;
            cumu_rank += popcountLinear(bits_.get(), i * word_per_basic_block, basic_block_size_);
        }
        rank_lut_[num_blocks - 1] = cumu_rank;
    }

    // Modified by Shunsuke Kanda
    position_t basic_block_size_ = 0;
    std::unique_ptr<position_t[]> rank_lut_;
    // position_t* rank_lut_;  // rank look-up table
};

}  // namespace surf

#endif  // RANK_H_
