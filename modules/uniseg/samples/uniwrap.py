#!/usr/bin/env python

from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)
import argparse
import io
import sys
from locale import getpreferredencoding

from uniseg.wrap import tt_wrap


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
    
    encoding = getpreferredencoding()
    
    parser = argparse.ArgumentParser()
    parser.add_argument('-e', '--encoding',
                        default=encoding,
                        help='file encoding (%(default)s)')
    parser.add_argument('-r', '--ruler',
                        action='store_true',
                        help='show ruler')
    parser.add_argument('-t', '--tab-width',
                        type=int,
                        default=8,
                        help='tab width (%(default)d)')
    parser.add_argument('-l', '--legacy',
                        action='store_true',
                        help='treat ambiguous-width letters as wide')
    parser.add_argument('-o', '--output',
                        default='-',
                        help='leave output to specified file')
    parser.add_argument('-w', '--wrap-width',
                        type=int,
                        default=60,
                        help='wrap width (%(default)s)')
    parser.add_argument('-c', '--char-wrap',
                        action='store_true',
                        help="""wrap on grapheme boundaries instead of 
                        line break boundaries""")
    parser.add_argument('file',
                        nargs='?',
                        default='-',
                        help='input file')
    args = parser.parse_args()
    
    ruler       = args.ruler
    tab_width   = args.tab_width
    wrap_width  = args.wrap_width
    char_wrap   = args.char_wrap
    legacy      = args.legacy
    encoding    = args.encoding
    fin         = argopen(args.file, 'r', encoding)
    fout        = argopen(args.output, 'w', encoding)
    
    if ruler:
        if tab_width:
            ruler = ('+'+'-'*(tab_width-1)) * (wrap_width//tab_width+1)
            ruler = ruler[:wrap_width]
        else:
            ruler = '-' * wrap_width
        print(ruler, file=fout)
    
    for para in fin:
        for line in tt_wrap(para, wrap_width, tab_width,
                            ambiguous_as_wide=legacy, char_wrap=char_wrap):
            print(line.rstrip('\n'), file=fout)


if __name__ == '__main__':
    main()
