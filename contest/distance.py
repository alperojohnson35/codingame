#!/usr/bin/env python3
import math
import argparse

parser = argparse.ArgumentParser(description='Compute distance 2')
parser.add_argument('integers', metavar='N', type=int, nargs='+',
                    help='an integer for the accumulator')

args = parser.parse_args()
print(math.sqrt(math.pow(args.integers[0] - args.integers[2], 2) + math.pow(args.integers[1] - args.integers[3], 2)))