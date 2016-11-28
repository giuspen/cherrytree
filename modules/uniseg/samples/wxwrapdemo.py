#!/usr/bin/env python
# encoding: utf-8
"""Text wrapping demo on uniseg + wxPython """

from __future__ import (absolute_import,
                        division,
                        print_function,
                        unicode_literals)
from locale import getpreferredencoding

import wx

from uniseg.wrap import wrap, Formatter


default_text = """The quick (\u201cbrown\u201d) fox \
can\u2019t jump 32.3 feet, right?

Alice was beginning to get very tired of sitting by her \
sister on the bank, and of having nothing to do: once or \
twice she had peeped into the book her sister was reading, \
but it had no pictures or conversations in it, 'and what is \
the use of a book,' thought Alice 'without pictures or \
conversation?'

\u864e\u68b5\u540d\u30f4\u30a3\u30e4\u30b0\u30e9\u3001\u4eca\
\u306e\u30a4\u30f3\u30c9\u8a9e\u3067\u30d0\u30b0\u3001\u5357\
\u30a4\u30f3\u30c9\u306e\u30bf\u30df\u30eb\u8a9e\u3067\u30d4\
\u30ea\u3001\u30b8\u30e3\u30ef\u540d\u30de\u30c1\u30e3\u30e0\
\u3001\u30de\u30ec\u30fc\u540d\u30ea\u30de\u30a6\u3001\u30a2\
\u30e9\u30d6\u540d\u30cb\u30e0\u30eb\u3001\u82f1\u8a9e\u3067\
\u30bf\u30a4\u30ac\u30fc\u3001\u305d\u306e\u4ed6\u6b27\u5dde\
\u8af8\u56fd\u5927\u62b5\u3053\u308c\u306b\u4f3c\u304a\u308a\
\u3001\u3044\u305a\u308c\u3082\u30ae\u30ea\u30b7\u30a2\u3084\
\u30e9\u30c6\u30f3\u306e\u30c1\u30b0\u30ea\u30b9\u306b\u57fa\
\u3065\u304f\u3002\u305d\u306e\u30c1\u30b0\u30ea\u30b9\u306a\
\u308b\u540d\u306f\u53e4\u30da\u30eb\u30b7\u30a2\u8a9e\u306e\
\u30c1\u30b0\u30ea\uff08\u7bad\uff09\u3088\u308a\u51fa\u3067\
\u3001\u864e\u306e\u99db\u304f\u8d70\u308b\u3092\u7bad\u306e\
\u98db\u3076\u306b\u6bd4\u3079\u305f\u308b\u306b\u56e0\u308b\
\u306a\u3089\u3093\u3068\u3044\u3046\u3002\u308f\u304c\u56fd\
\u3067\u3082\u53e4\u6765\u864e\u3092\u5b9f\u969b\u898b\u305a\
\u306b\u5343\u91cc\u3092\u8d70\u308b\u3068\u4fe1\u3058\u3001\
\u622f\u66f2\u306b\u6e05\u6b63\u306e\u6377\u75be\u3092\u8cde\
\u3057\u3066\u5343\u91cc\u4e00\u8df3\u864e\u4e4b\u52a9\u306a\
\u3069\u3068\u6d12\u843d\u3066\u5c45\u308b\u3002\u30d7\u30ea\
\u30cb\u306e\u300e\u535a\u7269\u5fd7\u300f\u306b\u62e0\u308c\
\u3070\u751f\u304d\u305f\u864e\u3092\u30ed\u30fc\u30de\u4eba\
\u304c\u521d\u3081\u3066\u898b\u305f\u306e\u306f\u30a2\u30a6\
\u30b0\u30b9\u30c3\u30b9\u5e1d\u306e\u4ee3\u3060\u3063\u305f\
\u3002
"""


_preferredencoding = getpreferredencoding()


class SampleWxFormatter(Formatter):
    
    def __init__(self, dc, log_width):
        
        self._dc = dc
        self._log_width = log_width
        self._log_cur_x = 0
        self._log_cur_y = 0
    
    @property
    def wrap_width(self):

        return self._log_width

    def reset(self):
        
        self._log_cur_x = 0
        self._log_cur_y = 0

    def text_extents(self, s):
        
        dc = self._dc
        return dc.GetPartialTextExtents(s)

    def handle_text(self, text, extents):
        
        if not text or not extents:
            return

        dc = self._dc
        dc.DrawText(text, self._log_cur_x, self._log_cur_y)
        self._log_cur_x += extents[-1]

    def handle_new_line(self):
        
        dc = self._dc
        log_line_height = dc.GetCharHeight()
        self._log_cur_y += log_line_height
        self._log_cur_x = 0


