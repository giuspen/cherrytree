# -*- coding: UTF-8 -*-
#
#       main.py
#
#       Copyright 2009-2014 Giuseppe Penone <giuspen@gmail.com>
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
                sep_pos = data.find("\x03")
                filepath = filepath_fix(data[4:sep_pos] if sep_pos != -1 else data[4:])
                node_name = data[sep_pos+1:] if sep_pos != -1 else ""
                self.semaphore.acquire()
                self.msg_server_to_core['p'] = filepath
                self.msg_server_to_core['n'] = node_name
                self.msg_server_to_core['f'] = 1
                self.semaphore.release()
            conn.close()


class CherryTreeHandler():
    def __init__(self, args, semaphore, msg_server_to_core, lang_str):
        self.semaphore = semaphore
        self.msg_server_to_core = msg_server_to_core
        self.lang_str = lang_str
        self.running_windows = []
        self.systray_active = False
        self.window_open_new(filepath_fix(args.filepath), args.node, True)
        self.server_check_timer_id = gobject.timeout_add(1000, self.server_periodic_check) # 1 sec

    def window_open_new(self, filepath, node_name, is_startup):
        """Open a new top level Window"""
        window = core.CherryTree(self.lang_str, filepath, node_name, self, is_startup)
        self.running_windows.append(window)

    def on_window_destroy_event(self, widget):
        """Before close the application (from the window top right X)..."""
        for i, runn_win in enumerate(self.running_windows):
            if runn_win.window == widget:
                print "win destroy: runn_win found with id", i
                break
        else:
            print "win destroy: runn_win not found"
            i = -1
        self.running_windows.pop(i)
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
                    runn_win.window.present()
                    node_name = self.msg_server_to_core['n']
                    if node_name:
                        node_name_iter = runn_win.get_tree_iter_from_node_name(node_name, use_content=False)
                        if not node_name_iter:
                            node_name_iter = runn_win.get_tree_iter_from_node_name(node_name, use_content=True)
                        if node_name_iter:
                            runn_win.treeview_safe_set_cursor(node_name_iter)
                            runn_win.sourceview.grab_focus()
                    break
            else:
                # 2) check for opened window with empty path
                for i, runn_win in enumerate(self.running_windows):
                    if not runn_win.file_name:
                        print "2 rise existing '%s'" % self.msg_server_to_core['p']
                        runn_win.window.present()
                        runn_win.file_startup_load(self.msg_server_to_core['p'], "")
                        break
                else:
                    # 3) check for opened window hidden (systray) in case there's no filepath (run from launcher)
                    just_run_new_win = True
                    if not self.msg_server_to_core['p']:
                        for i, runn_win in enumerate(self.running_windows):
                            if not runn_win.window.get_property(cons.STR_VISIBLE):
                                print "3 rise existing hidden in systray"
                                runn_win.window.present()
                                just_run_new_win = False
                                break
                    if just_run_new_win:
                        # 4) run new window
                        print "4 run '%s'" % self.msg_server_to_core['p']
                        self.window_open_new(self.msg_server_to_core['p'], self.msg_server_to_core['n'], False)
        self.semaphore.release()
        return True # this way we keep the timer alive


def initializations():
    """Initializations"""
    if not os.path.isdir(os.path.dirname(cons.CONFIG_DIR)): os.mkdir(os.path.dirname(cons.CONFIG_DIR))
    if not os.path.isdir(cons.CONFIG_DIR): os.mkdir(cons.CONFIG_DIR)
    if cons.IS_WIN_OS:
        import warnings
        warnings.filterwarnings(cons.STR_IGNORE)
    else:
        try:
            # change process name
            import ctypes, ctypes.util
            libc = ctypes.cdll.LoadLibrary(ctypes.util.find_library("libc"))
            libc.prctl(15, cons.APP_NAME, 0, 0, 0)
        except:
            print "libc.prctl not available, the process name will be python and not cherrytree"
    import locale
    locale.setlocale(locale.LC_ALL, '')
    try:
        # change locale text domain
        locale.bindtextdomain(cons.APP_NAME, cons.LOCALE_PATH)
        locale.bind_textdomain_codeset(cons.APP_NAME, cons.STR_UTF8)
    except:
        try:
            from ctypes import cdll
            libintl = cdll.intl
            libintl.bindtextdomain(cons.APP_NAME, cons.LOCALE_PATH)
            libintl.bind_textdomain_codeset(cons.APP_NAME, cons.STR_UTF8)
        except:
            print "ctypes.cdll.intl.bindtextdomain not available, the glade i18n will not work properly"
        try:
            from ctypes import windll
            lcid = windll.kernel32.GetUserDefaultUILanguage()
            os.environ["LANGUAGE"] = cons.MICROSOFT_WINDOWS_LCID_to_ISO_LANG[lcid]
        except:
            print "ctypes.windll.kernel32.GetUserDefaultUILanguage, the i18n may not work properly"
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


def main(args):
    """Everything Starts from Here"""
    try:
        # client
        sock_cln = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock_cln.connect((HOST, PORT))
        if not args.node: sock_cln.send("ct*=%s" % args.filepath)
        else: sock_cln.send("ct*=%s\x03%s" % (args.filepath, args.node))
        data = sock_cln.recv(1024)
        sock_cln.close()
        if data != "okz": raise
    except:
        #raise
        # server + core
        lang_str = initializations()
        gobject.threads_init()
        semaphore = threading.Semaphore()
        msg_server_to_core = {'f':0, 'p':""}
        server_thread = ServerThread(semaphore, msg_server_to_core)
        server_thread.start()
        CherryTreeHandler(args, semaphore, msg_server_to_core, lang_str)
        gtk.main() # start the gtk main loop
        # quit thread
        if cons.IS_WIN_OS:
            import ctypes
            PROCESS_TERMINATE = 1
            handle = ctypes.windll.kernel32.OpenProcess(PROCESS_TERMINATE, False, os.getpid())
            ctypes.windll.kernel32.TerminateProcess(handle, -1)
            ctypes.windll.kernel32.CloseHandle(handle)
        else:
            import signal
            os.kill(os.getpid(), signal.SIGALRM)
