"""Unicode-aware text wrapping """

from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)
import re
from unicodedata import east_asian_width

from .codepoint         import ord, code_point, code_points
from .graphemecluster   import grapheme_clusters, grapheme_cluster_boundaries
from .linebreak         import line_break_boundaries


__all__ = [
    'Wrapper',
    'wrap',
    'Formatter',
    'TTFormatter', 
    'tt_width',
    'tt_text_extents',
    'tt_wrap'
]


class Wrapper(object):
    
    """ Text wrapping engine 

    Usually, you don't need to create an instance of the class directly.  Use 
    :func:`wrap` instead.
    """

    def wrap(self, formatter, s, cur=0, offset=0, char_wrap=None):
        
        """Wrap string `s` with `formatter` and invoke its handlers

        The optional arguments, `cur` is the starting position of the string 
        in logical length, and `offset` means left-side offset of the wrapping 
        area in logical length --- this parameter is only used for calculating 
        tab-stopping positions for now.

        If `char_wrap` is set to ``True``, the text will be warpped with its 
        grapheme cluster boundaries instead of its line break boundaries.
        This may be helpful when you don't want the word wrapping feature in 
        your application.

        This function returns the total count of wrapped lines.

        - *Changed in version 0.7:* The order of the parameters are changed.
        - *Changed in version 0.7.1:* It returns the count of lines now.
        """

        partial_extents = self._partial_extents
        if char_wrap:
            iter_boundaries = grapheme_cluster_boundaries
        else:
            iter_boundaries = line_break_boundaries
        
        iline = 0
        for para in s.splitlines(True):
            for field in re.split('(\\t)', para):
                if field == '\t':
                    tw = formatter.tab_width
                    field_extents = [tw - (offset + cur) % tw]
                else:
                    field_extents = formatter.text_extents(field)
                prev_boundary = 0
                prev_extent = 0
                breakpoint = 0
                for boundary in iter_boundaries(field):
                    extent = field_extents[boundary-1]
                    w = extent - prev_extent
                    wrap_width = formatter.wrap_width
                    if wrap_width is not None and cur + w > wrap_width:
                        line = field[breakpoint:prev_boundary]
                        line_extents = partial_extents(field_extents,
                                                       breakpoint,
                                                       prev_boundary)
                        formatter.handle_text(line, line_extents)
                        formatter.handle_new_line(); iline += 1
                        cur = 0
                        breakpoint = prev_boundary
                    cur += w
                    prev_boundary = boundary
                    prev_extent = extent
                line = field[breakpoint:]
                line_extents = partial_extents(field_extents, breakpoint)
                formatter.handle_text(line, line_extents)
            formatter.handle_new_line(); iline += 1
            cur = 0
        return iline
    
    @staticmethod
    def _partial_extents(extents, start, stop=None):

        """(internal) return partial extents of `extents[start:end]` """
        
        if stop is None:
            stop = len(extents)
        extent_offset = extents[start-1] if start > 0 else 0
        return [extents[x] - extent_offset for x in range(start, stop)]


### static objects
__wrapper__ = Wrapper()


def wrap(formatter, s, cur=0, offset=0, char_wrap=None):

    """Wrap string `s` with `formatter` using the module's static 
    :class:`Wrapper` instance

    See :meth:`Wrapper.wrap` for further details of the parameters.
    
    - *Changed in version 0.7.1:* It returns the count of lines now.
    """
    return __wrapper__.wrap(formatter, s, cur, offset, char_wrap)


class Formatter(object):
    
    """The abstruct base class for formatters invoked by a :class:`Wrapper` 
    object

    This class is implemented only for convinience sake and does nothing 
    itself.  You don't have to design your own formatter as a subclass of it, 
    while it is not deprecated either.

    **Your formatters should have the methods and properties this class has.**  
    They are invoked by a :class:`Wrapper` object to determin *logical widths* 
    of texts and to give you the ways to handle them, such as to render them.
    """

    @property
    def wrap_width(self):
        
        """The logical width of text wrapping 

        Note that returning ``None`` (which is the default) means *"do not 
        wrap"* while returning ``0`` means *"wrap as narrowly as possible."*
        """
        return None

    @property
    def tab_width(self):
        
        """The logical width of tab forwarding

        This property value is used by a :class:`Wrapper` object to determin 
        the actual forwarding extents of tabs in each of the positions.
        """
        return 0

    def reset(self):
        
        """Reset all states of the formatter """
        pass

    def text_extents(self, s):
        
        """Return a list of logical lengths from start of the string to 
        each of characters in `s`
        """
        pass
    
    def handle_text(self, text, extents):
        
        """The handler method which is invoked when `text` should be put 
        on the current position with `extents`
        """
        pass

    def handle_new_line(self):

        """The handler method which is invoked when the current line is 
        over and a new line begins
        """
        pass


### TT

