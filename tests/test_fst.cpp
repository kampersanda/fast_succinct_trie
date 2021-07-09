#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <fst.hpp>

#include "doctest/doctest.h"

void test_exact_search(const fst::Trie& trie, const std::vector<std::string>& keys,
                       const std::vector<std::string>& others) {
    REQUIRE_EQ(trie.getNumKeys(), keys.size());
    for (size_t i = 0; i < keys.size(); i++) {
        fst::position_t key_id = trie.exactSearch(keys[i]);
        REQUIRE_NE(key_id, fst::kNotFound);
    }
    for (size_t i = 0; i < others.size(); i++) {
        fst::position_t key_id = trie.exactSearch(others[i]);
        REQUIRE_EQ(key_id, fst::kNotFound);
    }
}

void test_io(const fst::Trie& trie, const std::vector<std::string>& keys, const std::vector<std::string>& others) {
    const char* tmp_filepath = "tmp.idx";
    {
        std::ofstream ofs(tmp_filepath);
        trie.save(ofs);
    }
    {
        fst::Trie loaded;
        std::ifstream ifs(tmp_filepath);
        loaded.load(ifs);

        REQUIRE_EQ(trie.getNumKeys(), loaded.getNumKeys());
        REQUIRE_EQ(trie.getNumNodes(), loaded.getNumNodes());
        REQUIRE_EQ(trie.getSuffixBytes(), loaded.getSuffixBytes());
        REQUIRE_EQ(trie.getMemoryUsage(), loaded.getMemoryUsage());
        REQUIRE_EQ(trie.getSizeIO(), loaded.getSizeIO());
        test_exact_search(loaded, keys, others);
    }
    std::remove(tmp_filepath);
}

template <class T>
std::vector<T> to_unique_vec(std::vector<T>&& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    return std::move(vec);
}

std::vector<std::string> make_random_keys(std::uint64_t n, std::uint64_t min_m, std::uint64_t max_m,  //
                                          char min_c = 'A', char max_c = 'Z', std::uint64_t seed = 13) {
    std::mt19937_64 engine(seed);
    std::uniform_int_distribution<std::uint64_t> dist_m(min_m, max_m);
    std::uniform_int_distribution<char> dist_c(min_c, max_c);

    std::vector<std::string> keys(n);
    for (std::uint64_t i = 0; i < n; i++) {
        keys[i].resize(dist_m(engine));
        for (std::uint64_t j = 0; j < keys[i].size(); j++) {
            keys[i][j] = dist_c(engine);
        }
    }
    return keys;
}

std::vector<std::string> extract_keys(std::vector<std::string>& keys, double ratio = 0.1, std::uint64_t seed = 13) {
    std::mt19937_64 engine(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::vector<std::string> keys1;
    std::vector<std::string> keys2;

    for (std::uint64_t i = 0; i < keys.size(); ++i) {
        if (ratio < dist(engine)) {
            keys1.push_back(keys[i]);
        } else {
            keys2.push_back(keys[i]);
        }
    }

    keys = keys1;
    return keys2;
}

TEST_CASE("Test fst::Trie (tiny)") {
    std::vector<std::string> keys = {
        "AirPods",  "AirTag",  "Mac",  "MacBook", "MacBook_Air", "MacBook_Pro",
        "Mac_Mini", "Mac_Pro", "iMac", "iPad",    "iPhone",      "iPhone_SE",
    };
    std::vector<std::string> others = {
        "Google_Pixel", "iPad_mini", "iPadOS", "iPod", "ThinkPad",
    };

    fst::Trie trie(keys);
    test_exact_search(trie, keys, others);
    test_io(trie, keys, others);
}

TEST_CASE("Test fst::Trie (random 10K, A--B)") {
    auto keys = to_unique_vec(make_random_keys(10000, 1, 30, 'A', 'B'));
    auto others = extract_keys(keys);

    fst::Trie trie(keys);
    test_exact_search(trie, keys, others);
    test_io(trie, keys, others);
}

TEST_CASE("Test fst::Trie (random 10K, A--Z)") {
    auto keys = to_unique_vec(make_random_keys(10000, 1, 30, 'A', 'Z'));
    auto others = extract_keys(keys);

    fst::Trie trie(keys);
    test_exact_search(trie, keys, others);
    test_io(trie, keys, others);
}
