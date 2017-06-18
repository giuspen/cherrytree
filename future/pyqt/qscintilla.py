#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
#       qscintilla.py
#
#       Copyright 2017 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#       MA 02110-1301, USA.

import sys
#from PyQt5.QtCore import
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QApplication
import PyQt5.Qsci as Qsci


def get_hardcoded_lexers():
    lexers = [l[9:] for l in dir(Qsci) if l.startswith("QsciLexer")]
    lexers.remove("")
    lexers.remove("Custom")
    return lexers


class MyQScintilla(Qsci.QsciScintilla):

    def __init__(self, parent=None):
        super(MyQScintilla, self).__init__(parent)

        lexer = Qsci.QsciLexerPython()
        self.setLexer(lexer)
        self.setWhitespaceVisibility(Qsci.QsciScintilla.WsVisible)
        self.setMarginLineNumbers(1, True)
        self.setMarginWidth(1, 25)
        self.setReadOnly(False)
        self.setIndentationGuides(True)
        self.setIndentationWidth(4)
        self.setIndentationsUseTabs(False)
        self.setWrapMode(Qsci.QsciScintilla.WrapWord)
        self.setUtf8(True)
        self.setBraceMatching(Qsci.QsciScintilla.SloppyBraceMatch)
        self.setCaretLineVisible(True) # highlight current line
        self.setCaretLineBackgroundColor(QColor("#e5e5e5"))

    def setContent(self, content_text):
        self.setText(content_text)
        self.setModified(False)
        self.modificationChanged.connect(self.onModificationChanged)

    def onModificationChanged(self, modified_flag):
        print("modified", modified_flag)


if __name__ == "__main__":
    hardcoded_lexers = get_hardcoded_lexers()
    print(len(hardcoded_lexers), hardcoded_lexers)
    app = QApplication(sys.argv)
    editor = MyQScintilla()
    with open(__file__, 'r') as fd:
        myself_text = fd.read()
    editor.setContent(myself_text)
    editor.show()
    editor.ensureLineVisible(46)
    #editor.ensureCursorVisible()
    sys.exit(app.exec_())
