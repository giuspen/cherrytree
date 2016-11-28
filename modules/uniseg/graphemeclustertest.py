#!/usr/bin/env python

from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

import doctest
import unittest

from . import graphemecluster
from .db import iter_grapheme_cluster_break_tests
from .test import implement_break_tests


@implement_break_tests(graphemecluster.grapheme_cluster_boundaries,
                       iter_grapheme_cluster_break_tests())
class GraphemeClusterTest(unittest.TestCase):
    pass


def load_tests(loader, tests, ignore):
    
    tests.addTests(doctest.DocTestSuite(graphemecluster))
    return tests


if __name__ == '__main__':
    unittest.main()
