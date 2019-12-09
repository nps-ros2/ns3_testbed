#!/usr/bin/env python3

from argparse import ArgumentParser
import subprocess
import sys

# To start nns1: sudo ip netns exec nns1 /bin/bash
# For status about network spaces: sudo ip netns list
# For status about network devices: ip a

# calculate IP values given index starting at 1
def _ip_from_index(i):
    if i < 1 or i > 253//3: # 84
        raise Exception("bad")

    # wifi IP values are contiguous triplets 10.0.0.x starting at 10.0.0.1
    x=(i-1)*3
    wifi_veth = "10.0.0.%d/24"%(x+1)
    wifi_vethb = "10.0.0.%d/24"%(x+2)
    wifi_tap = "10.0.0.%d/24"%(x+3)

    return wifi_veth, wifi_vethb, wifi_tap

# calculate MAC values given index starting at 1
def _mac_from_index(i):
    if i < 1 or i > 253//3: # 84
        raise Exception("bad")

    # wifi MAC values, https://superuser.com/questions/725467/set-mac-address-fails-rtnetlink-answers-cannot-assign-requested-address/725472
    mac1="12:%02d:00:00:00:00"%i
    mac2="22:%02d:00:00:00:00"%i
    mac3="32:%02d:00:00:00:00"%i
    return mac1, mac2, mac3

# create tap device: ip tuntap add tap1 mode tap; ip link set dev tap1 up

def _run_cmd(cmd):
    print("Command: %s"%cmd)
    subprocess.run(cmd.split(), stdout=subprocess.PIPE).stdout.decode('utf-8')

def do_setup_nns(i):

    # network namespace
    _run_cmd("ip netns add nns%d"%i)

    # enable loopback - helps Ctrl-C activate on first rather than second
    _run_cmd("ip netns exec nns%d ip link set dev lo up"%i)

def do_setup_wifi(i):

    wifi_veth, wifi_vethb, wifi_tap, = _ip_from_index(i)
    mac_veth, mac_vethb, mac_wifi_tap = _mac_from_index(i)

    # create veth pair
    _run_cmd("ip link add wifi_veth%d type veth peer name wifi_vethb%d"%(i,i))
    _run_cmd("ip link set dev wifi_veth%d address %s"%(i, mac_veth))
    _run_cmd("ip link set dev wifi_vethb%d address %s"%(i, mac_vethb))
    _run_cmd("ip address add %s dev wifi_vethb%d"%(wifi_vethb,i))

    # associate wifi_veth with nns
    _run_cmd("ip link set wifi_veth%d netns nns%d"%(i,i))

    # set wifi_veth IP at nns
    _run_cmd("ip netns exec nns%d ip addr add %s "
             " dev wifi_veth%d"%(i, wifi_veth, i))

    # create bridge
    _run_cmd("ip link add name wifi_br%d type bridge"%i)

    # bring up bridge and veth pair
    _run_cmd("ip link set wifi_br%d up"%i)
    _run_cmd("ip link set wifi_vethb%d up"%i)
    _run_cmd("ip netns exec nns%d ip link set wifi_veth%d up"%(i,i))

    # add wifi_vethb to bridge
    _run_cmd("ip link set wifi_vethb%d master wifi_br%d"%(i,i))

    # create tap device
    _run_cmd("ip tuntap add wifi_tap%d mode tap"%i)
    _run_cmd("ip link set dev wifi_tap%d address %s"%(i, mac_wifi_tap))
    _run_cmd("ip addr flush dev wifi_tap%d"%i) # clear IP
    _run_cmd("ip address add %s dev wifi_tap%d"%(wifi_tap, i))
    _run_cmd("ip link set wifi_tap%d up"%i)

    # add tap to bridge
    _run_cmd("ip link set wifi_tap%d master wifi_br%d"%(i,i))


def do_teardown_wifi(i):
    # wifi
    _run_cmd("ip link set wifi_br%d down"%i)
    _run_cmd("ip link set wifi_tap%d down"%i)
    _run_cmd("ip link set wifi_vethb%d down"%i)
    _run_cmd("ip link delete wifi_vethb%d"%i)
    _run_cmd("ip link delete wifi_tap%d"%i)
    _run_cmd("ip link delete wifi_br%d type bridge"%i)

def do_teardown_nns(i):
    # network namespace
    _run_cmd("ip netns del nns%d"%i)


if __name__=="__main__":
    DEFAULT_COUNT = 30
    parser = ArgumentParser(prog='lxc_setup.py',
                            description="Manage containers used with ns-3.")
    parser.add_argument("command", type=str, help="The command to execute.",
                        choices=["setup", "teardown"])
    parser.add_argument("-c", "--count", type=int, default=DEFAULT_COUNT,
                        help="The number of network namespaces to set up for, "
                             "default %d."%DEFAULT_COUNT)
    args = parser.parse_args()

    print("Providing '%s' services for %d network namespaces..."%(
                                             args.command, args.count))

    if args.command == "setup":
        for i in range(1,args.count+1):
            do_setup_nns(i)
            do_setup_wifi(i)
    elif args.command == "teardown":
        for i in range(1,args.count+1):
            do_teardown_wifi(i)
            do_teardown_nns(i)
    else:
        print("Invalid command: %s"%args.command)
        sys.exit(1)

    print("Done providing '%s' services for %d network namespaces."%(
                                             args.command, args.count))

