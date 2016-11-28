# encoding: utf-8
"""Unicode word breaking

UAX #29: Unicode Text Segmentation (Unicode 6.2.0)
http://www.unicode.org/reports/tr29/tr29-21.html
"""


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)

from .breaking import boundaries, break_units
from .codepoint import code_point, code_points
from .db import word_break as _word_break


__all__ = [
    'word_break',
    'word_breakables',
    'word_boundaries',
    'words',
]


Other = 'Other'
CR = 'CR'
LF = 'LF'
Newline = 'Newline'
Extend = 'Extend'
Regional_Indicator = 'Regional_Indicator'
Format = 'Format'
Katakana = 'Katakana'
ALetter = 'ALetter'
MidNumLet = 'MidNumLet'
MidLetter = 'MidLetter'
MidNum = 'MidNum'
Numeric = 'Numeric'
ExtendNumLet = 'ExtendNumLet'

ALetter_FormatFE = 'ALetter_FormatFE'
ALetter_MidLetter = 'ALetter_MidLetter'
ALetter_MidNumLet = 'ALetter_MidNumLet'
ALetter_MidNumLet_FormatFE = 'ALetter_MidNumLet_FormatFE'
ALetter_MidNum = 'ALetter_MidNum'
Numeric_MidLetter = 'Numeric_MidLetter'
Numeric_MidNumLet = 'Numeric_MidNumLet'
Numeric_MidNum = 'Numeric_MidNum'
Numeric_MidNumLet_FormatFE = 'Numeric_MidNumLet_FormatFE'

break_table_index = [
    Other,
    CR,
    LF, 
    Newline,
    Katakana,
    ALetter,
    MidLetter,
    MidNum,
    MidNumLet,
    Numeric,
    ExtendNumLet,
    Regional_Indicator,
    Format,
    Extend,
    ALetter_FormatFE,
    ALetter_MidLetter,
    ALetter_MidNumLet,
    ALetter_MidNumLet_FormatFE,
    ALetter_MidNum,
    Numeric_MidLetter,
    Numeric_MidNumLet,
    Numeric_MidNum,
    Numeric_MidNumLet_FormatFE,
]

# cf. http://www.unicode.org/Public/6.2.0/ucd/auxiliary/WordBreakTest.html
break_table = [
#### 0  1  2  3  4  5  6  7  8  9 10 11 12 13 ###
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 0 Other
    [1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], # 1 CR
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], # 2 LF
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], # 3 Newline
    [1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0], # 4 Katakana
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0], # 5 ALetter
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 6 MidLetter
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 7 MidNum
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 8 MidNumLet
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0], # 9 Numeric
    [1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0], # 10 ExtendNumLet
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0], # 11 Regional_Indicator
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 12 Format_FE
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 13 Extend_FE
    # ========================================= #
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0], # 14 ALetter Format_FE
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0], # 15 ALetter MidLetter
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0], # 16 ALetter MidNumLet
    [1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0], # 17 ALetter MidNumLet Format_FE
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 18 ALetter MidNum
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0], # 19 Numeric MidLetter
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0], # 20 Numeric MidNumLet
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0], # 21 Numeric MidNum
    [1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0], # 22 Numeric MidNumLet Format_FE
]


def word_break(c, index=0):
    
    r"""Return the Word_Break property of `c`
    
    `c` must be a single Unicode code point string.
    
    >>> print(word_break('\x0d'))
    CR
    >>> print(word_break('\x0b'))
    Newline
    >>> print(word_break('\u30a2'))
    Katakana
    
    If `index` is specified, this function consider `c` as a unicode 
    string and return Word_Break property of the code point at 
    c[index].
    
    >>> print(word_break('A\u30a2', 1))
    Katakana
    """
    
    return _word_break(code_point(c, index))


