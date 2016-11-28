"""Unicode line breaking algorithm

UAX #14: Unicode Line Breaking Algorithm
    http://www.unicode.org/reports/tr14/tr14-24.html
"""


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)
from unicodedata import east_asian_width

from .breaking import boundaries, break_units
from .codepoint import ord, unichr, code_point, code_points
from .db import line_break as _line_break


__all__ = [
    'line_break',
    'line_break_breakables',
    'line_break_boundaries',
    'line_break_units',
]

BK = 'BK'   # Mandatory Break
CR = 'CR'   # Carriage Return
LF = 'LF'   # Line Feed
CM = 'CM'   # Combining Mark
NL = 'NL'   # Next Line
SG = 'SG'   # Surrogate
WJ = 'WJ'   # Word Joiner
ZW = 'ZW'   # Zero Width Space
GL = 'GL'   # Non-breaking ("Glue")
SP = 'SP'   # Space
B2 = 'B2'   # Break Opportunity Before and After
BA = 'BA'   # Break After
BB = 'BB'   # Break Before
HY = 'HY'   # Hyphen
CB = 'CB'   # Contingent Break Opportunity
CL = 'CL'   # Close Punctuation
CP = 'CP'   # Close Parenthesis
EX = 'EX'   # Exclamation/Interrogation
IN = 'IN'   # Inseparable
NS = 'NS'   # Nonstarter
OP = 'OP'   # Open Punctuation
QU = 'QU'   # Quotation
IS = 'IS'   # Infix Numeric Separator
NU = 'NU'   # Numeric
PO = 'PO'   # Postfix Numeric
PR = 'PR'   # Prefix Numeric
SY = 'SY'   # Symbols Allowing Break After
AI = 'AI'   # Ambiguous (Alphabetic or Ideographic)
AL = 'AL'   # Alphabetic
CJ = 'CJ'   # Conditional Japanese Starter
H2 = 'H2'   # Hangul LV Syllable
H3 = 'H3'   # Hangul LVT Syllable
HL = 'HL'   # Hebrew Letter
ID = 'ID'   # Ideographic
JL = 'JL'   # Hangul L Jamo
JV = 'JV'   # Hangul V Jamo
JT = 'JT'   # Hangul T Jamo
RI = 'RI'   # Regional Indicator
SA = 'SA'   # Complex Context Dependent (South East Asian)
XX = 'XX'   # Unknown

# Pair table based on UAX #14.
# cf. http://www.unicode.org/reports/tr14/#ExampleTable
# The table is extended to handle CBs.
pair_table = {
OP: {OP: '^', CL: '^', CP: '^', QU: '^', GL: '^', NS: '^',
     EX: '^', SY: '^', IS: '^', PR: '^', PO: '^', NU: '^',
     AL: '^', ID: '^', IN: '^', HY: '^', BA: '^', BB: '^',
     B2: '^', ZW: '^', CM: '@', WJ: '^', H2: '^', H3: '^',
     JL: '^', JV: '^', JT: '^', CB: '^'},
CL: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '^',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
CP: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '^',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '%',
     AL: '%', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
QU: {OP: '^', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '%',
     AL: '%', ID: '%', IN: '%', HY: '%', BA: '%', BB: '%',
     B2: '%', ZW: '^', CM: '#', WJ: '^', H2: '%', H3: '%',
     JL: '%', JV: '%', JT: '%', CB: '%'},
GL: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '%',
     AL: '%', ID: '%', IN: '%', HY: '%', BA: '%', BB: '%',
     B2: '%', ZW: '^', CM: '#', WJ: '^', H2: '%', H3: '%',
     JL: '%', JV: '%', JT: '%', CB: '%'},
NS: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
EX: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
SY: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
IS: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '%', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
PR: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '%', ID: '%', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '%', H3: '%',
     JL: '%', JV: '%', JT: '%', CB: '_'},
PO: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '%', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
NU: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '%',
     AL: '%', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
AL: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '%', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
ID: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
IN: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
HY: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '_', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
BA: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '_', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
BB: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '%',
     AL: '%', ID: '%', IN: '%', HY: '%', BA: '%', BB: '%',
     B2: '%', ZW: '^', CM: '#', WJ: '^', H2: '%', H3: '%',
     JL: '%', JV: '%', JT: '%', CB: '_'},
B2: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '%', BA: '%', BB: '_',
     B2: '^', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
