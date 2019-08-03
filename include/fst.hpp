#pragma once

#include "surf/louds_dense.hpp"
#include "surf/louds_sparse.hpp"
#include "surf/surf_builder.hpp"

namespace fst {

using position_t = surf::position_t;
using level_t = surf::level_t;
using surf::kNotFound;

namespace detail {

class CompactArray {
  public:
    CompactArray() = default;
    CompactArray(const std::vector<uint32_t>& input, const uint32_t bits);

    ~CompactArray() = default;

    uint32_t operator[](uint32_t i) const;

    uint32_t getSize() const;
    uint64_t getSizeIO() const;
    uint64_t getMemoryUsage() const;

    void save(std::ostream& os) const;
    void load(std::istream& is);

  private:
    uint32_t size_ = 0;
    uint32_t mask_ = 0;
    uint32_t bits_ = 0;
    std::vector<uint32_t> chunks_;
};

template <class T>
static void saveVec(std::ostream& os, const std::vector<T>& vec) {
    size_t n = vec.size();
    surf::saveValue(os, n);
    os.write(reinterpret_cast<const char*>(vec.data()), sizeof(T) * n);
}
template <class T>
static void loadVec(std::istream& is, std::vector<T>& vec) {
    size_t n = 0;
    surf::loadValue(is, n);
    vec.resize(n);
    is.read(reinterpret_cast<char*>(vec.data()), sizeof(T) * n);
}
template <class T>
static uint64_t getVecSizeIO(const std::vector<T>& vec) {
    return sizeof(size_t) + sizeof(T) * vec.size();
}
template <class T>
static uint64_t getVecMemoryUsage(const std::vector<T>& vec) {
    return sizeof(T) * vec.size();
}

}  // namespace detail

class Trie {
  public:
    Trie() = default;
    Trie(const std::vector<std::string>& keys);
    Trie(const std::vector<std::string>& keys, const bool include_dense, const uint32_t sparse_dense_ratio);

    ~Trie() = default;

    position_t exactSearch(const std::string& key) const;

    uint64_t getSizeIO() const;
    uint64_t getMemoryUsage() const;

    level_t getHeight() const;
    level_t getSparseStartLevel() const;

    uint64_t getNumKeys() const;
    uint64_t getNumNodes() const;
    uint64_t getSuffixBytes() const;

    void save(std::ostream& os) const;
    void load(std::istream& is);

    void debugPrint(std::ostream& os) const;

  private:
    std::pair<position_t, level_t> traverse(const std::string& key) const;

