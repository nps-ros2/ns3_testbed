#!/usr/bin/env python3

from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from collections import defaultdict
from statistics import mean
import csv
import matplotlib.pyplot as plt


def plot_latency(args, outliers, plots_x, plots_y):
    plt.clf()
    if outliers == 0:
        plt.title("Latency graph for %s"%args.dataset_name)
    elif outliers == 1:
        plt.title("Latency graph for %s\n(1 outlier dropped)"%args.dataset_name)
    else:
        plt.title("Latency graph for %s\n(%d outliers dropped)"%(args.dataset_name,
                                                             outliers))
    plt.ylabel("Latency in milliseconds")
    plt.xlabel("Time in seconds")
    for key in sorted(list(plots_x.keys())):
        plt.plot(plots_x[key], plots_y[key], '.', markersize=2,
                 label="%s, %d datapoints"%(key, len(plots_x[key])))
    plt.legend()
    if args.write:
        plt.savefig("%s_latency.pdf"%args.input_file)
    else:
        plt.show()

def plot_throughput(args, plots_x, plots_y):
    plt.clf()
    plt.title("Throughput graph for %s"%args.dataset_name)
    plt.ylabel("Bytes per second")
    plt.xlabel("Time in seconds")
    for key in sorted(list(plots_x.keys())):
        plt.plot(plots_x[key], plots_y[key], '.', markersize=2,
                 label=key)
    plt.legend()
    if args.write:
        plt.savefig("%s_throughput.pdf"%args.input_file)
    else:
        plt.show()

def plot_loss(args, plots_x, plots_y):
    plt.clf()
    plt.title("Loss graph for %s"%args.dataset_name)
    plt.ylabel("%Packets lost")
    plt.xlabel("Time in seconds")
    for key in sorted(list(plots_x.keys())):
        plt.plot(plots_x[key], plots_y[key], '.', markersize=2,
                 label=key)
    plt.legend()
    if args.write:
        plt.savefig("%s_loss.pdf"%args.input_file)
    else:
        plt.show()

if __name__=="__main__":

    parser = ArgumentParser(description="Plot latency graph for network flows.",
                        formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("input_file", type=str,
                        help="The CSV data input file.")
    parser.add_argument("dataset_name", type=str,
                    help="The name of this dataset.")
    parser.add_argument("-m","--max_ms_latency", type=float,
                help="The maximum ms latency allowed without being dropped.",
                        default = 20)
    parser.add_argument("-w","--write", action="store_true",
                    help="Write to <input_file>_<plot_type>.pdf.")
    args = parser.parse_args()

    plots_x_latency=defaultdict(list)
    plots_y_latency=defaultdict(list)
    latency_outliers = 0

    with open(args.input_file) as f:
        row=csv.reader(f)
        max_y=args.max_ms_latency
        datapoints = dict()
        t0=None
        for c in row:
            key = c[0],c[1],c[2], int(c[3]) # from, to, subscription, tx_count
            if len(c) == 5:
                if not t0:
                    t0 = int(c[4]) # ns
                # tx: from, to, subscription, tx_count, timestamp
                datapoints[key] = int(c[4]), 0, 0 # time_ns, latency, size

            elif len(c) == 7:
                # from, to, subscription, tx_count, rx_count, size, timestamp
                time = (datapoints[key][0] - t0) / 1000000000 # ns
                latency = (int(c[6]) - datapoints[key][0]) / 1000000 # ms
                datapoints[key] = int(c[6]), latency, c[5] # t_ns, latency, size

                if latency > max_y:
                    latency_outliers += 1
                else:
                    latency_key = "%s, %s, %s, %d bytes"%(c[0],c[1],c[2],
                                                          int(c[5]))
                    plots_x_latency[latency_key].append(time)
                    plots_y_latency[latency_key].append(latency)

    # statistics by second
    throughputs = defaultdict(list)
    percent_losses = defaultdict(list)
    for key, value in datapoints.items():
        # key = from, to, subscription, _tx_count
        # value = time_ns, latency, size
        stat_key = key[0],key[1],key[2],int(int((value[0])-t0)/1000000000)

        # throughput, percent_loss
        throughput = int(value[2])
        if throughput == 0:
            percent_loss = 100
        else:
            percent_loss = 0
        
        throughputs[stat_key].append(throughput)
        percent_losses[stat_key].append(percent_loss)

    # throughput
    throughputs_x = defaultdict(list)
    throughputs_y = defaultdict(list)
    for key, value in throughputs.items():
        # key = from, to, subscription, int_second
        # value = int throughputs
        key_string = "%s, %s, %s"%(key[0],key[1],key[2])
        throughputs_x[key_string].append(key[3]) # second
        throughputs_y[key_string].append(sum(value))

    # percent loss
    percent_losses_x = defaultdict(list)
    percent_losses_y = defaultdict(list)
    for key, value in percent_losses.items():
        # key = from, to, subscription, int_second
        # value = int percent_losses
        key_string = "%s, %s, %s"%(key[0],key[1],key[2])
        percent_losses_x[key_string].append(key[3]) # second
        percent_losses_y[key_string].append(mean(value))

    plot_latency(args, latency_outliers, plots_x_latency, plots_y_latency)
    plot_throughput(args, throughputs_x, throughputs_y)
    plot_loss(args, percent_losses_x, percent_losses_y)

