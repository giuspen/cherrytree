#!/usr/bin/env python

from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)
import io
import sys

from uniseg.codepoint       import code_points
from uniseg.graphemecluster import grapheme_clusters
from uniseg.wordbreak       import words
from uniseg.sentencebreak   import sentences
from uniseg.linebreak       import line_break_units


def argopen(file, mode, encoding=None, errors=None):
    
    closefd = True
    if file == '-':
        closefd = False
        if 'r' in mode:
            file = sys.stdin.fileno()
        else:
            file = sys.stdout.fileno()
    return io.open(file, mode, encoding=encoding, errors=errors,
                   closefd=closefd)


def main():
    
    import argparse
    from locale import getpreferredencoding
    
    parser = argparse.ArgumentParser()
    parser.add_argument('-e', '--encoding',
                        default=getpreferredencoding(),
                        help="""text encoding of the input (%(default)s)""")
    parser.add_argument('-l', '--legacy',
                        action='store_true',
                        help="""legacy mode (makes sense only with
                        '--mode l')""")
    parser.add_argument('-m', '--mode',
                        choices=['c', 'g', 'l', 's', 'w'],
                        default='w',
                        help="""breaking algorithm (%(default)s)
                        (c: code points, g: grapheme clusters,
                        s: sentences l: line breaking units, w: words)""")
    parser.add_argument('-o', '--output',
                        default='-',
                        help="""leave output to specified file""")
    parser.add_argument('file',
                        nargs='?',
                        default='-',
                        help="""input text file""")
    args = parser.parse_args()
    
    encoding = args.encoding
    fin = argopen(args.file, 'r', encoding)
    fout = argopen(args.output, 'w', encoding)
    _words = {'c': code_points,
              'g': grapheme_clusters,
              'l': lambda x: line_break_units(x, args.legacy),
              's': sentences,
              'w': words,
              }[args.mode]
    for line in fin:
        for w in _words(line):
            print(w, file=fout)


if __name__ == '__main__':
    main()
