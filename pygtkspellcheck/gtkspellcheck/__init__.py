# -*- coding:utf-8 -*-
#
# Copyright (C) 2012, Maximilian Köhl <linuxmaxi@googlemail.com>
# Copyright (C) 2012, Carlos Jenkins <carlos@jenkins.co.cr>
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

__version__ = '4.0.1'
__project__ = 'Python GTK Spellcheck'
__short_name__ = 'pygtkspellcheck'
__authors__ = 'Maximilian Köhl & Carlos Jenkins'
__emails__ = 'linuxmaxi@googlemail.com & carlos@jenkins.co.cr'
__website__ = 'http://koehlma.github.com/projects/pygtkspellcheck.html'
__download_url__ = 'https://github.com/koehlma/pygtkspellcheck/tarball/master'
__source__ = 'https://github.com/koehlma/pygtkspellcheck/'
__vcs__ = 'git://github.com/koehlma/pygtkspellcheck.git'
__copyright__ = '2012, Maximilian Köhl & Carlos Jenkins'
__desc_short__ = ('a simple but quite powerful Python spell checking library '
                  'for GtkTextViews based on Enchant')
__desc_long__ = ('A simple but quite powerful spellchecking library written in '
                 'pure Python for Gtk based on Enchant. It supports PyGObject '
                 'as well as PyGtk for Python 2 and 3 with automatic switching '
                 'and binding detection. For automatic translation of the user '
                 'interface it can use Gedit’s translation files.')

__metadata__ = {'__version__' : __version__,
                '__project__' : __project__,
                '__short_name__' : __short_name__,
                '__authors__' : __authors__,
                '__emails__' : __emails__,
                '__website__' : __website__,
                '__download_url__' : __download_url__,
                '__source__' : __source__,
                '__vcs__' : __vcs__,
                '__copyright__' : __copyright__,
                '__desc_short__' : __desc_short__,
                '__desc_long__' : __desc_long__}

from gtkspellcheck.spellcheck import (SpellChecker, NoDictionariesFound,
                                      NoGtkBindingFound)