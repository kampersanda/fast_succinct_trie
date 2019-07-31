#!/bin/sh

exe() { echo "\$ $@" ; "$@" ; }

EXE_DIR=$1
KEY_PATH=$2
QUERY_PATH=$3

exe ./$EXE_DIR/bench_fst -k $KEY_PATH -q $QUERY_PATH
exe ./$EXE_DIR/bench_dartsc -k $KEY_PATH -q $QUERY_PATH
exe ./$EXE_DIR/bench_xcdat -k $KEY_PATH -q $QUERY_PATH
exe ./$EXE_DIR/bench_tx -k $KEY_PATH -q $QUERY_PATH
exe ./$EXE_DIR/bench_marisa -k $KEY_PATH -q $QUERY_PATH
exe ./$EXE_DIR/bench_pdt -k $KEY_PATH -q $QUERY_PATH
