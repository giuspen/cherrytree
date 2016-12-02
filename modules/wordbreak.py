# encoding: utf-8

# These modules are extracted from the uniseg package (https://bitbucket.org/emptypage/uniseg-python).
# The "entry" point used by cherrypy is words()

import re
import sys
from wordbreak_data import wordbreak_data

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


if sys.maxunicode < 0x10000:
    # narrow unicode build

    def ord_impl(c, index):

        if isinstance(c, str):
            return ord(c[index or 0])
        if not isinstance(c, unicode):
            raise TypeError('must be unicode, not %s' % type(c).__name__)
        i = index or 0
        len_s = len(c) - i
        if len_s:
            value = hi = ord(c[i])
            i += 1
            if 0xd800 <= hi < 0xdc00:
                if len_s > 1:
                    lo = ord(c[i])
                    i += 1
                    if 0xdc00 <= lo < 0xe000:
                        value = (hi - 0xd800) * 0x400 + (lo - 0xdc00) + 0x10000
            if index is not None or i == len_s:
                return value
        raise TypeError('need a single Unicode code point as parameter')


    def unichr_impl(cp):

        if not isinstance(cp, int):
            raise TypeError('must be int, not %s' % type(cp).__name__)
        if cp < 0x10000:
            return chr(cp)
        hi, lo = divmod(cp - 0x10000, 0x400)
        hi += 0xd800
        lo += 0xdc00
        if 0xd800 <= hi < 0xdc00 and 0xdc00 <= lo < 0xe000:
            return chr(hi) + chr(lo)
        raise ValueError('illeagal code point')


    rx_codepoints = re.compile(r'[\ud800-\udbff][\udc00-\udfff]|.', re.DOTALL)


    def code_point_impl(s, index):

        L = rx_codepoints.findall(s)
        return L[index]


    def code_points_impl(s):

        return rx_codepoints.findall(s)

else:
    # wide unicode build

    def ord_impl(c, index):
        return ord(c if index is None else c[index])


    def unichr_impl(cp):
        return chr(cp)


    def code_point_impl(s, index):
        return s[index or 0]


    def code_points_impl(s):
        return list(s)


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
        next_pos, next_wb = (primitive_boundaries[i + 1]
                             if i < len(primitive_boundaries) - 1 else (len(s), None))
        # print pos, prev_wb, wb
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
        for j in range(next_pos - pos):
            yield int(j == 0 and do_break)
        prev_pos = pos
        prev_prev_wb = prev_wb
        prev_wb = wb


def break_units(s, breakables):
    """Iterate every tokens of `s` basing on breakable table, `breakables`

    >>> list(break_units('ABC', [1, 1, 1])) == ['A', 'B', 'C']
    True
    >>> list(break_units('ABC', [1, 0, 1])) == ['AB', 'C']
    True
    >>> list(break_units('ABC', [1, 0, 0])) == ['ABC']
    True

    The length of `s` must be equal to that of `breakables`.
    """

    i = 0
    for j, bk in enumerate(breakables):
        if bk:
            if j:
                yield s[i:j]
            i = j
    if s:
        yield s[i:]


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


def code_points(s):
    """Iterate every Unicode code points of the unicode string `s`

    >>> s = 'hello'
    >>> list(code_points(s)) == ['h', 'e', 'l', 'l', 'o']
    True

    The number of iteration may differ from the ``len(s)``, because some
    code points may be represented as a couple of other code points
    ("surrogate pair") in narrow-build Python.

    >>> s = 'abc\\U00020b9f\\u3042'
    >>> list(code_points(s)) == ['a', 'b', 'c', '\\U00020b9f', '\\u3042']
    True
    """

    return code_points_impl(s)


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


def _word_break(u):
    value = wordbreak_data.get(ord(u))
    return value if value is not None else 'Other'

def code_point(s, index=0):
    """Return code point at s[index]

    >>> code_point('ABC') == 'A'
    True
    >>> code_point('ABC', 1) == 'B'
    True
    >>> code_point('\\U00020b9f\\u3042') == '\\U00020b9f'
    True
    >>> code_point('\\U00020b9f\u3042', 1) == '\\u3042'
    True
    """

    return code_point_impl(s, index)