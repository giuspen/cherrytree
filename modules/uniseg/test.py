"""Test submodules and provide utility functions for testing
"""


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

import locale
import unittest
from functools import wraps

from .codepoint import unichr, code_points


preferred_encoding = locale.getpreferredencoding()


def parse_breaking_test_pattern(pattern):
    
    BREAK = '\u00f7'
    DONT_BREAK = '\u00d7'
    
    codepoint_list = []
    breakpoint_list = []
    codepoint_count = 0
    for token in pattern.split():
        if token == BREAK:
            breakpoint_list.append(codepoint_count)
        elif token == DONT_BREAK:
            pass
        else:
            cp = int(token, 16)
            u = unichr(cp)
            codepoint_list.append(u)
            codepoint_count += len(u)
    string = ''.join(codepoint_list)
    expect = breakpoint_list
    return string, expect


def implement_break_tests(func_boundaries, test_iter, skips=None):
    
    if skips is None:
        skips = []
    
    def create_test_func(name, string, expect, comment):
        
        def _test(self):
            result = list(func_boundaries(string))
            msg = '%r expects %r but %r' % (string, expect, result)
            self.assertEqual(expect, result, msg)
        
        _test.__name__ = str('test_%s' % name)
        _test.__doc__ = comment
        
        return _test
    
    def decolator(cls):
        
        for name, pattern, comment in test_iter:
            string, expect = parse_breaking_test_pattern(pattern)
            f = create_test_func(name, string, expect, comment)
            if (string, expect) in skips:
                f = unittest.skip('test may be wrong.')(f)
            setattr(cls, f.__name__, wraps(f)(lambda *a, **k: f(*a, **k)))
        
        return cls
    
    return decolator


if __name__ == '__main__':
    
    loader = unittest.defaultTestLoader
    suite = loader.discover('.', '*test.py')
    runner = unittest.TextTestRunner()
    runner.run(suite)