ZW: {OP: '_', CL: '_', CP: '_', QU: '_', GL: '_', NS: '_',
     EX: '_', SY: '_', IS: '_', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '_', BA: '_', BB: '_',
     B2: '_', ZW: '^', CM: '_', WJ: '_', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
CM: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '%',
     AL: '%', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
WJ: {OP: '%', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '%', PO: '%', NU: '%',
     AL: '%', ID: '%', IN: '%', HY: '%', BA: '%', BB: '%',
     B2: '%', ZW: '^', CM: '#', WJ: '^', H2: '%', H3: '%',
     JL: '%', JV: '%', JT: '%', CB: '%'},
H2: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '%', JT: '%', CB: '_'},
H3: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '%', CB: '_'},
JL: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '%', H3: '%',
     JL: '%', JV: '%', JT: '_', CB: '_'},
JV: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '%', JT: '%', CB: '_'},
JT: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '%',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '%', NU: '_',
     AL: '_', ID: '_', IN: '%', HY: '%', BA: '%', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '%', CB: '_'},
CB: {OP: '_', CL: '^', CP: '^', QU: '%', GL: '%', NS: '_',
     EX: '^', SY: '^', IS: '^', PR: '_', PO: '_', NU: '_',
     AL: '_', ID: '_', IN: '_', HY: '_', BA: '_', BB: '_',
     B2: '_', ZW: '^', CM: '#', WJ: '^', H2: '_', H3: '_',
     JL: '_', JV: '_', JT: '_', CB: '_'},
}


def line_break(c, index=0):
    
    r"""Return the Line_Break property of `c`
    
    `c` must be a single Unicode code point string.
    
    >>> print(line_break('\x0d'))
    CR
    >>> print(line_break(' '))
    SP
    >>> print(line_break('1'))
    NU
    
    If `index` is specified, this function consider `c` as a unicode 
    string and return Line_Break property of the code point at 
    c[index].
    
    >>> print(line_break(u'a\x0d', 1))
    CR
    """
    
    return _line_break(code_point(c, index))


def _preprocess_boundaries(s):
    
    r"""(internal) Preprocess LB9: X CM* -> X
    
    Where X is not in (BK, CR, LF, NL, SP, ZW)
    
    >>> list(_preprocess_boundaries(u'\r\n')) == [(0, 'CR'), (1, 'LF')]
    True
    >>> list(_preprocess_boundaries(u'A\x01A')) == [(0, 'AL'), (2, 'AL')]
    True
    >>> list(_preprocess_boundaries(u'\n\x01')) == [(0, 'LF'), (1, 'CM')]
    True
    >>> list(_preprocess_boundaries(u'\n  A')) == [(0, 'LF'), (1, 'SP'), (2, 'SP'), (3, 'AL')]
    True
    """
    
    prev_prop = None
    i = 0
    for c in code_points(s):
        prop = line_break(c)
        if prop in (BK, CR, LF, SP, NL, ZW):
            yield (i, prop)
            prev_prop = None
        elif prop == CM:
            if prev_prop is None:
                yield (i, prop)
                prev_prop = prop
        else:
            yield (i, prop)
            prev_prop = prop
        i += len(c)


