#!/usr/bin/python
#
# Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
#
# Jansson is free software; you can redistribute it and/or modify
# it under the terms of the MIT license. See LICENSE for details.

import simplejson
import sys

def load(filename):
    try:
        jsonfile = open(filename)
    except IOError, err:
        print >>sys.stderr, "unable to load %s: %s" % \
            (filename, err.strerror)
        sys.exit(1)

    try:
        json = simplejson.load(jsonfile)
    except ValueError, err:
        print "%s is malformed: %s" % (filename, err)
        sys.exit(1)
    finally:
        jsonfile.close()

    return json

def main():
    if len(sys.argv) != 3:
        print >>sys.stderr, "usage: %s json1 json2" % sys.argv[0]
        return 2

    json1 = load(sys.argv[1])
    json2 = load(sys.argv[2])
    if json1 == json2:
        return 0
    else:
        return 1

if __name__ == '__main__':
    sys.exit(main() or 0)
