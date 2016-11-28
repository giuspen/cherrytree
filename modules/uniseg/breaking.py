"""Breakable table and string tokenization """


from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)


__all__ = [
    'boundaries',
    'break_units',
]


def boundaries(breakables):
    
    """Iterate boundary indices of the breakabe table, `breakables`
    
    The boundaries start from 0 to the end of the sequence (== 
    len(breakables)).
    
    >>> list(boundaries([1, 1, 1]))
    [0, 1, 2, 3]
    >>> list(boundaries([1, 0, 1]))
    [0, 2, 3]
    >>> list(boundaries([0, 1, 0]))
    [1, 3]
    
    It yields empty when the given sequece is empty.
    
    >>> list(boundaries([]))
    []
    """
    
    i = None
    for i, breakable in enumerate(breakables):
        if breakable:
            yield i
    if i is not None:
        yield i+1


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


if __name__ == '__main__':
    import doctest
    doctest.testmod()
