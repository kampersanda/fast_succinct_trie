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


def plot_constr_vs_memory(logs_dict, title):
    # fig, ax = plt.subplots(figsize=(4.5, 3.5))
    fig, ax = plt.subplots()
    for name, prop in PROPS.items():
        if not name in logs_dict:
            continue
        log_dict = logs_dict[name]
        x = int(log_dict['memory_in_bytes']) / MiB
        y = float(log_dict['construction_sec'])
        ax.plot(x, y, **prop)
    ax.legend()
    ax.set_xlabel('Memory usage (MiB)')
    ax.set_ylabel('Construction time (sec)')
    xmin, xmax = ax.get_xlim()
    ax.set_xlim(0, xmax)
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(0, ymax)
    ax.set_title(title)
    fig.tight_layout()
    fig.savefig('constr_vs_memory.png')
    plt.close(fig)


def plot_lookup_vs_memory(logs_dict, title):
    # fig, ax = plt.subplots(figsize=(4.5, 3.5))
    fig, ax = plt.subplots()
    for name, prop in PROPS.items():
        if not name in logs_dict:
            continue
        log_dict = logs_dict[name]
        x = int(log_dict['memory_in_bytes']) / MiB
        y = float(log_dict['lookup_us_per_query'])
        ax.plot(x, y, **prop)
    ax.legend()
    # ax.legend(loc='upper right')
    ax.set_xlabel('Memory usage (MiB)')
    ax.set_ylabel('Lookup time (microsec/query)')
    xmin, xmax = ax.get_xlim()
    ax.set_xlim(0, xmax)
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(0, ymax)
    ax.set_title(title)
    fig.tight_layout()
    fig.savefig('lookup_vs_memory.png')
    plt.close(fig)


def plot_decode_vs_memory(logs_dict, title):
    # fig, ax = plt.subplots(figsize=(4.5, 3.5))
    fig, ax = plt.subplots()
    for name, prop in PROPS.items():
        if not name in logs_dict:
            continue
        log_dict = logs_dict[name]
        if not 'decode_us_per_query' in log_dict:
            continue
        x = int(log_dict['memory_in_bytes']) / MiB
        y = float(log_dict['decode_us_per_query'])
        ax.plot(x, y, **prop)
    ax.legend()
    # ax.legend(loc='upper left')
    ax.set_xlabel('Memory usage (MiB)')
    ax.set_ylabel('Decode time (microsec/query)')
    xmin, xmax = ax.get_xlim()
    ax.set_xlim(0, xmax)
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(0, ymax)
    ax.set_title(title)
    fig.tight_layout()
    fig.savefig('decode_vs_memory.png')
    plt.close(fig)


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
        constr = float(log_dict['construction_sec'])
        lookup = float(log_dict['lookup_us_per_query'])
        if 'decode_us_per_query' in log_dict:
            decode = float(log_dict['decode_us_per_query'])
        else:
            decode = 0.0
        print(label, memory, constr, lookup, decode, sep='\t')


if __name__ == "__main__":
    main()
