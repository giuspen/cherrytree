#!/usr/bin/env python


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

import doctest
import unittest

from . import wordbreak
from .db import iter_word_break_tests
from .test import implement_break_tests


@implement_break_tests(wordbreak.word_boundaries,
                       iter_word_break_tests())
class WordBreakTest(unittest.TestCase):
    pass


def load_tests(loader, tests, ignore):
    
    tests.addTests(doctest.DocTestSuite(wordbreak))
    return tests


if __name__ == '__main__':
    unittest.main()