class App(wx.App):
    
    def OnInit(self):
        
        frame = Frame(None, wx.ID_ANY, __file__)
        
        self.SetTopWindow(frame)
        frame.Show()
        return True


class Frame(wx.Frame):
    
    ID_FONT = wx.NewId()
    
    def __init__(self, parent, id_, title,
                 pos=wx.DefaultPosition,
                 size=wx.DefaultSize,
                 style=wx.DEFAULT_FRAME_STYLE,
                 name='frame'):
        
        wx.Frame.__init__(self, parent, id_, title, pos, size, style, name)
        
        self.Bind(wx.EVT_MENU, self.OnCmdOpen, id=wx.ID_OPEN)
        self.Bind(wx.EVT_MENU, self.OnCmdExit, id=wx.ID_EXIT)
        self.Bind(wx.EVT_MENU, self.OnCmdFont, id=self.ID_FONT)
        
        menubar = wx.MenuBar()
        menu = wx.Menu()
        menu.Append(wx.ID_OPEN, '&Open')
        menu.AppendSeparator()
        menu.Append(wx.ID_EXIT, '&Exit')
        menubar.Append(menu, '&File')
        menu = wx.Menu()
        menu.Append(self.ID_FONT, '&Font...')
        menubar.Append(menu, 'F&ormat')
        self.SetMenuBar(menubar)
        
        self.wrap_window = WrapWindow(self, wx.ID_ANY)
    
    def OnCmdOpen(self, evt):
        
        filename = wx.FileSelector('Open')
        if not filename:
            return
        raw_text = open(filename, 'rU').read()
        for enc in {'utf-8', _preferredencoding}:
            try:
                text = raw_text.decode(enc)
            except UnicodeDecodeError:
                continue
            else:
                break
        else:
            wx.MessageBox('Couldn\'t open this file.', 'Open', wx.ICON_ERROR)
            return
        self.wrap_window.SetText(text)
        self.wrap_window.Refresh()
    
    def OnCmdExit(self, evt):
        
        self.Close()
    
    def OnCmdFont(self, evt):
        
        data = wx.FontData()
        font = self.wrap_window.GetFont()
        data.SetInitialFont(font)
        
        dlg = wx.FontDialog(self, data)
        if dlg.ShowModal() == wx.ID_OK:
            ret_data = dlg.GetFontData()
            ret_font = ret_data.GetChosenFont()
            self.wrap_window.SetFont(ret_font)
            self.wrap_window.Refresh()


class WrapWindow(wx.Window):
    
    _text = default_text
    _default_fontface = 'Times New Roman'
    _default_fontsize = 18
    
    def __init__(self, parent, id_,
                 pos=wx.DefaultPosition, size=wx.DefaultSize,
                 style=0, name=wx.PanelNameStr):
        
        wx.Window.__init__(self, parent, id_, pos, size, style, name)
        
        self.SetBackgroundStyle(wx.BG_STYLE_CUSTOM)
        
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        
        self.SetBackgroundColour(wx.WHITE)
        self.SetForegroundColour(wx.BLACK)
        font = wx.Font(self._default_fontsize,
                       wx.FONTFAMILY_DEFAULT,
                       wx.FONTSTYLE_NORMAL,
                       wx.FONTWEIGHT_NORMAL,
                       False,
                       self._default_fontface)
        self.SetFont(font)
    
    def GetText(self, value):
        
        return _text
    
    def SetText(self, value):
        
        self._text = value
    
    def OnPaint(self, evt):
        
        dc = wx.AutoBufferedPaintDC(self)
        dc.Clear()
        
        font = self.GetFont()
        dc.SetFont(font)

        dev_width, dev_height = self.GetClientSize()
        log_width   = dc.DeviceToLogicalX(dev_width)
        log_height  = dc.DeviceToLogicalY(dev_height)
        
        formatter = SampleWxFormatter(dc, log_width)
        wrap(formatter, self._text)
    
    def OnSize(self, evt):
        
        self.Refresh()


def main():
    
    app = App(0)
    app.MainLoop()


if __name__ == '__main__':
    main()
