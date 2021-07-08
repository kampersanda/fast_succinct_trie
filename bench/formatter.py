#!/usr/bin/env python3

import json
from argparse import ArgumentParser

MiB = 1024*1024

PROPS = {
    'XCDAT_8': {'label': 'xcdat<8>'},
    'XCDAT_16': {'label': 'xcdat<16>'},
    'XCDAT_7': {'label': 'xcdat<7>'},
    'XCDAT_15': {'label': 'xcdat<15>'},

    'DARTS': {'label': 'darts'},
    'DARTSC': {'label': 'darts-clone'},
    'CEDAR': {'label': 'cedar'},
    'CEDARPP': {'label': 'cedarpp'},
    'DASTRIE': {'label': 'dastrie'},

    'TX': {'label': 'tx'},
    'FST': {'label': 'fst'},
    'MARISA': {'label': 'marisa'},
    'PDT': {'label': 'pdt'},

    'HATTRIE': {'label': 'hat-trie'},
}


def load_logs(input_path):
    logs_dict = {}
    for json_str in open(input_path, 'rt'):
        obj = json.loads(json_str)
        logs_dict[obj['name']] = obj
    return logs_dict


def main():
    parser = ArgumentParser()
    parser.add_argument('input_path')
    args = parser.parse_args()

    logs_dict = load_logs(args.input_path)

    for name, prop in PROPS.items():
        if not name in logs_dict:
            continue
        log_dict = logs_dict[name]

        label = prop['label']
        memory = int(log_dict['memory_in_bytes']) / MiB
        constr = float(log_dict['build_ns_per_key'])
        lookup = float(log_dict['best_lookup_ns_per_query'])
        if 'best_decode_ns_per_query' in log_dict:
            decode = float(log_dict['best_decode_ns_per_query'])
        else:
            decode = 0.0
        print(label, memory, lookup, decode, constr, sep='\t')


if __name__ == "__main__":
    main()
