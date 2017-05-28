#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
#import PyQt5.QtCore as QtCore
import PyQt5.QtGui as QtGui
import PyQt5.QtWidgets as QtWidgets
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
        self.setCaretLineBackgroundColor(QtGui.QColor("#e5e5e5"))

    def setContent(self, content_text):
        self.setText(content_text)
        self.setModified(False)
        self.modificationChanged.connect(self.onModificationChanged)

    def onModificationChanged(self, modified_flag):
        print("modified", modified_flag)


if __name__ == "__main__":
    print(get_hardcoded_lexers())
    app = QtWidgets.QApplication(sys.argv)
    editor = MyQScintilla()
    with open(__file__, 'r') as fd:
        myself_text = fd.read()
    editor.setContent(myself_text)
    editor.show()
    editor.ensureLineVisible(46)
    #editor.ensureCursorVisible()
    sys.exit(app.exec_())
