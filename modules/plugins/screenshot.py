#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

from __future__ import absolute_import
from __future__ import print_function

# __author__ = 'Jakub Kolasa <jkolczasty@gmail.com'>

import support
import gtk
import os.path
import subprocess
import tempfile
import shlex
# Handle screenshot insert from external application

from .base import CTPlugin, CTMenuItem, KB_CONTROL, KB_ALT


class ScreenShotPlugin(CTPlugin):
    friendly_name = _("Screenshot Plugin")

    plugin_menu = [
        CTMenuItem('MenuBar:EditMenu:menu_screenshot', after='handle_image', sd="Insert Screenshot", sk='image_insert', kb=KB_CONTROL+KB_ALT+"S", cb='handle_screenshot')
    ]

    config_defaults = dict(screenshot_exec=None)

    def handle_screenshot(self, *args):
        if not self.config.get('screenshot_exec'):
            support.dialog_error(_("Screenshot tools is not configured. Update preferences."), self.dad.window)
            return

        if not self.dad.node_sel_and_rich_text(): return
        if not self.dad.is_curr_node_not_read_only_or_error(): return
        filename = None
        try:
            h, filename = tempfile.mkstemp(prefix='tmpctscreenshot-', suffix='.png')
            os.close(h)
            fargs = dict(tempfilename=filename)
            args = self.config['screenshot_exec'].format(**fargs)
            args = shlex.split(args)
            subprocess.check_call(args)
            if not os.path.getsize(filename):
                os.unlink(filename)
                return
        except Exception as e:
            support.dialog_error(_("Exception: {0}: {1}".format(e.__class__.__name__, e)), self.dad.window)
            if filename:
                os.unlink(filename)
            return

        try:
            pixbuf = gtk.gdk.pixbuf_new_from_file(filename)
            self.dad.image_edit_dialog(pixbuf, self.dad.curr_buffer.get_iter_at_mark(self.dad.curr_buffer.get_insert()))
        except:
            support.dialog_error(_("Image Format Not Recognized"), self.dad.window)
        finally:
            if filename:
                os.unlink(filename)

    def ui_preferences_tab(self, dad, vbox_tools, pref_dialog):
        """Preferences Dialog, Misc Tab"""
        for child in vbox_tools.get_children(): child.destroy()

        hbox_screenshot = gtk.HBox()
        hbox_screenshot.set_spacing(4)
        label_screenshot = gtk.Label(_("Screenshot command, e.g.: spectacle -r -b -d 5000 -n -o {tempfilename}"))
        entry_screenshot = gtk.Entry()
        entry_screenshot.set_text(self.config['screenshot_exec'] or "")
        hbox_screenshot.pack_start(label_screenshot, expand=False)
        hbox_screenshot.pack_start(entry_screenshot)

        vbox_all = gtk.VBox()
        vbox_all.set_spacing(2)
        vbox_all.pack_start(hbox_screenshot)
        frame_all = gtk.Frame(label="<b>" + _("Tools") + "</b>")
        frame_all.get_label_widget().set_use_markup(True)
        frame_all.set_shadow_type(gtk.SHADOW_NONE)
        align_all = gtk.Alignment()
        align_all.set_padding(3, 6, 6, 6)
        align_all.add(vbox_all)
        frame_all.add(align_all)

        vbox_tools.pack_start(frame_all, expand=False)

        def on_entry_screenshot_changed(entry):
            self.config['screenshot_exec'] = entry.get_text()

        entry_screenshot.connect('changed', on_entry_screenshot_changed)


FACTORY = ScreenShotPlugin
