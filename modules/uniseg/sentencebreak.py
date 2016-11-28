"""sentence boundaries

UAX #29: Unicode Text Segmentation
http://www.unicode.org/reports/tr29/tr29-15.html
"""


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

from .breaking import boundaries, break_units
from .codepoint import ord, code_point, code_points
from .db import sentence_break as _sentence_break


__all__ = [
    'sentence_break',
    'sentence_breakables',
    'sentence_boundaries',
    'sentences',
]


CR = 'CR'
LF = 'LF'
Extend = 'Extend'
Sep = 'Sep'
Format = 'Format'
Sp = 'Sp'
Lower = 'Lower'
Upper = 'Upper'
OLetter = 'OLetter'
Numeric = 'Numeric'
ATerm = 'ATerm'
SContinue = 'SContinue'
STerm = 'STerm'
Close = 'Close'


def sentence_break(c, index=0):
    
    r"""Return Sentence_Break property value of `c`
    
    `c` must be a single Unicode code point string.
    
    >>> print(sentence_break(u'\x0d'))
    CR
    >>> print(sentence_break(u' '))
    Sp
    >>> print(sentence_break(u'a'))
    Lower
    
    If `index` is specified, this function consider `c` as a unicode 
    string and return Sentence_Break property of the code point at 
    c[index].
    
    >>> print(sentence_break(u'a\x0d', 1))
    CR
    """
    
    return _sentence_break(code_point(c, index))


def _preprocess_boundaries(s):
    
    r"""(internal)
    
    >>> list(_preprocess_boundaries('Aa')) == [(0, 'Upper'), (1, 'Lower')]
    True
    >>> list(_preprocess_boundaries('A a')) == [(0, 'Upper'), (1, 'Sp'), (2, 'Lower')]
    True
    >>> list(_preprocess_boundaries('A" a')) == [(0, 'Upper'), (1, 'Close'), (2, 'Sp'), (3, 'Lower')]
    True
    >>> list(_preprocess_boundaries('A\xad "')) == [(0, 'Upper'), (2, 'Sp'), (3, 'Close')]
    True
    >>> list(_preprocess_boundaries('\r\rA')) == [(0, 'CR'), (1, 'CR'), (2, 'Upper')]
    True
    """
    
    prev_prop = None
    i = 0
    for c in code_points(s):
        prop = sentence_break(c)
        if prop in (Sep, CR, LF):
            yield (i, prop)
            prev_prop = None
        elif prop in (Extend, Format):
            if prev_prop is None:
                yield (i, prop)
                prev_prop = prop
        elif prev_prop != prop:
            yield (i, prop)
            prev_prop = prop
        i += len(c)


def _next_break(primitive_boundaries, pos, expects):
    
    """(internal)
    """
    
    for i in xrange(pos, len(primitive_boundaries)):
        sb = primitive_boundaries[i][1]
        if sb in expects:
            return sb
    return None


def sentence_breakables(s):
    
    r"""Iterate sentence breaking opportunities for every position of 
    `s`
    
    1 for "break" and 0 for "do not break".  The length of iteration 
    will be the same as ``len(s)``.
    
    >>> s = 'He said, \u201cAre you going?\u201d John shook his head.'
    >>> list(sentence_breakables(s))
    [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    """
    
    primitive_boundaries = list(_preprocess_boundaries(s))
    prev_prev_prev_prev_sb = None
    prev_prev_prev_sb = None
    prev_prev_sb = None
    prev_sb = None
    pos = 0
    for i, (pos, sb) in enumerate(primitive_boundaries):
        next_pos, next_sb = (primitive_boundaries[i+1]
                             if i<len(primitive_boundaries)-1 else (len(s), None))
        if pos == 0:
            do_break = True
        # SB3
        elif prev_sb == CR and sb == LF:
            do_break = False
        # SB4
        elif prev_sb in (Sep, CR, LF):
            do_break = True
        # SB6
        elif prev_sb == ATerm and sb == Numeric:
            do_break = False
        # SB7
        elif prev_prev_sb == Upper and prev_sb == ATerm and sb == Upper:
            do_break = False
        # SB8
        elif (((prev_sb == ATerm)
               or (prev_prev_sb == ATerm
                   and prev_sb == Close)
               or (prev_prev_sb == ATerm
                   and prev_sb == Sp)
               or (prev_prev_prev_sb == ATerm
                   and prev_prev_sb == Close
                   and prev_sb == Sp))
              and _next_break(primitive_boundaries, i,
                              [OLetter, Upper, Lower, Sep, CR, LF,
                               STerm, ATerm]) == Lower):
            do_break = False
        # SB8a
        elif (((prev_sb in (STerm, ATerm))
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Close)
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Sp)
               or (prev_prev_prev_sb in (STerm, ATerm)
                   and prev_prev_sb == Close
                   and prev_sb == Sp))
              and sb in (SContinue, STerm, ATerm)):
            do_break = False
        # SB9
        elif (((prev_sb in (STerm, ATerm))
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Close))
              and sb in (Close, Sp, Sep, CR, LF)):
            do_break = False
        # SB10
        elif (((prev_sb in (STerm, ATerm))
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Close)
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Sp)
               or (prev_prev_prev_sb in (STerm, ATerm)
                   and prev_prev_sb == Close
                   and prev_sb == Sp))
              and sb in (Sp, Sep, CR, LF)):
            do_break = False
        # SB11
        elif ((prev_sb in (STerm, ATerm))
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Close)
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb == Sp)
               or (prev_prev_sb in (STerm, ATerm)
                   and prev_sb in (Sep, CR, LF))
               or (prev_prev_prev_sb in (STerm, ATerm)
                   and prev_prev_sb == Close
                   and prev_sb == Sp)
               or (prev_prev_prev_sb in (STerm, ATerm)
                   and prev_prev_sb == Close
                   and prev_sb in (Sep, CR, LF))
               or (prev_prev_prev_sb in (STerm, ATerm)
                   and prev_prev_sb == Sp
                   and prev_sb in (Sep, CR, LF))
               or (prev_prev_prev_prev_sb in (STerm, ATerm)
                   and prev_prev_prev_sb == Close
                   and prev_prev_sb == Sp
                   and prev_sb in (Sep, CR, LF))):
            do_break = True
        else:
            do_break = False
        for j in range(next_pos-pos):
            yield int(j==0 and do_break)
        prev_prev_prev_prev_sb = prev_prev_prev_sb
        prev_prev_prev_sb = prev_prev_sb
        prev_prev_sb = prev_sb
        prev_sb = sb
        pos = next_pos


def sentence_boundaries(s, tailor=None):
    
    """Iterate indices of the sentence boundaries of `s`
    
    This function yields from 0 to the end of the string (== len(s)).
    
    >>> list(sentence_boundaries(u'ABC'))
    [0, 3]
    >>> s = 'He said, \u201cAre you going?\u201d John shook his head.'
    >>> list(sentence_boundaries(s))
    [0, 26, 46]
    >>> list(sentence_boundaries(u''))
    []
    """
    
    breakables = sentence_breakables(s)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return boundaries(breakables)


def sentences(s, tailor=None):
    
    """Iterate every sentence of `s`
    
    >>> s = 'He said, \u201cAre you going?\u201d John shook his head.'
    >>> list(sentences(s)) == ['He said, \u201cAre you going?\u201d ', 'John shook his head.']
    True
    """
    
    breakables = sentence_breakables(s)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return list(break_units(s, breakables))


if __name__ == '__main__':
    import doctest
    doctest.testmod()
