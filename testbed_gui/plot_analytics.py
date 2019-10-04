#!/usr/bin/env python3

#import pandas as pd
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from collections import defaultdict
import csv
import matplotlib.pyplot as plt

def plot_loss(args, plots_x, plots_y):
    plt.clf()
    plt.title("Loss graph for %s"%args.dataset_name)
    plt.ylabel("%Packets lost")
    plt.xlabel("Time in seconds")
    for key in sorted(list(plots_x.keys())):
        plt.plot(plots_x[key], plots_y[key], '.', markersize=2,
                 label="%s, %d datapoints"%(key, len(plots_x[key])))
    plt.legend()
    if args.write:
        plt.savefig("%s_loss.pdf"%args.input_file)
    else:
        plt.show()


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
#        plt.plot(plots_x[key], plots_y[key], '.',
#                 label="%s, %d datapoints"%(key, len(plots_x[key])))
        plt.plot(plots_x[key], plots_y[key], '.', markersize=2,
                 label="%s, %d datapoints"%(key, len(plots_x[key])))
    plt.legend()
    if args.write:
        plt.savefig("%s_latency.pdf"%args.input_file)
    else:
        plt.show()

if __name__=="__main__":

    # column titles from testbed_robot_callbacks.cpp, network_data_table_model.py:
    # ["Src-Dest", "Subscription", "Size", "Index", "Time Sent ns", "Latency ms"]

    parser = ArgumentParser(description="Plot latency graph for network flows.",
                        formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument("dataset_name", type=str,
                    help="The name of this dataset.")
    parser.add_argument("-i","--input_file", type=str,
                        help="The CSV data input file.",
                        default = "_infile.csv")
    parser.add_argument("-m","--max_ms_latency", type=float,
                help="The maximum ms latency allowed without being dropped.",
                        default = 20)
    parser.add_argument("-w","--write", action="store_true",
                    help="Write to <input_file>.pdf.")
    args = parser.parse_args()

    plots_x_loss=defaultdict(list)
    plots_y_loss=defaultdict(list)
    plots_x_latency=defaultdict(list)
    plots_y_latency=defaultdict(list)
    latency_outliers = 0
    with open(args.input_file) as f:
#        row=csv.reader(f, quoting=csv.QUOTE_NONNUMERIC)
        row=csv.reader(f)
        max_y=args.max_ms_latency
        t0=None
        for c in row:
            key = "%s, %s, %s bytes"%(c[0],c[1],c[2])
            time = float(c[5])/1000000000
            if not t0:
                t0 = time

            # loss
            plots_x_loss[key].append(time-t0)
            percent_loss = float(c[4])
            plots_y_loss[key].append(percent_loss)

            # latency
            latency = float(c[6])
            if latency > max_y:
                latency_outliers += 1
            else:
                plots_x_latency[key].append(time-t0)
                plots_y_latency[key].append(latency)

    plot_loss(args, plots_x_loss, plots_y_loss)
    plot_latency(args, latency_outliers, plots_x_latency, plots_y_latency)

