# Benchmark of dictionary libraries

The code provides a benchmark of selected dictionary libraries (in offline setting).

The benchmark result in my environment is presented [here](https://github.com/kampersanda/xcdat#performance).

## Build instructions

First, at the root of this repository, download the submodules.

```sh
$ git submodule update --init --recursive
```

Then, at this directory, compile the code with the following commands:

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make -j
```

## Running example

You can test all the data structures using `scripts/run_bench.py` as follows. The results will be output in `out.json`.

```sh
$ python scripts/run_bench.py words.txt out.json
./build/bench_cedar words.txt
./build/bench_cedarpp words.txt
./build/bench_darts words.txt
./build/bench_dartsc words.txt
./build/bench_dastrie words.txt
./build/bench_hattrie words.txt
./build/bench_arrayhash words.txt
./build/bench_tx words.txt
./build/bench_marisa words.txt
./build/bench_fst words.txt
./build/bench_pdt words.txt
./build/bench_xcdat_8 words.txt
./build/bench_xcdat_16 words.txt
./build/bench_xcdat_7 words.txt
./build/bench_xcdat_15 words.txt
```

