#!/usr/bin/env python3

import sys
import math

from operator import xor

#
# help and errors
#

def print_help_and_exit():
    print("Output tables of 16-bit movement vectors or sine/cosine values based on a given step size.")
    print("usage:")
    print(f'  {sys.argv[0]} vectors <steps> <length>')
    print(f'  {sys.argv[0]} sincos <steps>')
    print(f'  {sys.argv[0]} tan <steps>')
    
    quit()


def print_param_not_number_and_exit(param):
    print(f'<{param}> should be an integer greater than 0')
    quit()


#
# helpers
#

def to_hex_string(value, negate = False, bits = 16):
    if value < 0 or negate:
        value = xor(abs(value) - 1, 2 ** bits - 1)
    fmt = '${:0' + str(bits // 4) + 'x}'
    return fmt.format(value)


# convert a float number to a 2.14 fixed point format displayed in hex
# TODO: work out how this generated '$-10000' and fix it
def to_2_14_fixed_point_string(number):
    num = abs(number)
    if num >= 2:
        return "ERR!"

    fp = 0

    # check 1/2^n (0-14) and add 2^(14-n)
    for n in range(0,15):
        check = 1 / 2 ** n
        bit = 2 ** (14 - n)
        if num >= check:
            num -= check
            fp += bit
    
    return to_hex_string(fp, number < 0)

# convert a float number to a 3.13 fixed point format displayed in hex
# TODO: work out how this generated '$-10000' and fix it
def to_3_13_fixed_point_string(number):
    num = abs(number)
    if num >= 4:
        return "ERR!"

    fp = 0

    if num >= 2:
        num -= 2
        fp += 2 ** 14

    # check 1/2^n (0-14) and add 2^(14-n)
    for n in range(0,14):
        check = 1 / 2 ** n
        bit = 2 ** (13 - n)
        if num >= check:
            num -= check
            fp += bit
    
    return to_hex_string(fp, number < 0)

#
# calculate all values of sine for a complete circle, divided
#   into `steps` parts
#
def calc_sin(steps = 2048, limit = math.pi * 2):
    step = math.pi / (steps / 2)
    radians = 0
    results = []
    while radians <= limit:
        results.append(math.sin(radians))
        radians += step
    return results

#
# calculate all values of cosine for a complete circle, divided
#   info `steps` parts
#
def calc_cos(steps = 2048, limit = math.pi * 2):
    step = math.pi / (steps / 2)
    radians = 0
    results = []
    while radians <= limit:
        results.append(math.cos(radians))
        radians += step
    return results

#
# calculate all values of cosine for a complete circle, divided
#   info `steps` parts
#
def calc_tan(steps = 2048, limit = math.pi * 2):
    step = math.pi / (steps / 2)
    radians = 0
    results = []
    max_tan = 0
    min_tan = 0
    while radians <= limit:
        t = math.tan(radians)
        print(t)
        if t > max_tan:
            max_tan = t
        elif t < min_tan:
            min_tan = t
        results.append(t)
        radians += step

    print(max_tan,min_tan)
    return results


# calculate a table of vectors representing a movement of `length`
# in a given direction
def calc_vectors(steps = 2048, length = 150):
    step = math.pi / (steps / 2)
    limit = math.pi * 2 # full circle
    radians = limit
    results =[]
    while radians >= 0:
        results.append(round( -length * math.sin(radians)))
        results.append(round(  length * math.cos(radians)))
        radians -= step
    return results


# output in a form which can be used by the compiler
# assume all the values have already been transformed
def output_defines(label, data, cols = 8):
    print(f'{label}:')
    for start in range(0, len(data), cols):
        end = start + cols if start + cols < len(data) else len(data)
        row = ', '.join(data[start:end])
        print(f'\tdc.w\t{row}')


#
# main functions
#


def parse_vectors():
    #
    # cmd vectors steps length
    #
    if len(sys.argv) < 4:
        print_help_and_exit()
    
    steps = int(sys.argv[2])
    if not steps or steps < 0:
        print_param_not_number_and_exit("steps")

    length = int(sys.argv[3])
    if not length or length < 0:
        print_param_not_number_and_exit("length")

    label = "_vectors_" + str(length)
    vectors = [to_hex_string(v) for v in calc_vectors(steps, length)]
    output_defines(label, vectors)


def parse_sincos():
    #
    # cmd sincos steps
    #
    if len(sys.argv) < 3:
        print_help_and_exit()

    steps = int(sys.argv[2])
    if not steps or steps < 0:
        print_param_not_number_and_exit("steps")

    label = "_sincos_" + str(steps)

    sin = [to_2_14_fixed_point_string(s) for s in calc_sin(steps)]
    cos = [to_2_14_fixed_point_string(c) for c in calc_cos(steps)]
    sincos = [item for sublist in zip(sin, cos) for item in sublist]

    output_defines(label, sincos)


def parse_tan():
    #
    # cmd tan steps
    #
    if len(sys.argv) < 3:
        print_help_and_exit()

    steps = int(sys.argv[2])
    if not steps or steps < 0:
        print_param_not_number_and_exit("steps")

    label = "_tan_" + str(steps)

    tan = [to_3_13_fixed_point_string(t) for t in calc_tan(steps, math.pi * 0.4)]

    output_defines(label, tan)


if len(sys.argv) < 2:
    print_help_and_exit()

match sys.argv[1]:
    case 'vectors':
        parse_vectors()
    case 'sincos':
        parse_sincos()
    case 'tan':
        parse_tan()
    case _:
        print_help_and_exit()
