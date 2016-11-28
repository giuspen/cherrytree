"""Unicode code point """


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)
import re
import sys
if sys.version_info >= (3, 0):
    from builtins import ord as _ord, chr as _chr
else:
    from __builtin__ import ord as _ord, unichr as _chr


__all__ = [
    'ord',
    'unichr',
    'code_points'
]


if sys.maxunicode < 0x10000:
    # narrow unicode build
    
    def ord_impl(c, index):
        
        if isinstance(c, str):
            return _ord(c[index or 0])
        if not isinstance(c, unicode):
            raise TypeError('must be unicode, not %s' % type(c).__name__)
        i = index or 0
        len_s = len(c)-i
        if len_s:
            value = hi = _ord(c[i])
            i += 1
            if 0xd800 <= hi < 0xdc00:
                if len_s > 1:
                    lo = _ord(c[i])
                    i += 1
                    if 0xdc00 <= lo < 0xe000:
                        value = (hi-0xd800)*0x400+(lo-0xdc00)+0x10000
            if index is not None or i == len_s:
                return value
        raise TypeError('need a single Unicode code point as parameter')
    
    def unichr_impl(cp):
        
        if not isinstance(cp, int):
            raise TypeError('must be int, not %s' % type(c).__name__)
        if cp < 0x10000:
            return _chr(cp)
        hi, lo = divmod(cp-0x10000, 0x400)
        hi += 0xd800
        lo += 0xdc00
        if 0xd800 <= hi < 0xdc00 and 0xdc00 <= lo < 0xe000:
            return _chr(hi)+_chr(lo)
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
        return _ord(c if index is None else c[index])
    
    def unichr_impl(cp):
        return _chr(cp)
    
    def code_point_impl(s, index):
        return s[index or 0]
    
    def code_points_impl(s):
        return list(s)


def ord(c, index=None):
    
    """Return the integer value of the Unicode code point `c`
    
    NOTE: Some Unicode code points may be expressed with a couple of 
    other code points ("surrogate pair").  This function treats 
    surrogate pairs as representations of original code points; e.g. 
    ``ord(u'\\ud842\\udf9f')`` returns ``134047`` (``0x20b9f``). 
    ``u'\\ud842\\udf9f'`` is a surrogate pair expression which means 
    ``u'\\U00020b9f'``.
    
    >>> ord('a')
    97
    >>> ord('\\u3042')
    12354
    >>> ord('\\U00020b9f')
    134047
    >>> ord('abc')
    Traceback (most recent call last):
      ...
    TypeError: need a single Unicode code point as parameter
    
    It returns the result of built-in ord() when `c` is a single str 
    object for compatibility:
    
    >>> ord('a')
    97
    
    When `index` argument is specified (to not ``None``), this function 
    treats `c` as a Unicode string and returns integer value of code 
    point at ``c[index]`` (or may be ``c[index:index+2]``):
    
    >>> ord('hello', 0)
    104
    >>> ord('hello', 1)
    101
    >>> ord('a\\U00020b9f', 1)
    134047
    """
    
    return ord_impl(c, index)


def unichr(cp):
    
    """Return the unicode object represents the code point integer `cp`
    
    >>> unichr(0x61) == 'a'
    True
    
    Notice that some Unicode code points may be expressed with a 
    couple of other code points ("surrogate pair") in narrow-build 
    Python.  In those cases, this function will return a unicode 
    object of which length is more than one; e.g. ``unichr(0x20b9f)`` 
    returns ``u'\\U00020b9f'`` while built-in ``unichr()`` may raise 
    ValueError.
    
    >>> unichr(0x20b9f) == '\\U00020b9f'
    True
    """
    
    return unichr_impl(cp)


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


if __name__ == '__main__':
    import doctest
    doctest.testmod(optionflags=doctest.IGNORE_EXCEPTION_DETAIL)
