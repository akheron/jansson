#!/usr/bin/python
#
# Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
#
# Jansson is free software; you can redistribute it and/or modify
# it under the terms of the MIT license. See LICENSE for details.

import os
import sys
from optparse import OptionParser

def strip_file(filename):
    with open(filename) as fobj:
        data = fobj.read()
    with open(filename, 'w') as fobj:
        fobj.write(data.strip())

def open_files(outdir, i, name):
    basename = '%02d_%s' % (i, name)
    print basename
    input_path = os.path.join(outdir, basename + '.in')
    output_path = os.path.join(outdir, basename + '.out')
    return open(input_path, 'w'), open(output_path, 'w')

def main():
    parser = OptionParser('usage: %prog [options] inputfile outputdir')
    parser.add_option('--strip', help='strip whitespace from input',
                      action='store_true', default=False)
    options, args = parser.parse_args()

    if len(args) != 2:
        parser.print_help()
        return 2

    infile = os.path.normpath(args[0])
    outdir = os.path.normpath(args[1])

    if not os.path.exists(outdir):
        print >>sys.stderr, 'output directory %r does not exist!' % outdir
        return 1

    n = 0
    current = None
    input, output = None, None

    for line in open(infile):
        if line.startswith('==== '):
            n += 1
            if input is not None and output is not None:
                input.close()
                output.close()
                if options.strip:
                    strip_file(input.name)
            input, output = open_files(outdir, n, line[5:line.find(' ====\n')])
            current = input
        elif line == '====\n':
            current = output
        else:
            current.write(line)

    if input is not None and output is not None:
        input.close()
        output.close()

    print >>sys.stderr, "%s: %d test cases" % (infile, n)

if __name__ == '__main__':
    sys.exit(main() or 0)
