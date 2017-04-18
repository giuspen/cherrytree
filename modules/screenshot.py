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


class ScreenShotHandler(object):
    def __init__(self, dad):
        print ("Screenshot handler loaded")
        self.dad = dad

    def __call__(self, *args):
        if not self.dad.screenshot_exec:
            support.dialog_error(_("Screenshot tools is not configured. Update preferences."), self.dad.window)
            return

        if not self.dad.node_sel_and_rich_text(): return
        if not self.dad.is_curr_node_not_read_only_or_error(): return
        filename = None
        try:
            h, filename = tempfile.mkstemp(prefix='tmpctscreenshot-', suffix='.png')
            os.close(h)
            fargs = dict(tempfilename=filename)
            args = self.dad.screenshot_exec.format(**fargs)
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
