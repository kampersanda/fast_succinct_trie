#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

#include "cmdline/cmdline.h"

static const int SEARCH_RUNS = 10;

std::vector<std::string> load_strings(const std::string& fn) {
    std::ifstream ifs(fn);
    if (!ifs) {
        std::cerr << "error: failed to open " << fn << std::endl;
        exit(1);
    }

    std::vector<std::string> strings;
    for (std::string line; std::getline(ifs, line);) {
        strings.push_back(line);
    }
    return strings;
}

// Templates of operations
template <class T>
std::unique_ptr<T> build(std::vector<std::string>&);
template <class T>
bool find(T*, const std::string&);
template <class T>
uint64_t get_memory(T*);
template <class T>
void show_stats(T*, std::ostream&);

#ifdef USE_FST
#include <fst.hpp>
using trie_t = fst::Trie;
static const uint32_t SPARSE_DENSE_RATIO = 16;
template <>
std::unique_ptr<trie_t> build(std::vector<std::string>& keys) {
    auto trie = std::make_unique<trie_t>(keys, true, SPARSE_DENSE_RATIO);
    return trie;
}
template <>
bool find(trie_t* trie, const std::string& query) {
    return trie->exactSearch(query) != fst::kNotFound;
}
template <>
uint64_t get_memory(trie_t* trie) {
    return trie->getSizeIO();
}
template <>
void show_stats(trie_t* trie, std::ostream& os) {
    os << "- statistics: "  //
       << trie->getNumNodes() << " nodes; "  //
       << trie->getSuffixBytes() << " suffix bytes; "  //
       << trie->getSparseStartLevel() << " dense height; "  //
       << trie->getHeight() << " height; "  //
       << std::endl;
}
#endif

#ifdef USE_DARTSC
#include <darts.h>
using trie_t = Darts::DoubleArray;
template <>
std::unique_ptr<trie_t> build(std::vector<std::string>& key_strs) {
    std::size_t num_keys = key_strs.size();
    std::vector<const char*> keys(num_keys);
    for (std::size_t i = 0; i < num_keys; ++i) {
        keys[i] = key_strs[i].c_str();
    }

    auto trie = std::make_unique<trie_t>();
    trie->build(num_keys, keys.data());
    return trie;
}
template <>
bool find(trie_t* trie, const std::string& query) {
    return trie->exactMatchSearch<int>(query.c_str(), query.length()) != -1;
}
template <>
uint64_t get_memory(trie_t* trie) {
    return trie->total_size();
}
template <>
void show_stats(trie_t* trie, std::ostream& os) {
    os << "- statistics: "  //
       << trie->size() << " nodes; "  //
       << trie->nonzero_size() << " nonzero nodes"  //
       << std::endl;
}
#endif

#ifdef USE_TX
#include <tx.hpp>
using trie_t = tx_tool::tx;
static const char* TX_INDEX = "tx.idx";
template <>
std::unique_ptr<trie_t> build(std::vector<std::string>& keys) {
    {
        trie_t trie;
        trie.build(keys, TX_INDEX);
    }
    auto trie = std::make_unique<trie_t>();
    trie->read(TX_INDEX);
    return trie;
}
template <>
bool find(trie_t* trie, const std::string& query) {
    size_t retLen = 0;
    return trie->prefixSearch(query.c_str(), query.length(), retLen) != tx_tool::tx::NOTFOUND;
}
template <>
uint64_t get_memory(trie_t*) {
    std::ifstream is(TX_INDEX, std::ios::binary | std::ios::ate);
    return uint64_t(is.tellg());
}
template <>
void show_stats(trie_t* trie, std::ostream& os) {}
#endif

#ifdef USE_MARISA
#include <marisa.h>
using trie_t = marisa::Trie;
template <>
std::unique_ptr<trie_t> build(std::vector<std::string>& keys) {
    marisa::Keyset keyset;
    for (std::size_t i = 0; i < keys.size(); ++i) {
        keyset.push_back(keys[i].c_str(), keys[i].length(), 1.0F);
    }
    auto trie = std::make_unique<trie_t>();
    trie->build(keyset);
    return trie;
}
template <>
bool find(trie_t* trie, const std::string& query) {
    static marisa::Agent agent;
    agent.set_query(query.c_str(), query.length());
    return trie->lookup(agent);
}
template <>
uint64_t get_memory(trie_t* trie) {
    return trie->io_size();
}
template <>
void show_stats(trie_t* trie, std::ostream& os) {}
#endif

