#! /usr/bin/python3

import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-m", action="store_true", help="Runs Make before user program")
parser.add_argument("-v", action="store_true", help="Run user terminal")
parser.add_argument("program", help="user program to run")
parser.add_argument("programargs", help="optional arguments for user program", default=[], nargs=argparse.REMAINDER)

args = parser.parse_args()

if args.m:
    try:
        subprocess.call("make")
    except ErrorReturnCode:
        print("Make failed, exiting")
        exit(1)

command = "pintos -p ../examples/{0} -a {0}"

if args.v: command += " -v"

command += " -k --fs-disk=2 -- -f -q run {0}"

command = command.format(args.program)

if args.programargs: command += " " + " ".join(args.programargs)

print(command)

subprocess.call(command, shell=True)
