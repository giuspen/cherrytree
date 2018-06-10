/*
 * qscintilla.cxx
 * 
 * Copyright 2017-2018 Giuseppe Penone <giuspen@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <QtWidgets>
#include <QtCore>
#include <QtGui>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>


class MyQScintilla : public QsciScintilla
{
public:
    MyQScintilla(QsciScintilla *parent = 0);
    ~MyQScintilla();
    void setContent(const QString content_text);

protected:
    QsciLexerCPP *mp_lexerCpp;
    void onModificationChanged(bool m);
};


MyQScintilla::MyQScintilla(QsciScintilla *parent) : QsciScintilla(parent)
{
    mp_lexerCpp = new QsciLexerCPP();
    setLexer(mp_lexerCpp);
    setWhitespaceVisibility(WsVisible);
    setMarginLineNumbers(1, true);
    setMarginWidth(1, 25);
    setReadOnly(false);
    setIndentationGuides(true);
    setIndentationWidth(4);
    setIndentationsUseTabs(false);
    setWrapMode(WrapWord);
    setUtf8(true);
    setBraceMatching(SloppyBraceMatch);
    setCaretLineVisible(true); // highlight current line
    setCaretLineBackgroundColor(QColor("#e5e5e5"));
}

MyQScintilla::~MyQScintilla()
{
    delete mp_lexerCpp;
}

void MyQScintilla::setContent(const QString content_text)
{
    setText(content_text);
    setModified(false);
    connect(this, &MyQScintilla::modificationChanged, this, &MyQScintilla::onModificationChanged);
}

void MyQScintilla::onModificationChanged(bool m)
{
    qInfo() << "modified " << m;
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyQScintilla editor;
    QFile  fd(__FILE__);
    fd.open(QFile::ReadOnly | QFile::Text);
    QTextStream  in(&fd);
    editor.setContent(in.readAll());
    editor.show();
    editor.ensureLineVisible(46);
    //editor.ensureCursorVisible();
    return app.exec();
}