  private:
    std::unique_ptr<surf::LoudsDense> louds_dense_;
    std::unique_ptr<surf::LoudsSparse> louds_sparse_;
    detail::CompactArray suffix_ptrs_;
    std::vector<char> suffixes_;  // unified
    position_t num_keys_ = 0;
};

Trie::Trie(const std::vector<std::string>& keys) : Trie(keys, surf::kIncludeDense, surf::kSparseDenseRatio) {}

Trie::Trie(const std::vector<std::string>& keys, const bool include_dense, const uint32_t sparse_dense_ratio) {
    auto builder = std::make_unique<surf::SuRFBuilder>(include_dense, sparse_dense_ratio, surf::kNone, 0, 0);
    builder->build(keys);
    louds_dense_ = std::make_unique<surf::LoudsDense>(builder.get());
    louds_sparse_ = std::make_unique<surf::LoudsSparse>(builder.get());

    num_keys_ = 0;
    for (level_t level = 0; level < louds_sparse_->getHeight(); ++level) {
        num_keys_ += builder->getSuffixCounts()[level];
    }

    struct suffix_t {
        std::pair<const char*, size_t> str;
        position_t key_id = kNotFound;

        size_t length() const {
            return str.second;
        }
        char operator[](size_t i) const {
            return str.first[str.second - i - 1];
        }

        const char* begin() const {
            return str.first;
        }
        const char* end() const {
            return str.first + str.second;
        }

        std::reverse_iterator<const char*> rbegin() const {
            return std::make_reverse_iterator(str.first + str.second);
        }
        std::reverse_iterator<const char*> rend() const {
            return std::make_reverse_iterator(str.first);
        }
    };

    std::vector<suffix_t> suffixes_builder(num_keys_);

    for (position_t i = 0; i < keys.size(); ++i) {
        if (i != 0 && keys[i] == keys[i - 1]) {
            continue;
        }

        position_t key_id = 0;
        level_t level = 0;

        std::tie(key_id, level) = traverse(keys[i]);
        assert(key_id < num_keys_);
        assert(suffixes_builder[key_id].key_id == kNotFound);
        assert(level <= keys[i].length());

        auto str = std::make_pair(keys[i].c_str() + level, keys[i].length() - level);
        suffixes_builder[key_id] = suffix_t{str, key_id};
    }

    std::sort(suffixes_builder.begin(), suffixes_builder.end(), [](const suffix_t& x, const suffix_t& y) {
        return std::lexicographical_compare(x.rbegin(), x.rend(), y.rbegin(), y.rend());
    });

    std::vector<uint32_t> suffix_ptrs(num_keys_);
    suffixes_.emplace_back('\0');  // for empty suffix

    suffix_t prev_suffix = {{nullptr, 0}, kNotFound};

    for (size_t i = 0; i < num_keys_; ++i) {
        const suffix_t& curr_suffix = suffixes_builder[num_keys_ - i - 1];
        if (curr_suffix.length() == 0) {
            suffix_ptrs[curr_suffix.key_id] = 0;
            continue;
        }

        size_t match = 0;
        while ((match < curr_suffix.length()) && (match < prev_suffix.length()) &&
               (prev_suffix[match] == curr_suffix[match])) {
            ++match;
        }

        if ((match == curr_suffix.length()) && (prev_suffix.length() != 0)) {  // share
            suffix_ptrs[curr_suffix.key_id] =
                static_cast<position_t>(suffix_ptrs[prev_suffix.key_id] + (prev_suffix.length() - match));
        } else {  // append
            suffix_ptrs[curr_suffix.key_id] = static_cast<position_t>(suffixes_.size());
            std::copy(curr_suffix.begin(), curr_suffix.end(), std::back_inserter(suffixes_));
            suffixes_.emplace_back('\0');
        }

        prev_suffix = curr_suffix;
    }

    uint32_t suf_bits = 0;
    uint32_t max_ptr = static_cast<uint32_t>(suffixes_.size());
    do {
        suf_bits += 1;
        max_ptr >>= 1;
    } while (max_ptr != 0);

    suffix_ptrs_ = detail::CompactArray(suffix_ptrs, suf_bits);
    suffixes_.shrink_to_fit();
}

position_t Trie::exactSearch(const std::string& key) const {
    position_t key_id = 0;
    level_t level = 0;

    std::tie(key_id, level) = traverse(key);
    if (key_id == kNotFound) {
        return kNotFound;
    }

    position_t suf_pos = suffix_ptrs_[key_id];
    for (; level < key.length(); ++level) {
        if (key[level] != suffixes_[suf_pos]) {
            return kNotFound;
        }
        ++suf_pos;
    }
    if (suffixes_[suf_pos] != '\0') {
        return kNotFound;
    }
    return key_id;
}

uint64_t Trie::getSizeIO() const {
    return louds_dense_->serializedSize() + louds_sparse_->serializedSize() + suffix_ptrs_.getSizeIO() +
           detail::getVecSizeIO(suffixes_) + sizeof(num_keys_);
}

uint64_t Trie::getMemoryUsage() const {
    return sizeof(Trie) + louds_dense_->getMemoryUsage() + louds_sparse_->getMemoryUsage() +
           suffix_ptrs_.getMemoryUsage() + detail::getVecMemoryUsage(suffixes_);
}

level_t Trie::getHeight() const {
    return louds_sparse_->getHeight();
}

level_t Trie::getSparseStartLevel() const {
    return louds_sparse_->getStartLevel();
}

uint64_t Trie::getNumKeys() const {
    return num_keys_;
}

uint64_t Trie::getNumNodes() const {
    return louds_dense_->getNumNodes() + louds_sparse_->getNumNodes();
}
uint64_t Trie::getSuffixBytes() const {
    return suffixes_.size();
}

void Trie::save(std::ostream& os) const {
    louds_dense_->save(os);
    louds_sparse_->save(os);
    suffix_ptrs_.save(os);
    detail::saveVec(os, suffixes_);
    surf::saveValue(os, num_keys_);
}

void Trie::load(std::istream& is) {
    louds_dense_ = std::make_unique<surf::LoudsDense>();
    louds_dense_->load(is);
    louds_sparse_ = std::make_unique<surf::LoudsSparse>();
    louds_sparse_->load(is);
    suffix_ptrs_.load(is);
    detail::loadVec(is, suffixes_);
    surf::loadValue(is, num_keys_);
}

void Trie::debugPrint(std::ostream& os) const {
    louds_dense_->debugPrint(os);
    louds_sparse_->debugPrint(os);
    os << "-- Suffixes --" << std::endl;
    os << "POINTERS: ";
    for (uint32_t i = 0; i < suffix_ptrs_.getSize(); ++i) {
        os << suffix_ptrs_[i] << " ";
    }
    os << '\n';
    os << "SUFFIXES: ";
    for (size_t i = 0; i < suffixes_.size(); ++i) {
        char c = suffixes_[i];
        os << (c ? c : '?') << " ";
    }
    os << std::endl;
}

std::pair<position_t, level_t> Trie::traverse(const std::string& key) const {
    position_t connect_node_num = 0;
    auto ret = louds_dense_->findKey(key, connect_node_num);
    if (ret.first != kNotFound) {
        return ret;
    }
    if (connect_node_num != kNotFound) {
        return louds_sparse_->findKey(key, connect_node_num);
    }
    return ret;
}

namespace detail {

CompactArray::CompactArray(const std::vector<uint32_t>& input, const uint32_t bits)
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

uint32_t CompactArray::operator[](uint32_t i) const {
    const uint32_t quo = i * bits_ / 32;
    const uint32_t mod = i * bits_ % 32;
    if (mod + bits_ <= 32) {
        return (chunks_[quo] >> mod) & mask_;
    } else {
        return ((chunks_[quo] >> mod) | (chunks_[quo + 1] << (32 - mod))) & mask_;
    }
}

uint32_t CompactArray::getSize() const {
    return size_;
}

uint64_t CompactArray::getSizeIO() const {
    return (sizeof(uint32_t) * 3) + detail::getVecSizeIO(chunks_);
}

uint64_t CompactArray::getMemoryUsage() const {
    return detail::getVecMemoryUsage(chunks_);
}

void CompactArray::save(std::ostream& os) const {
    surf::saveValue(os, size_);
    surf::saveValue(os, mask_);
    surf::saveValue(os, bits_);
    detail::saveVec(os, chunks_);
}

void CompactArray::load(std::istream& is) {
    surf::loadValue(is, size_);
    surf::loadValue(is, mask_);
    surf::loadValue(is, bits_);
    detail::loadVec(is, chunks_);
}

}  // namespace detail

}  // namespace fst
