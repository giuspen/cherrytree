# -*- coding:utf-8 -*-
#
# Copyright (C) 2012, Maximilian Köhl <linuxmaxi@googlemail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import unicode_literals

__version__ = '1.1'
__project__ = 'Python Locales'
__short_name__ = 'pylocales'
__authors__ = 'Maximilian Köhl'
__emails__ = 'linuxmaxi@googlemail.com'
__website__ = 'http://koehlma.github.com/projects/pygtkspellcheck.html'
__source__ = 'https://github.com/koehlma/pygtkspellcheck/'
__vcs__ = 'git://github.com/koehlma/pygtkspellcheck.git'
__copyright__ = '2012, Maximilian Köhl'
__desc_short__ = 'query the ISO 639/3166 database about a country or a language.'
__desc_long__ = ('Query the ISO 639/3166 database about a country or a'
                 'language. The locales database contains ISO 639 language'
                 'definitions and ISO 3166 country definitions. This package'
                 'provides translation for country and language names if'
                 'the iso-code messages are installed on your system.')

__metadata__ = {'__version__' : __version__,
                '__project__' : __project__,
                '__short_name__' : __short_name__,
                '__authors__' : __authors__,
                '__emails__' : __emails__,
                '__website__' : __website__,
                '__source__' : __source__,
                '__vcs__' : __vcs__,
                '__copyright__' : __copyright__,
                '__desc_short__' : __desc_short__,
                '__desc_long__' : __desc_long__}

from pylocales.locales import (Country, Language, LanguageNotFound,
                               CountryNotFound, code_to_name)