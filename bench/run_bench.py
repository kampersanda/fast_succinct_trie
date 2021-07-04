#!/usr/bin/env python3

import os
import re
import subprocess
from datetime import datetime
from collections import namedtuple
from argparse import ArgumentParser

BUILD_DIR = './build'

COMMANDS = [
    'bench_cedar',
    'bench_cedarpp',
    'bench_dartsc',
    'bench_dastrie',
    'bench_hattrie',
    'bench_tx',
    'bench_marisa',
    'bench_fst',
    'bench_pdt',
    'bench_xcdat_8',
    'bench_xcdat_16',
    'bench_xcdat_7',
    'bench_xcdat_15',
]


def run_command(cmd):
    print(cmd)
    output = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            encoding='utf-8', shell=True)
    return output.stdout


def main():
    parser = ArgumentParser()
    parser.add_argument('input_keys')
    args = parser.parse_args()

    now = datetime.now().strftime('%Y%m%d%H%M%S')
    input_keys = args.input_keys

    fout = open(f'log-{now}.json', 'wt')
    for command in COMMANDS:
        cmd = f'{BUILD_DIR}/{command} {input_keys}'
        stdout = run_command(cmd)
        fout.write(stdout)
    fout.close()


if __name__ == "__main__":
    main()
