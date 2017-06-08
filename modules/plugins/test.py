#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

from __future__ import absolute_import
from __future__ import print_function

# __author__ = 'Jakub Kolasa <jakub@arker.pl'>

from .base import CTPlugin, CTMenuItem, KB_SHIFT, KB_CONTROL, KB_ALT
import support


class TestPlugin(CTPlugin):
    plugin_menu = [
        CTMenuItem('MenuBar:EditMenu:menu_test', after='handle_screenshot', sk='image_insert', kb=KB_CONTROL + KB_SHIFT + KB_ALT + "T", cb='handle_test')
    ]

    def get_plugin_menu(self):
        pass

    def handle_test(self, *args):
        support.dialog_info("Hello from test", self.dad.window)


FACTORY = TestPlugin
