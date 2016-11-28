#!/usr/bin/env python


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

import doctest
import unittest

from . import sentencebreak
from .db import iter_sentence_break_tests
from .test import implement_break_tests


@implement_break_tests(sentencebreak.sentence_boundaries,
                       iter_sentence_break_tests())
class SentenceBreakTest(unittest.TestCase):
    pass


def load_tests(loader, tests, ignore):
    
    tests.addTests(doctest.DocTestSuite(sentencebreak))
    return tests


if __name__ == '__main__':
    unittest.main()
