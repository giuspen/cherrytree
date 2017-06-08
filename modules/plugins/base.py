#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

from __future__ import absolute_import
from __future__ import print_function

# __author__ = 'Jakub Kolasa <jakub@arker.pl'>
import sys
import re
import traceback
from menusutils import menuBarInsert
from menus import KB_ALT, KB_CONTROL, KB_SHIFT
import os.path
from ConfigParser import SafeConfigParser
import cons

DEBUG = 1

# TODO: add support for stock images for menus, etc


class CTMenuItem(object):
    def __init__(self, action, after=None, sk=None, sd=None, kb=None, dn=None, cb=None):
        """# sk = stock; sd = short description; kb = keyboard shortcut, dn = description, cb = callback
        action is split by ':',2
        after = insert menu before item
        """

        self.action = action.split(":", 2)
        actionlast = self.action[-1]
        self.actionlast = actionlast
        self.sk = sk or actionlast
        # self.sd = _(sd or actionlast)
        # where is defined _ ?
        self.sd = sd or actionlast
        self.kb = kb
        self.dn = dn or self.sd
        self.cb = cb or actionlast
        self.after = after

    def build(self, dad, plugin):
        if not dad.menudict:
            print ("DAD is not ready")
            return
        cb = getattr(plugin, self.cb)
        menu = dict(sk=self.sk, sd=self.sd, kb=self.kb, dn=self.dn, cb=cb)
        dad.menudict[self.actionlast] = menu
        menuBarInsert(self.action[0:-1], self.actionlast, self.after)

        # TODO: unify with CONFIG_ACTIONS_DICT, somehow
        # CONFIG_ACTIONS_DICT[self.action[0]].append(self.action[1])


class CTPlugin(object):
    name = None
    friendly_name = None
    plugin_menu = None
    config = None
    config_defaults = None

    def __init__(self, dad):
        self.dad = dad
        self.config = {}
        if not self.name:
            self.name = self.__class__.__name__.lower()

        if not self.friendly_name:
            self.friendly_name = self.name

        self.config_load()

    def config_load(self):
        self.config = self.config_defaults.copy() if self.config_defaults else dict()
        fn = os.path.join(cons.CONFIG_DIR, self.name + ".conf")
        if not os.path.isfile(fn):
            return

        cfg = SafeConfigParser()
        try:
            config = self.config
            cfg.read(fn)
            for name, value in cfg.items(self.name):
                if name not in config:
                    continue
                try:
                    defvalue = config[name]
                    if defvalue is not None:
                        value = type(defvalue)(value)
                    config[name] = value
                except:
                    pass

        except Exception as e:
            # TODO: logger?
            print("Config read failed:", e.__class__.__name__, e)

    def config_save(self):
        cfg = SafeConfigParser()
        try:
            config = self.config
            section = self.name
            cfg.add_section(section)
            for name, value in config.items():
                cfg.set(section, name, value)

            fn = os.path.join(cons.CONFIG_DIR, self.name + ".conf")
            with open(fn, 'wb') as fn:
                cfg.write(fn)

        except Exception as e:
            # TODO: logger?
            print("Config write failed")

    def build_plugin_menu(self):
        """return plugin menu, which should be placed in menu"""
        if not self.plugin_menu:
            return None
        ret = []
        for m in self.plugin_menu:
            m.build(self.dad, self)

        return None


class CTPlugins(object):
    plugins = None
    __iter = 0

    def __init__(self, dad):
        self.dad = dad
        self.plugins = []

    def _load(self, name):
        #
        # Dynamic plugin/module load with real dynamic support
        # Real dynamic loading is deprecated cause of freezing methods.
        #
        module = 'plugins.{0}'.format(re.sub("[^a-zA-Z0-9_]", "_", name))

        try:
            return sys.modules[module], None
        except KeyError:
            pass

        # try to import in a very classic way first
        try:
            __import__(module)
            mod = sys.modules[module]
            return mod, None
        except Exception as e:
            # print ">>>",e
            if DEBUG:
                traceback.print_exc(100, file=sys.stdout)
            return None, str(e)

    def load(self, name):
        mod, err = self._load(name)

        obj = mod.FACTORY(self.dad)
        self.plugins.append(obj)

    def config_save(self):
        for p in self.plugins:
            p.config_save()

    def build_menus(self):
        for obj in self.plugins:
            obj.build_plugin_menu()

    def __iter__(self):
        return iter(self.plugins)
