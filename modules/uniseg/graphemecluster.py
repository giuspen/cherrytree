"""Unicode grapheme cluster breaking

UAX #29: Unicode Text Segmentation (Unicode 6.2.0)
http://www.unicode.org/reports/tr29/tr29-21.html
"""


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

from .breaking import boundaries, break_units
from .codepoint import code_point, code_points
from .db import grapheme_cluster_break as _grapheme_cluster_break


__all__ = [
    'grapheme_cluster_break',
    'grapheme_cluster_breakables',
    'grapheme_cluster_boundaries',
    'grapheme_clusters',
]


Other = 0
CR = 1
LF = 2
Control = 3
Extend = 4
SpacingMark = 5
L = 6
V = 7
T = 8
LV = 9
LVT = 10
Regional_Indicator = 11

names = [
    'Other',        # 0
    'CR',           # 1
    'LF',           # 2
    'Control',      # 3
    'Extend',       # 4
    'SpacingMark',  # 5
    'L',            # 6
    'V',            # 7
    'T',            # 8
    'LV',           # 9
    'LVT',          # 10
    'Regional_Indicator',   # 11
]

# cf. http://www.unicode.org/Public/6.2.0/ucd/auxiliary/GraphemeBreakTest.html
# 0: not break, 1: break
break_table = [
    [1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1],
    [1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1],
    [1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1],
    [1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0],
]


def grapheme_cluster_break(c, index=0):
    
    r"""Return the Grapheme_Cluster_Break property of `c`
    
    `c` must be a single Unicode code point string.
    
    >>> print(grapheme_cluster_break('\x0d'))
    CR
    >>> print(grapheme_cluster_break('\x0a'))
    LF
    >>> print(grapheme_cluster_break('a'))
    Other
    
    If `index` is specified, this function consider `c` as a unicode 
    string and return Grapheme_Cluster_Break property of the code 
    point at c[index].
    
    >>> print(grapheme_cluster_break(u'a\x0d', 1))
    CR
    """
    
    return _grapheme_cluster_break(code_point(c, index))


def grapheme_cluster_breakables(s):
    
    """Iterate grapheme cluster breaking opportunities for every 
    position of `s`
    
    1 for "break" and 0 for "do not break".  The length of iteration 
    will be the same as ``len(s)``.
    
    >>> list(grapheme_cluster_breakables(u'ABC'))
    [1, 1, 1]
    >>> list(grapheme_cluster_breakables(u'\x67\u0308'))
    [1, 0]
    >>> list(grapheme_cluster_breakables(u''))
    []
    """
    
    if not s:
        return
    
    prev_gcbi = 0
    i = 0
    for c in code_points(s):
        gcb = grapheme_cluster_break(c)
        gcbi = names.index(gcb)
        if i > 0:
            breakable = break_table[prev_gcbi][gcbi]
        else:
            breakable = 1
        for j in range(len(c)):
            yield int(j==0 and breakable)
        prev_gcbi = gcbi
        i += len(c)


def grapheme_cluster_boundaries(s, tailor=None):
    
    """Iterate indices of the grapheme cluster boundaries of `s`
    
    This function yields from 0 to the end of the string (== len(s)).
    
    >>> list(grapheme_cluster_boundaries('ABC'))
    [0, 1, 2, 3]
    >>> list(grapheme_cluster_boundaries('\x67\u0308'))
    [0, 2]
    >>> list(grapheme_cluster_boundaries(''))
    []
    """
    
    breakables = grapheme_cluster_breakables(s)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return boundaries(breakables)


def grapheme_clusters(s, tailor=None):
    
    r"""Iterate every grapheme cluster token of `s`
    
    Grapheme clusters (both legacy and extended):
    
    >>> list(grapheme_clusters('g\u0308')) == ['g\u0308']
    True
    >>> list(grapheme_clusters('\uac01')) == ['\uac01']
    True
    >>> list(grapheme_clusters('\u1100\u1161\u11a8')) == ['\u1100\u1161\u11a8']
    True
    
    Extended grapheme clusters:
    
    >>> list(grapheme_clusters('\u0ba8\u0bbf')) == ['\u0ba8\u0bbf']
    True
    >>> list(grapheme_clusters('\u0937\u093f')) == ['\u0937\u093f']
    True
    
    Empty string leads the result of empty sequence:
    
    >>> list(grapheme_clusters('')) == []
    True
    
    You can customize the default breaking behavior by modifying 
    breakable table so as to fit the specific locale in `tailor` 
    function.  It receives `s` and its default breaking sequence 
    (iterator) as its arguments and returns the sequence of customized 
    breaking opportunities:

    >>> def tailor_grapheme_cluster_breakables(s, breakables):
    ...     
    ...     for i, breakable in enumerate(breakables):
    ...         # don't break between 'c' and 'h'
    ...         if s.endswith('c', 0, i) and s.startswith('h', i):
    ...             yield 0
    ...         else:
    ...             yield breakable
    ... 
    >>> s = 'Czech'
    >>> list(grapheme_clusters(s)) == ['C', 'z', 'e', 'c', 'h']
    True
    >>> list(grapheme_clusters(s, tailor_grapheme_cluster_breakables)) == ['C', 'z', 'e', 'ch']
    True
    """
    
    breakables = grapheme_cluster_breakables(s)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return break_units(s, breakables)


if __name__ == '__main__':
    import doctest
    doctest.testmod()
