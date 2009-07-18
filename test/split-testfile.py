#!/usr/bin/python

import os
import sys

def open_files(outdir, i):
    return (open(os.path.join(outdir, 'test%02d.in' % i), 'w'),
            open(os.path.join(outdir, 'test%02d.out' % i), 'w'))

def close_files(input, output):
    print os.path.basename(input.name), os.path.basename(output.name)
    input.close()
    output.close()

def main():
    if len(sys.argv) != 3:
        print 'usage: %s input-file output-directory' % sys.argv[0]
        return 2

    infile = os.path.normpath(sys.argv[1])
    outdir = os.path.normpath(sys.argv[2])

    if not os.path.exists(outdir):
        print >>sys.stderr, 'output directory %r does not exist!' % outdir
        return 1

    i = 0
    input, output = open_files(outdir, i)
    current = input

    for line in open(infile):
        if line == '====\n':
            current = output
        elif line == '========\n':
            close_files(input, output)
            i += 1
            input, output = open_files(outdir, i)
            current = input
        else:
            current.write(line)

    close_files(input, output)
    print >>sys.stderr, "%s: %d test cases" % (infile, i + 1)

if __name__ == '__main__':
    sys.exit(main() or 0)