class TTFormatter(object):
    
    """A Fixed-width text wrapping formatter """

    def __init__(self, wrap_width,
                 tab_width=8, tab_char=' ', ambiguous_as_wide=False):

        self._lines = ['']
        
        self.wrap_width         = wrap_width
        self.tab_width          = tab_width
        self.ambiguous_as_wide  = ambiguous_as_wide
        self.tab_char           = tab_char
    
    @property
    def wrap_width(self):
        """Wrapping width """
        return self._wrap_width

    @wrap_width.setter
    def wrap_width(self, value):
        self._wrap_width = value

    @property
    def tab_width(self):
        """forwarding size of tabs """
        return self._tab_width
    
    @tab_width.setter
    def tab_width(self, value):
        self._tab_width = value
    
    @property
    def tab_char(self):
        """Character to fill tab spaces with """
        return self._tab_char
    
    @tab_char.setter
    def tab_char(self, value):
        if (east_asian_width(value) not in ('N', 'Na', 'H')):
            raise ValueError("""only a narrow code point is available for 
                             tab_char""")
        self._tab_char = value

    @property
    def ambiguous_as_wide(self):
        """Treat code points with its East_Easian_Width property is 'A' as 
        those with 'W'; having double width as alpha-numerics
        """
        return self._ambiguous_as_wide

    @ambiguous_as_wide.setter
    def ambiguous_as_wide(self, value):
        self._ambiguous_as_wide = value

    def reset(self):
        
        """Reset all states of the formatter
        """
        del self._lines[:]

    def text_extents(self, s):
        
        """Return a list of logical lengths from start of the string to 
        each of characters in `s`
        """
        return tt_text_extents(s, self.ambiguous_as_wide)

    def handle_text(self, text, extents):
        
        """The handler which is invoked when a text should be put on the 
        current position
        """
        if text == '\t':
            text = self.tab_char * extents[0]
        self._lines[-1] += text

    def handle_new_line(self):

        """The handler which is invoked when the current line is over and a 
        new line begins
        """
        self._lines.append('')
    
    def lines(self):

        """Iterate every wrapped line strings
        """
        if not self._lines[-1]:
            self._lines.pop()
        return iter(self._lines)


def tt_width(s, index=0, ambiguous_as_wide=False):
    
    """Return logical width of the grapheme cluster at `s[index]` on 
    fixed-width typography
    
    Return value will be ``1`` (halfwidth) or ``2`` (fullwidth).
    
    Generally, the width of a grapheme cluster is determined by its leading 
    code point.
    
    >>> tt_width('A')
    1
    >>> tt_width('\\u8240')     # U+8240: CJK UNIFIED IDEOGRAPH-8240
    2
    >>> tt_width('g\\u0308')    # U+0308: COMBINING DIAERESIS
    1
    >>> tt_width('\\U00029e3d') # U+29E3D: CJK UNIFIED IDEOGRAPH-29E3D
    2
    
    If `ambiguous_as_wide` is specified to ``True``, some characters such as 
    greek alphabets are treated as they have fullwidth as well as ideographics 
    does.
    
    >>> tt_width('\\u03b1')     # U+03B1: GREEK SMALL LETTER ALPHA
    1
    >>> tt_width('\\u03b1', ambiguous_as_wide=True)
    2
    """
    cp = code_point(s, index)
    eaw = east_asian_width(cp)
    if eaw in ('W', 'F') or (eaw == 'A' and ambiguous_as_wide):
        return 2
    return 1


def tt_text_extents(s, ambiguous_as_wide=False):
    
    """Return a list of logical widths from the start of `s` to each of 
    characters *(not of code points)* on fixed-width typography
    
    >>> tt_text_extents('')
    []
    >>> tt_text_extents('abc')
    [1, 2, 3]
    >>> tt_text_extents('\\u3042\\u3044\\u3046')
    [2, 4, 6]
    >>> import sys
    >>> s = '\\U00029e3d'   # test a code point out of BMP
    >>> actual = tt_text_extents(s)
    >>> expect = [2] if sys.maxunicode > 0xffff else [2, 2]
    >>> len(s) == len(expect)
    True
    >>> actual == expect
    True
    
    The meaning of `ambiguous_as_wide` is the same as that of 
    :func:`tt_width`.
    """
    widths = []
    total_width = 0
    for gc in grapheme_clusters(s):
        total_width += tt_width(gc, ambiguous_as_wide)
        widths.extend(total_width for __ in gc)
    return widths


def tt_wrap(s, wrap_width, tab_width=8, tab_char=' ', ambiguous_as_wide=False,
            cur=0, offset=0, char_wrap=False):
    """Wrap `s` with given parameters and return a list of wrapped lines

    See :class:`TTFormatter` for `wrap_width`, `tab_width` and `tab_char`, and 
    :func:`tt_wrap` for `cur`, `offset` and `char_wrap`.
    """
    formatter = TTFormatter(wrap_width, tab_width, tab_char,
                            ambiguous_as_wide)
    __wrapper__.wrap(formatter, s, cur, offset, char_wrap)
    return formatter.lines()


### Main

if __name__ == '__main__':
    import doctest
    doctest.testmod()