def line_break_breakables(s, legacy=False):
    
    """Iterate line breaking opportunities for every position of `s`
    
    1 means "break" and 0 means "do not break" BEFORE the postion.  
    The length of iteration will be the same as ``len(s)``.
    
    >>> list(line_break_breakables('ABC'))
    [0, 0, 0]
    >>> list(line_break_breakables('Hello, world.'))
    [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0]
    >>> list(line_break_breakables(u''))
    []
    """
    
    if not s:
        return
    
    primitive_boundaries = list(_preprocess_boundaries(s))
    prev_prev_lb = None
    prev_lb = None
    for i, (pos, lb) in enumerate(primitive_boundaries):
        next_pos, __ = (primitive_boundaries[i+1]
                        if i<len(primitive_boundaries)-1 else (len(s), None))
        
        if legacy:
            if lb == AL:
                cp = unichr(ord(s, pos))
                lb = ID if east_asian_width(cp) == 'A' else AL
            elif lb == AI:
                lb = ID
        else:
            if lb == AI:
                lb = AL
        
        if lb == CJ:
            lb = NS

        if lb in (CM, XX, SA):
            lb = AL
        # LB4
        if pos == 0:
            do_break = False
        elif prev_lb == BK:
            do_break = True
        # LB5
        elif prev_lb in (CR, LF, NL):
            do_break = not (prev_lb == CR and lb == LF)
        # LB6
        elif lb in (BK, CR, LF, NL):
            do_break = False
        # LB7
        elif lb in (SP, ZW):
            do_break = False
        # LB8
        elif ((prev_prev_lb == ZW and prev_lb == SP) or (prev_lb == ZW)):
            do_break = True
        # LB11
        elif lb == WJ or prev_lb == WJ:
            do_break = False
        # LB12
        elif prev_lb == GL:
            do_break = False
        # LB12a
        elif prev_lb not in (SP, BA, HY) and lb == GL:
            do_break = False
        # LB13
        elif lb in (CL, CP, EX, IS, SY):
            do_break = False
        # LB14
        elif (prev_prev_lb == OP and prev_lb == SP) or prev_lb == OP:
            do_break = False
        # LB15
        elif ((prev_prev_lb == QU and prev_lb == SP and lb == OP)
              or (prev_lb == QU and lb == OP)):
            do_break = False
        # LB16
        elif ((prev_prev_lb in (CL, CP) and prev_lb == SP and lb == NS)
              or (prev_lb in (CL, CP) and lb == NS)):
            do_break = False
        # LB17
        elif ((prev_prev_lb == B2 and prev_lb == SP and lb == B2)
              or (prev_lb == B2 and lb == B2)):
            do_break = False
        # LB18
        elif prev_lb == SP:
            do_break = True
        # LB19
        elif lb == QU or prev_lb == QU:
            do_break = False
        # LB20
        elif lb == CB or prev_lb == CB:
            do_break = True
        # LB21
        elif lb in (BA, HY, NS) or prev_lb == BB:
            do_break = False
        # LB22
        elif prev_lb in (AL, HL, ID, IN, NU) and lb == IN:
            do_break = False
        # LB23
        elif ((prev_lb == ID and lb == PO)
              or (prev_lb in (AL, HL) and lb == NU)
              or (prev_lb == NU and lb in (AL, HL))):
            do_break = False
        # LB24
        elif ((prev_lb == PR and lb == ID)
              or (prev_lb == PR and lb in (AL, HL))
              or (prev_lb == PO and lb in (AL, HL))):
            do_break = False
        # LB25
        elif ((prev_lb == CL and lb == PO)
              or (prev_lb == CP and lb == PO)
              or (prev_lb == CL and lb == PR)
              or (prev_lb == CP and lb == PR)
              or (prev_lb == NU and lb == PO)
              or (prev_lb == NU and lb == PR)
              or (prev_lb == PO and lb == OP)
              or (prev_lb == PO and lb == NU)
              or (prev_lb == PR and lb == OP)
              or (prev_lb == PR and lb == NU)
              or (prev_lb == HY and lb == NU)
              or (prev_lb == IS and lb == NU)
              or (prev_lb == NU and lb == NU)
              or (prev_lb == SY and lb == NU)):
            do_break = False
        # LB26
        elif ((prev_lb == JL and lb in (JL, JV, H2, H3))
              or (prev_lb in (JV, H2) and lb in (JV, JT))
              or (prev_lb in (JT, H3) and lb == JT)):
            do_break = False
        # LB27
        elif ((prev_lb in (JL, JV, JT, H2, H3) and lb in (IN, PO))
              or (prev_lb == PR and lb in (JL, JV, JT, H2, H3))):
            do_break = False
        # LB28
        elif prev_lb in (AL, HL) and lb in (AL, HL):
            do_break = False
        # LB29
        elif prev_lb == IS and lb in (AL, HL):
            do_break = False
        # LB30
        elif ((prev_lb in (AL, HL, NU) and lb == OP)
              or (prev_lb == CP and lb in (AL, HL, NU))):
            do_break = False
        # LB30a
        elif prev_lb == lb == RI:
            do_break = False
        else:
            do_break = True
        for j in range(next_pos-pos):
            yield int(j==0 and do_break)
        prev_prev_lb = prev_lb
        prev_lb = lb


def line_break_boundaries(s, legacy=False, tailor=None):
    
    """Iterate indices of the line breaking boundaries of `s`
    
    This function yields from 0 to the end of the string (== len(s)).
    """
    
    breakables = line_break_breakables(s, legacy)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return boundaries(breakables)


def line_break_units(s, legacy=False, tailor=None):
    
    r"""Iterate every line breaking token of `s`
    
    >>> s = 'The quick (\u201cbrown\u201d) fox can\u2019t jump 32.3 feet, right?'
    >>> '|'.join(line_break_units(s)) == 'The |quick |(\u201cbrown\u201d) |fox |can\u2019t |jump |32.3 |feet, |right?'
    True
    >>> list(line_break_units(u''))
    []
    
    >>> list(line_break_units('\u03b1\u03b1')) == [u'\u03b1\u03b1']
    True
    >>> list(line_break_units(u'\u03b1\u03b1', True)) == [u'\u03b1', u'\u03b1']
    True
    """
    
    breakables = line_break_breakables(s, legacy)
    if tailor is not None:
        breakables = tailor(s, breakables)
    return break_units(s, breakables)


if __name__ == '__main__':
    import doctest
    doctest.testmod()
