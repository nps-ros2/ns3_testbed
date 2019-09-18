#!/usr/bin/env python3

#import pandas as pd
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
import matplotlib.pyplot as plt
import csv
from collections import defaultdict

# from testbed_robot_callbacks.cpp:
# // message summary
#  std::stringstream ss;
#  ss << msg->source << "-" << r_ptr->r << "," // src-dest
#     << msg->message_name << ","              // subscription name
#     << msg->message.size() << ","            // size of these records
#     << msg->message_number << ","            // message number
#     << msg->nanoseconds << ","               // time sent in nanoseconds
#     << (_now_nanoseconds() - msg->nanoseconds) / 1000000.0; // delta ms

# from network_data_table_model.py:
#        self.column_titles = ["Src-Dest", "Subscription", "Size", "Index",
#                              "Time Sent ns", "Latency ms"]
 

parser = ArgumentParser(description="Plot latency graph for network flows.",
                        formatter_class=ArgumentDefaultsHelpFormatter)

parser.add_argument("dataset_name", type=str,
                    help="The name of this dataset.")
parser.add_argument("-i","--input_file", type=str,
                        help="The CSV data input file.",
                        default = "_infile.csv")
args = parser.parse_args()

plots_x=defaultdict(list)
plots_y=defaultdict(list)
matrix=list()
with open(args.input_file) as f:
#    row=csv.reader(f, quoting=csv.QUOTE_NONNUMERIC)
    row=csv.reader(f)
    max_y=20
    t0=None
    for c in row:
        key = "%s, %s, %s bytes"%(c[0],c[1],c[2])
        time = float(c[4])/1000000000
        if not t0:
            t0 = time
        latency = float(c[5])
        if latency > max_y:
            latency=max_y
        plots_x[key].append(time-t0)
#        plots_y[key].append(float(c[5]))
        plots_y[key].append(latency)

plt.title("Latency graph for %s"%args.dataset_name)
plt.ylabel("latency in milliseconds")
plt.xlabel("time in seconds")
for key in sorted(list(plots_x.keys())):
#    plt.plot(plots_x[key], plots_y[key], 'o',
#             label="%s, %d datapoints"%(key, len(plots_x[key])))
    plt.plot(plots_x[key], plots_y[key], 'o', markersize=2,
             label="%s, %d datapoints"%(key, len(plots_x[key])))
plt.legend()
plt.show()
#plt.savefig("%s.pdf"%args.input_file)

