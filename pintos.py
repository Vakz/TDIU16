#! /usr/bin/python3

import subprocess
import argparse
from os import getcwd
from os.path import basename

minpath = "pintos/src/userprog"

cwd = basename(getcwd())

if cwd != "userprog":
    pos = minpath.find(cwd)
    movepath = minpath[minpath.find("/", pos):]
    from os import chdir
    chdir(getcwd() + movepath)


parser = argparse.ArgumentParser()
parser.add_argument("-m", action="store_true", help="Runs Make before user program")
parser.add_argument("-v", action="store_true", help="Run user terminal")
parser.add_argument("-u", action="store_true", help="Run Make on examples")
parser.add_argument("program", help="user program to run")
parser.add_argument("programargs", help="optional arguments for user program", default=[], nargs=argparse.REMAINDER)

args = parser.parse_args()

if args.m and subprocess.call("make", shell=True) != 0:
    print("Make failed, exiting")
    exit(1)

if args.u and subprocess.call("make -C ../examples", shell=True) != 0:
    print("Make Examples failed, exiting")
    exit(1)

command = "pintos -p ../examples/{0} -a testfile"

if args.v: command += " -v"

command += " -k --fs-disk=2 -- -f -q run 'testfile"

command = command.format(args.program)

if args.programargs: command += " " + " ".join(args.programargs)

command += "'"

print(command)

try:
    subprocess.call(command, shell=True)
except KeyboardInterrupt:
    print("Keyboard Interrupt. Terminating..")