def _preprocess_boundaries(s):
    
    r"""(internal) Preprocess WB4; X [Extend Format]* -> X
    
    >>> result = list(_preprocess_boundaries('\r\n'))
    >>> result == [(0, 'CR'), (1, 'LF')]
    True
    >>> result = list(_preprocess_boundaries('A\u0308A'))
    >>> result == [(0, 'ALetter'), (2, 'ALetter')]
    True
    >>> result = list(_preprocess_boundaries('\n\u2060'))
    >>> result == [(0, 'LF'), (1, 'Format')]
    True
    >>> result = list(_preprocess_boundaries('\x01\u0308\x01'))
    >>> result == [(0, 'Other'), (2, 'Other')]
    True
    """
    
    prev_prop = None
    i = 0
    for c in code_points(s):
        prop = word_break(c)
        if prop in (Newline, CR, LF):
            yield (i, prop)
            prev_prop = None
        elif prop in (Extend, Format):
            if prev_prop is None:
                yield (i, prop)
                prev_prop = prop
        else:
            yield (i, prop)
            prev_prop = prop
        i += len(c)


def word_breakables(s):
    
    r"""Iterate word breaking opportunities for every position of `s`
    
    1 for "break" and 0 for "do not break".  The length of iteration 
    will be the same as ``len(s)``.
    
    >>> list(word_breakables(u'ABC'))
    [1, 0, 0]
    >>> list(word_breakables(u'Hello, world.'))
    [1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1]
    >>> list(word_breakables(u'\x01\u0308\x01'))
    [1, 0, 1]
    """
    
    if not s:
        return
    
    primitive_boundaries = list(_preprocess_boundaries(s))
    prev_prev_wb = None
    prev_wb = None
    prev_pos = 0
    for i, (pos, wb) in enumerate(primitive_boundaries):
        next_pos, next_wb = (primitive_boundaries[i+1]
                             if i<len(primitive_boundaries)-1 else (len(s), None))
        #print pos, prev_wb, wb
        if prev_wb in (Newline, CR, LF) or wb in (Newline, CR, LF):
            do_break = not (prev_wb == CR and wb == LF)
        # WB5.
        elif prev_wb == wb == ALetter:
            do_break = False
        # WB6.
        elif (prev_wb == next_wb == ALetter
              and wb in (MidLetter, MidNumLet)):
            do_break = False
        # WB7.
        elif (prev_prev_wb == wb == ALetter
              and prev_wb in (MidLetter, MidNumLet)):
            do_break = False
        # WB8.
        elif prev_wb == wb == Numeric:
            do_break = False
        # WB9.
        elif prev_wb == ALetter and wb == Numeric:
            do_break = False
        # WB10.
        elif prev_wb == Numeric and wb == ALetter:
            do_break = False
        # WB11.
        elif (prev_prev_wb == wb == Numeric
              and prev_wb in (MidNum, MidNumLet)):
            do_break = False
        # WB12.
        elif (prev_wb == next_wb == Numeric
              and wb in (MidNum, MidNumLet)):
            do_break = False
        # WB13. WB13a. WB13b.
        elif (prev_wb == wb == Katakana
              or (prev_wb in (ALetter, Numeric, Katakana, ExtendNumLet)
                  and wb == ExtendNumLet)
              or (prev_wb == ExtendNumLet
                  and wb in ((ALetter, Numeric, Katakana)))
              ):
            do_break = False
        # WB13c.
        elif prev_wb == wb == Regional_Indicator:
            do_break = False
        # WB14.
        else:
            do_break = True
        for j in range(next_pos-pos):
            yield int(j==0 and do_break)
        prev_pos = pos
        prev_prev_wb = prev_wb
        prev_wb = wb


def word_boundaries(s, tailor=None):
    
    """Iterate indices of the word boundaries of `s`
    
    This function yields indices from the first boundary position (> 0) 
    to the end of the string (== len(s)).
    """
    
    breakables = word_breakables(s)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return boundaries(breakables)


def words(s, tailor=None):
    
    """Iterate *user-perceived* words of `s`
    
    These examples bellow is from
    http://www.unicode.org/reports/tr29/tr29-15.html#Word_Boundaries
    
    >>> s = 'The quick (“brown”) fox can’t jump 32.3 feet, right?'
    >>> print('|'.join(words(s)))
    The| |quick| |(|“|brown|”|)| |fox| |can’t| |jump| |32.3| |feet|,| |right|?
    >>> list(words(u''))
    []
    """
    
    breakables = word_breakables(s)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return break_units(s, breakables)


if __name__ == '__main__':
    import doctest
    doctest.testmod()
