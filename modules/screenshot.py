# -*- coding: UTF-8 -*-
#
#       screenshot.py
#
#       Copyright CherryTree 2009-2018 Giuseppe Penone <giuspen@gmail.com>
#
#       Copyright screenshot.py 2018 David Holland <davidholland5499@outlook.com>, as long as it is a standalone module and not integrated with Cherry Tree
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

import gtk
import gtk.gdk

class ScreenshotWindow(gtk.Window):
    window_modifier = "move"
    window_mode = "Move-Mode"
    pixbuf_result = None

    label = gtk.Label()
    # label.set_justify(gtk.JUSTIFY_CENTER)

    def __init__(self, ready=None):
        gtk.Window.__init__(self)
        width = gtk.gdk.screen_width()
        height = gtk.gdk.screen_height()

        # self.connect("destroy",lambda wid:gtk.main_quit())
        self.move(250,100)

        self.resize(width-500, height-200)
        self.set_opacity(0.7)
        self.set_keep_above(True)
        self.set_decorated(False)

        self.connect("key_press_event", self.on_key_press)
        self.connect("key_release_event", self.on_key_release)
        self.connect("button-press-event", self.on_clicked)
        self.set_events(gtk.gdk.KEY_PRESS_MASK | gtk.gdk.KEY_RELEASE_MASK | gtk.gdk.BUTTON_PRESS_MASK)

        col = gtk.gdk.Color('#666')
        self.modify_bg(gtk.STATE_NORMAL, col)

        self.window_instructions = _('<span size="xx-large">To switch between Move- and Resize-Mode,\nsimply press <i>Space</i> or <i>Tab</i>.\nIf you want to move the upper, lower, left or right edge individually\npress either the <i>arrow</i> or <i>WASD</i> keys to switch modes.\nTo return to Move-Mode again press <i>space</i> or <i>tab</i>!\nIf you are in the desired mode,\nclick and drag with the left mouse button to modify the window.\nIf you are happy with the screen snippet,\npress the <i>Return</i> key to take the screenshot,\nor press the <i>Esc</i> key to cancel the screenshot.\n\n<b>Press Space to dismiss this dialog!</b></span>')
        self.label.show()

        self.add(self.label)

        self.update_mode_label(self.label)

        self.fullscreen()

    def run(self):
        self.show()
        gtk.main()
        self.destroy()
        return self.pixbuf_result

    def stop(self, result):
        self.pixbuf_result = result
        gtk.main_quit()

    def update_mode_label(self, mode_label):
        mode_label.set_markup('<span foreground="white" size="xx-large"><b>' + self.window_mode + '</b></span><span foreground="white">\nDouble-Click for help</span>');

    def on_clicked(self, widget, event):
        widget.window.unfullscreen()
        if event.type == 5 or event.type == 6:
            dlg = gtk.Dialog("How to", widget, gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT, (gtk.STOCK_OK, gtk.RESPONSE_OK))

            lbl = gtk.Label()
            lbl.set_markup(self.window_instructions);
            lbl.set_justify(gtk.JUSTIFY_CENTER)
            lbl.set_line_wrap(False)

            lbl.show()
            dlg.vbox.add(lbl)

            dlg.set_keep_above(True)
            dlg.present()
            dlg.run()
            dlg.destroy()
        if self.window_modifier == "move":
            widget.window.begin_move_drag(event.button, int(event.x_root), int(event.y_root), event.time)
        elif self.window_modifier == "resize":
            widget.window.begin_resize_drag(gtk.gdk.WINDOW_EDGE_SOUTH_EAST, event.button, int(event.x_root), int(event.y_root), event.time)
        elif self.window_modifier == "resize_l":
            widget.window.begin_resize_drag(gtk.gdk.WINDOW_EDGE_WEST, event.button, int(event.x_root), int(event.y_root), event.time)
        elif self.window_modifier == "resize_r":
            widget.window.begin_resize_drag(gtk.gdk.WINDOW_EDGE_EAST, event.button, int(event.x_root), int(event.y_root), event.time)
        elif self.window_modifier == "resize_u":
            widget.window.begin_resize_drag(gtk.gdk.WINDOW_EDGE_NORTH, event.button, int(event.x_root), int(event.y_root), event.time)
        elif self.window_modifier == "resize_d":
            widget.window.begin_resize_drag(gtk.gdk.WINDOW_EDGE_SOUTH, event.button, int(event.x_root), int(event.y_root), event.time)

    def on_key_press(self, widget, event):
        key_name = gtk.gdk.keyval_name(event.keyval)
        if key_name == "Return":
            if not widget.emit("delete-event", gtk.gdk.Event(gtk.gdk.DELETE)):
                x, y = widget.window.get_position()
                width, height = widget.window.get_size()
                widget.window.destroy()

                format = "png"
                screenshot = gtk.gdk.Pixbuf.get_from_drawable(gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, True, 8, width, height), gtk.gdk.get_default_root_window(), gtk.gdk.colormap_get_system(), x, y, 0, 0, width, height)
                self.stop(screenshot)
        if key_name == "Escape":
            self.stop(None)

    def on_key_release(self, widget, event):
        key_name = gtk.gdk.keyval_name(event.keyval)
        # print key_name
        if key_name == "space" or key_name == "Tab":
            if self.window_modifier == "move":
                self.window_modifier = "resize"
                self.window_mode = "Resize-Mode"
                self.update_mode_label(self.label)
            else:
                self.window_modifier = "move"
                self.window_mode = "Move-Mode"
                self.update_mode_label(self.label)
        elif key_name == "Left" or key_name == "a":
            self.window_modifier = "resize_l"
            self.window_mode = "Left-Edge"
            self.update_mode_label(self.label)
        elif key_name == "Right" or key_name == "d":
            self.window_modifier = "resize_r"
            self.window_mode = "Right-Edge"
            self.update_mode_label(self.label)
        elif key_name == "Up" or key_name == "w":
            self.window_modifier = "resize_u"
            self.window_mode = "Upper-Edge"
            self.update_mode_label(self.label)
        elif key_name == "Down" or key_name == "s":
            self.window_modifier = "resize_d"
            self.window_mode = "Lower-Edge"
            self.update_mode_label(self.label)