#ifdef USE_XCDAT
#include <xcdat.hpp>
using trie_t = xcdat::Trie<true>;
template <>
std::unique_ptr<trie_t> build(std::vector<std::string>& keys) {
    std::vector<std::string_view> key_views(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
        key_views[i] = std::string_view{keys[i]};
    }
    auto trie = std::make_unique<trie_t>(xcdat::TrieBuilder::build<true>(key_views));
    return trie;
}
template <>
bool find(trie_t* trie, const std::string& query) {
    return trie->lookup(query.c_str()) != trie_t::NOT_FOUND;
}
template <>
uint64_t get_memory(trie_t* trie) {
    return trie->size_in_bytes();
}
template <>
void show_stats(trie_t* trie, std::ostream& os) {}
#endif

#ifdef USE_PDT
#include <succinct/mapper.hpp>
#include <tries/compressed_string_pool.hpp>
#include <tries/path_decomposed_trie.hpp>
using trie_t = succinct::tries::path_decomposed_trie<succinct::tries::compressed_string_pool>;
template <>
std::unique_ptr<trie_t> build(std::vector<std::string>& keys) {
    auto trie = std::make_unique<trie_t>(keys);
    return trie;
}
template <>
bool find(trie_t* trie, const std::string& query) {
    return trie->index(query) != static_cast<size_t>(-1);
}
template <>
uint64_t get_memory(trie_t* trie) {
    return succinct::mapper::size_of(*trie);
}
template <>
void show_stats(trie_t* trie, std::ostream& os) {}
#endif

template <class T>
void main_template(std::vector<std::string>& keys, std::vector<std::string>& queries, const char* title) {
    using clock_type = std::chrono::high_resolution_clock;

    std::cout << "[" << title << "]" << std::endl;

    auto tp = clock_type::now();
    auto trie = build<T>(keys);
    const double constr_sec = std::chrono::duration_cast<std::chrono::seconds>(clock_type::now() - tp).count();
    std::cout << "- construction time: " << constr_sec << " seconds" << std::endl;

    // warmup
    size_t ok = 0, ng = 0;
    for (const auto& query : queries) {
        if (find(trie.get(), query)) {
            ++ok;
        } else {
            ++ng;
        }
    }
    std::cout << "- search results: " << ok << " ok; " << ng << " ng" << std::endl;

    double saerch_times[SEARCH_RUNS];
    std::cout << "- search times: ";

    for (int i = 0; i < SEARCH_RUNS; ++i) {
        tp = clock_type::now();
        for (const auto& query : queries) {
            if (find(trie.get(), query)) {
                ++ok;
            } else {
                ++ng;
            }
        }
        const double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(clock_type::now() - tp).count();
        saerch_times[i] = elapsed / queries.size();
        std::cout << "(" << i + 1 << ")" << saerch_times[i] << "; ";
    }
    std::cout << "(ave)" << std::accumulate(saerch_times, saerch_times + SEARCH_RUNS, 0.0) / SEARCH_RUNS
              << " microseconds per query" << std::endl;

    const size_t mem = get_memory(trie.get());
    std::cout << "- memory usage: " << mem << " bytes; " << mem / (1024.0 * 1024.0) << " MiB" << std::endl;

    show_stats(trie.get(), std::cout);
}

int main(int argc, char* argv[]) {
    std::ios::sync_with_stdio(false);

    cmdline::parser p;
    p.add<std::string>("key_fn", 'k', "input file name of keywords", true);
    p.add<std::string>("query_fn", 'q', "input file name of queries", true);
    p.parse_check(argc, argv);

    const auto key_fn = p.get<std::string>("key_fn");
    const auto query_fn = p.get<std::string>("query_fn");

    auto keys = load_strings(key_fn);
    auto queries = load_strings(query_fn);

#ifdef USE_FST
    main_template<trie_t>(keys, queries, "FST");
#endif
#ifdef USE_DARTSC
    main_template<trie_t>(keys, queries, "DARTSC");
#endif
#ifdef USE_TX
    main_template<trie_t>(keys, queries, "TX");
    std::remove(TX_INDEX);
#endif
#ifdef USE_MARISA
    main_template<trie_t>(keys, queries, "MARISA");
#endif
#ifdef USE_XCDAT
    main_template<trie_t>(keys, queries, "XCDAT");
#endif
#ifdef USE_PDT
    main_template<trie_t>(keys, queries, "PDT");
#endif

    return 0;
}