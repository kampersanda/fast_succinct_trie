#include <fstream>
#include <iostream>

#include <fst.hpp>

int main() {
    std::vector<std::string> keys = {
        "ACML",  "AISTATS", "DS",    "DSAA",   "ICDM",   "ICML",  //
        "PAKDD", "SDM",     "SIGIR", "SIGKDD", "SIGMOD",
    };

    // a trie-index constructed from string keys sorted
    fst::Trie trie(keys);

    // keys are mapped to unique integers in the range [0,#keys)
    std::cout << "[searching]" << std::endl;
    for (size_t i = 0; i < keys.size(); ++i) {
        fst::position_t key_id = trie.exactSearch(keys[i]);
        std::cout << " - " << keys[i] << ": " << key_id << std::endl;
    }

    std::cout << "[statistics]" << std::endl;
    std::cout << " - number of keys: " << trie.getNumKeys() << std::endl;
    std::cout << " - memory usage: " << trie.getMemoryUsage() << " bytes" << std::endl;
    std::cout << " - output file size: " << trie.getSizeIO() << " bytes" << std::endl;

    // write the trie-index to a file
    {
        std::ofstream ofs("fst.idx");
        trie.save(ofs);
    }

    // read the trie-index from a file
    {
        fst::Trie trie;
        std::ifstream ifs("fst.idx");
        trie.load(ifs);
    }

    std::remove("fst.idx");
    return 0;
}