#!/usr/bin/env python


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

import doctest
import unittest

from . import wrap


def load_tests(loader, tests, ignore):
    
    tests.addTests(doctest.DocTestSuite(wrap))
    return tests


if __name__ == '__main__':
    unittest.main()
