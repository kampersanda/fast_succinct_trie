#!/usr/bin/env python3

import matplotlib
import matplotlib.pyplot as plt
import json
from argparse import ArgumentParser

plt.style.use('ggplot')

MiB = 1024*1024
CNAMES = matplotlib.colors.cnames
PROPS = {
    'XCDAT_8': {'label': 'xcdat<8>', 'marker': '^', 'ls': 'None', 'color': CNAMES['red']},
    'XCDAT_7': {'label': 'xcdat<7>', 'marker': 'v', 'ls': 'None', 'color': CNAMES['red']},
    'XCDAT_16': {'label': 'xcdat<16>', 'marker': '^', 'ls': 'None', 'color': CNAMES['red'], 'mfc': 'None'},
    'XCDAT_15': {'label': 'xcdat<15>', 'marker': 'v', 'ls': 'None', 'color': CNAMES['red'], 'mfc': 'None'},

    'CEDAR': {'label': 'cedar', 'marker': 'o', 'ls': 'None', 'color': CNAMES['blue']},
    'CEDARPP': {'label': 'cedarpp', 'marker': 'o', 'ls': 'None', 'color': CNAMES['blue'], 'mfc': 'None'},
    'DARTSC': {'label': 'darts-clone', 'marker': 's', 'ls': 'None', 'color': CNAMES['skyblue']},
    'DASTRIE': {'label': 'dastrie', 'marker': 's', 'ls': 'None', 'color': CNAMES['darkorange'], 'mfc': 'None'},

    'TX': {'label': 'tx', 'marker': 'o', 'ls': 'None', 'color': CNAMES['red'], 'mfc': 'None'},
    'MARISA': {'label': 'marisa', 'marker': 'D', 'ls': 'None', 'color': CNAMES['yellow']},
    'FST': {'label': 'fst', 'marker': 'D', 'ls': 'None', 'color': CNAMES['green'], 'mfc': 'None'},
    'PDT': {'label': 'pdt', 'marker': 'x', 'ls': 'None', 'color': CNAMES['purple'], 'mfc': 'None'},

    'HATTRIE': {'label': 'hat-trie', 'marker': '+', 'ls': 'None', 'color': CNAMES['dimgrey'], 'mfc': 'None'},
}


def load_logs(input_path):
    logs_dict = {}
    for json_str in open(input_path, 'rt'):
        obj = json.loads(json_str)
        logs_dict[obj['name']] = obj
    return logs_dict


def plot_lookup_vs_memory(logs_dict):
    # fig, ax = plt.subplots(figsize=(4.5, 3.5))
    fig, ax = plt.subplots()
    for name, prop in PROPS.items():
        if not name in logs_dict:
            continue
        log_dict = logs_dict[name]
        x = int(log_dict['memory_in_bytes']) / MiB
        y = float(log_dict['lookup_us_per_query'])
        ax.plot(x, y, **prop)
    ax.legend(loc='upper right')
    ax.set_xlabel('Memory usage (MiB)')
    ax.set_ylabel('Lookup time (microsec/query)')
    xmin, xmax = ax.get_xlim()
    ax.set_xlim(0, xmax)
    ymin, ymax = ax.get_ylim()
    ax.set_ylim(0, ymax)
    fig.tight_layout()
    fig.savefig('lookup_vs_memory.png')
    plt.close(fig)


def main():
    parser = ArgumentParser()
    parser.add_argument('input_path')
    args = parser.parse_args()

    logs_dict = load_logs(args.input_path)
    plot_lookup_vs_memory(logs_dict)


if __name__ == "__main__":
    main()
