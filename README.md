# fast\_succinct\_trie

[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)

This library implements a trie-map through Fast Succinct Trie (FST), proposed in [SIGMOD 2018](http://www.cs.cmu.edu/~huanche1/publications/surf_paper.pdf).
The trie-map is implemented by using the original FST implementation [efficient/SuRF](https://github.com/efficient/SuRF). But, unique suffixes are managed in character arrays as MP-trie (c.f. [KAIS 2017](https://drive.google.com/open?id=1_BknOv1misIK-iUk4u9c9yZi3qmWNruf)).

## Build instructions

You can download and compile this library as the following commands:

```
$ git clone --recursive https://github.com/kampersanda/fast_succinct_trie.git
$ cd fast_succinct_trie
$ mkdir build && cd build
$ cmake .. -DFST_ENABLE_BENCH=ON
$ make -j4
```

Compiling third-party libraries at `bench` will be heavy.
If you do not need to run benchmarks, please put `-DFST_ENABLE_BENCH=OFF`.

## Sample usage

```cpp
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
    std::cout << " - number of nodes: " << trie.getNumNodes() << std::endl;
    std::cout << " - number of suffix bytes: " << trie.getSuffixBytes() << std::endl;
    std::cout << " - memory usage in bytes: " << trie.getMemoryUsage() << std::endl;
    std::cout << " - output file size in bytes: " << trie.getSizeIO() << std::endl;

    std::cout << "[configure]" << std::endl;
    trie.debugPrint(std::cout);

    // write the trie-index to a file
    {
        std::ofstream ofs("fst.idx");
        trie.save(ofs);
    }

    // read the trie-index from a file
    {
        fst::Trie other;
        std::ifstream ifs("fst.idx");
        other.load(ifs);
    }

    std::remove("fst.idx");
    return 0;
}
```
The output will be

```
[searching]
 - ACML: 1
 - AISTATS: 2
 - DS: 4
 - DSAA: 5
 - ICDM: 6
 - ICML: 7
 - PAKDD: 0
 - SDM: 3
 - SIGIR: 8
 - SIGKDD: 9
 - SIGMOD: 10
[statistics]
 - number of keys: 11
 - number of nodes: 19
 - number of suffix bytes: 24
 - memory usage in bytes: 607
 - output file size in bytes: 336
[configure]
-- LoudsDense (heigth=1) --
LABEL: A D I P S | 
CHILD: 1 1 1 0 1 | 
PREFX: 0         |
-- LoudsSparse --
LABEL: C I S C D I ? A D M G I K M ? 
CHILD: 0 0 1 1 0 1 0 0 0 0 1 0 0 0 
LOUDS: 1 0 1 1 1 0 1 0 1 0 1 1 0 0 
-- Suffixes --
POINTERS: 17 11 1 9 0 22 9 12 7 19 14 
SUFFIXES: ? S T A T S ? R ? M ? M L ? O D ? A K D D ? A ? 
```

## Licensing

This library is free software provided under [Apache License 2.0](https://github.com/kampersanda/fast_succinct_trie/blob/master/LICENSE), following the License of [efficient/SuRF](https://github.com/efficient/SuRF).
The modifications are denoted in each source file.
