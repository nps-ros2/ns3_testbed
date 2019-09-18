#!/usr/bin/env python3

#import pandas as pd
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from collections import defaultdict
import csv
import matplotlib.pyplot as plt

# column titles from testbed_robot_callbacks.cpp, network_data_table_model.py:
# ["Src-Dest", "Subscription", "Size", "Index", "Time Sent ns", "Latency ms"]

parser = ArgumentParser(description="Plot latency graph for network flows.",
                        formatter_class=ArgumentDefaultsHelpFormatter)
parser.add_argument("dataset_name", type=str,
                    help="The name of this dataset.")
parser.add_argument("-i","--input_file", type=str,
                        help="The CSV data input file.",
                        default = "_infile.csv")
parser.add_argument("-m","--max_ms_latency", type=int,
                help="The maximum ms latency allowed without being dropped.",
                        default = 20)
parser.add_argument("-w","--write", action="store_true",
                    help="Write to <input_file>.pdf.")
args = parser.parse_args()

plots_x=defaultdict(list)
plots_y=defaultdict(list)
matrix=list()
outliers = 0
with open(args.input_file) as f:
#    row=csv.reader(f, quoting=csv.QUOTE_NONNUMERIC)
    row=csv.reader(f)
    max_y=args.max_ms_latency
    t0=None
    for c in row:
        key = "%s, %s, %s bytes"%(c[0],c[1],c[2])
        time = float(c[4])/1000000000
        if not t0:
            t0 = time
        latency = float(c[5])
        if latency > max_y:
            outliers += 1
            continue
#            latency=-1
        plots_x[key].append(time-t0)
        plots_y[key].append(latency)
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
#    plt.plot(plots_x[key], plots_y[key], '.',
#             label="%s, %d datapoints"%(key, len(plots_x[key])))
    plt.plot(plots_x[key], plots_y[key], '.', markersize=2,
             label="%s, %d datapoints"%(key, len(plots_x[key])))
plt.legend()
if args.write:
    plt.savefig("%s.pdf"%args.input_file)
else:
    plt.show()

