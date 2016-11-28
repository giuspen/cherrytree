from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

import errno
import os
import sqlite3
import sys

from .codepoint import ord


def print_dbpath():

    """Print the path of the database file. """

    print(os.path.abspath(_dbpath))


def find_dbpath():

    """Find the database file in the specified order and return its path.

    The search paths (in the order of priority) are:
    1. The directory of the package,
    2. that of the executable
    3. and the current directory.
    """
    dbname = 'ucd.sqlite3'
    
    dbpath = os.path.join(os.path.dirname(__file__), dbname)
    if (os.path.exists(dbpath)):
        return dbpath

    dbpath = os.path.join(os.path.dirname(sys.executable), dbname)
    if (os.path.exists(dbpath)):
        return dbpath

    dbpath = os.path.join(os.getcwd(), dbname)
    if (os.path.exists(dbpath)):
        return dbpath

    return None


_dbpath = find_dbpath()
if _dbpath:
    _conn = sqlite3.connect(_dbpath)
else:
    _conn = None


def grapheme_cluster_break(u):
    
    cur = _conn.cursor()
    cur.execute('select value from GraphemeClusterBreak where cp = ?',
                (ord(u),))
    for value, in cur:
        return str(value)
    return 'Other'


def iter_grapheme_cluster_break_tests():
    
    cur = _conn.cursor()
    cur.execute('select name, pattern, comment from GraphemeClusterBreakTest')
    return iter(cur)


def word_break(u):
    
    cur = _conn.cursor()
    cur.execute('select value from WordBreak where cp = ?',
                (ord(u),))
    for value, in cur:
        return str(value)
    return 'Other'


def iter_word_break_tests():
    
    cur = _conn.cursor()
    cur.execute('select name, pattern, comment from WordBreakTest')
    return iter(cur)


def sentence_break(u):
    
    cur = _conn.cursor()
    cur.execute('select value from SentenceBreak where cp = ?',
                (ord(u),))
    for value, in cur:
        return str(value)
    return 'Other'


def iter_sentence_break_tests():
    
    cur = _conn.cursor()
    cur.execute('select name, pattern, comment from SentenceBreakTest')
    return iter(cur)


def line_break(u):
    
    cur = _conn.cursor()
    cur.execute('select value from LineBreak where cp = ?',
                (ord(u),))
    for value, in cur:
        return str(value)
    return 'Other'


def iter_line_break_tests():
    
    cur = _conn.cursor()
    cur.execute('select name, pattern, comment from LineBreakTest')
    return iter(cur)
