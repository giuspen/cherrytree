# -*- coding: UTF-8 -*-
#
#       main.py
#
#       Copyright 2009-2012 Giuseppe Penone <giuspen@gmail.com>
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
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

import gtk, gobject
import sys, os, gettext, socket, threading
import cons, core

HOST = "127.0.0.1"
PORT = 63891


class ServerThread(threading.Thread):
    """Server listening for requests to open new documents"""
    def __init__(self, semaphore, msg_server_to_core):
        super(ServerThread, self).__init__()
        self.semaphore = semaphore
        self.msg_server_to_core = msg_server_to_core

    def run(self):
        sock_srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try: sock_srv.bind((HOST, PORT))
        except:
            print "port %s busy => cherrytree multiple instances centralized control disabled" % PORT
            return
        sock_srv.listen(1)
        while True:
            try: conn, addr = sock_srv.accept()
            except: continue
            print "connected with", addr
            while True:
                data = conn.recv(1024)
                if not data: break
                if len(data) < 4 or data[:4] != "ct*=":
                    print "bad data =", data
                    break
                conn.send("okz")
                filepath = filepath_fix(data[4:])
                self.semaphore.acquire()
                self.msg_server_to_core['p'] = filepath
                self.msg_server_to_core['f'] = 1
                self.semaphore.release()
            conn.close()


class CherryTreeHandler():
    def __init__(self, filepath, semaphore, msg_server_to_core, lang_str):
        self.semaphore = semaphore
        self.msg_server_to_core = msg_server_to_core
        self.lang_str = lang_str
        self.running_windows = []
        filepath = filepath_fix(filepath)
        self.window_open_new(filepath)
        self.server_check_timer_id = gobject.timeout_add(1000, self.server_periodic_check) # 1 sec

    def window_open_new(self, filepath):
        """Open a new top level Window"""
        window = core.CherryTree(self.lang_str, filepath, self)
        self.running_windows.append(window)
        self.curr_win_idx = -1

    def on_window_destroy_event(self, widget):
        """Before close the application (from the window top right X)..."""
        self.running_windows.pop(self.curr_win_idx)
        self.curr_win_idx = -1
        if not self.running_windows: gtk.main_quit()

    def server_periodic_check(self):
        """Check Whether the server posted messages"""
        self.semaphore.acquire()
        #print "check '%s'" % self.msg_server_to_core['f']
        if self.msg_server_to_core['f']:
            self.msg_server_to_core['f'] = 0
            # 0) debug
            for i, runn_win in enumerate(self.running_windows):
                print "already running '%s' - '%s'" % (runn_win.file_dir, runn_win.file_name)
            # 1) check for opened window with same filepath
            for i, runn_win in enumerate(self.running_windows):
                if self.msg_server_to_core['p']\
                and runn_win.file_name\
                and self.msg_server_to_core['p'] == os.path.join(runn_win.file_dir, runn_win.file_name):
                    print "1 rise existing '%s'" % self.msg_server_to_core['p']
                    self.curr_win_idx = i
                    runn_win.window.present()
                    break
            else:
                # 2) check for opened window with empty path
                for i, runn_win in enumerate(self.running_windows):
                    if not runn_win.file_name:
                        print "2 rise existing '%s'" % self.msg_server_to_core['p']
                        self.curr_win_idx = i
                        runn_win.window.present()
                        runn_win.file_startup_load(self.msg_server_to_core['p'])
                        break
                else:
                    # 3) run new window
                    print "3 run '%s'" % self.msg_server_to_core['p']
                    self.window_open_new(self.msg_server_to_core['p'])
        self.semaphore.release()
        return True # this way we keep the timer alive


def initializations():
    """Initializations"""
    if sys.platform[0:3] == "win":
        import warnings
        warnings.filterwarnings("ignore")
    else:
        # config dir check
        all_apps_config_dir = os.path.join(os.path.expanduser('~'), '.config')
        if not os.path.isdir(all_apps_config_dir): os.mkdir(all_apps_config_dir)
        cherrytree_config_dir = os.path.join(all_apps_config_dir, 'cherrytree')
        if not os.path.isdir(cherrytree_config_dir): os.mkdir(cherrytree_config_dir)
        try:
            # change process name
            import ctypes, ctypes.util
            libc = ctypes.cdll.LoadLibrary(ctypes.util.find_library("libc"))
            libc.prctl(15, cons.APP_NAME, 0, 0, 0)
        except: print "libc.prctl not available, the process name will be python and not cherrytree"
    try:
        # change locale text domain
        import locale
        locale.bindtextdomain(cons.APP_NAME, cons.LOCALE_PATH)
    except:
        try:
            from ctypes import cdll
            libintl = cdll.intl
            libintl.bindtextdomain(cons.APP_NAME, cons.LOCALE_PATH)
        except:
            print "bindtextdomain not available, the glade i18n will not work properly"
    # language installation
    if os.path.isfile(cons.LANG_PATH):
        lang_file_descriptor = file(cons.LANG_PATH, 'r')
        lang_str = lang_file_descriptor.read()
        lang_file_descriptor.close()
        if lang_str != 'default': os.environ["LANGUAGE"] = lang_str
    else: lang_str = 'default'
    try: gettext.translation(cons.APP_NAME, cons.LOCALE_PATH).install()
    except:
        import __builtin__
        def _(transl_str):
            return transl_str
        __builtin__._ = _
    return lang_str


def filepath_fix(filepath):
    """Fix a FilePath to an Absolute Path"""
    if not filepath: return ""
    if not os.path.dirname(filepath): filepath = os.path.join(os.getcwd(), filepath)
    else: filepath = os.path.abspath(filepath)
    return filepath


def main(OPEN_WITH_FILE):
    """Everything Starts from Here"""
    try:
        # client
        sock_cln = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock_cln.connect((HOST, PORT))
        sock_cln.send("ct*=%s" % OPEN_WITH_FILE)
        data = sock_cln.recv(1024)
        sock_cln.close()
        if data != "okz": raise
    except:
        # server + core
        lang_str = initializations()
        gobject.threads_init()
        semaphore = threading.Semaphore()
        msg_server_to_core = {'f':0, 'p':""}
        server_thread = ServerThread(semaphore, msg_server_to_core)
        server_thread.start()
        CherryTreeHandler(OPEN_WITH_FILE, semaphore, msg_server_to_core, lang_str)
        gtk.main() # start the gtk main loop
        # quit thread
        if sys.platform[0:3] == "win":
            import ctypes
            PROCESS_TERMINATE = 1
            handle = ctypes.windll.kernel32.OpenProcess(PROCESS_TERMINATE, False, os.getpid())
            ctypes.windll.kernel32.TerminateProcess(handle, -1)
            ctypes.windll.kernel32.CloseHandle(handle)
        else:
            import signal
            os.kill(os.getpid(), signal.SIGALRM)
